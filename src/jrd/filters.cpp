/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		filters.cpp
 *	DESCRIPTION:	Blob filters for system define objects
 *
 * The contents of this file are subject to the Interbase Public
 * License Version 1.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy
 * of the License at http://www.Inprise.com/IPL.html
 *
 * Software distributed under the License is distributed on an
 * "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express
 * or implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code was created by Inprise Corporation
 * and its predecessors. Portions created by Inprise Corporation are
 * Copyright (C) Inprise Corporation.
 *
 * All Rights Reserved.
 * Contributor(s): ______________________________________.
 */

#include "fbdev.h"
#include "../jrd/ib_stdio.h"
#include <string.h>
#include "../jrd/jrd.h"
//#include "../jrd/y_ref.h"
#include "../jrd/ibase.h"
#include "../jrd/acl.h"
#include "../jrd/val.h"
#include "../jrd/met.h"
#include "../jrd/blob_filter.h"
#include "../jrd/intl.h"
#include "../jrd/intl_classes.h"
#include "../intlcpp/charsets.h"
#include "../jrd/gdsassert.h"
#include "../jrd/err_proto.h"
#include "../jrd/filte_proto.h"
#include "../jrd/gds_proto.h"
#include "../jrd/intl_proto.h"
#include "../jrd/thd_proto.h"

static ISC_STATUS caller(USHORT, ctl*, USHORT, UCHAR*, USHORT*);
static void dump_blr(void*, SSHORT, const char*);
static void move(const char*, char*, USHORT);
static ISC_STATUS string_filter(USHORT, ctl*);
static void string_put(ctl*, const char*);

/* Note:  This table is used to indicate which bytes could represent
 *	  ASCII characters - and is used to filter "untyped" blobs
 *	  (subtype 0) to type text.
 *	  For V3 historical reasons, only ASCII byte values are passed
 *	  through.  All other byte values are mapped to ascii '.'
 */
static const UCHAR char_tab[128] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
};


/* Miscellaneous filter stuff (should be moved someday) */

typedef struct tmp {
	tmp* tmp_next;
	USHORT tmp_length;
	TEXT tmp_string[1];
} *TMP;

#ifdef VMS
const char* const WILD_CARD_UIC = "<*,*>";
#else
const char* const WILD_CARD_UIC = "(*.*)";
#endif

/* TXNN: Used on filter of internal data structure to text */
static const TEXT acl_privs[] = "?CGDRWPIEUTX??";

static const TEXT acl_ids[][16] = {
	"?: ",
	"group: ",
	"user: ",
	"person: ",
	"project: ",
	"organization: ",
	"node: ",
	"view: ",
	"all views",
	"trigger: ",
	"procedure: ",
	"role: "
};

/* TXNN: Used on filter of internal data structure to text */
static const TEXT dtypes[][36] = {
	"",
	"TEXT, length %d",
	"COUNTED_STRING, length %d",
	"VARYING, max length %d",
	"",
	"",
	"PACKED DECIMAL, length %d, scale %d",
	"BYTE, scale %d",
	"SHORT, scale %d",
	"LONG, scale %d",
	"QUAD, scale %d",
	"FLOAT",
	"DOUBLE",
	"D_FLOAT",
	"",
	"",
	"DATE",
	"BLOB",
	"ARRAY"
};


ISC_STATUS filter_acl(USHORT action, ctl* control)
{
/**************************************
 *
 *	f i l t e r _ a c l
 *
 **************************************
 *
 * Functional description
 *	Get next segment from a access control list.
 *
 **************************************/
	if (action != ACTION_open)
		return string_filter(action, control);

/* Initialize for retrieval */
	UCHAR buffer[512];
	const SLONG l = control->ctl_source_handle->ctl_total_length;
	UCHAR* const temp =
		(l <= (SLONG) sizeof(buffer)) ? buffer : (UCHAR*) gds__alloc((SLONG) l);
/* FREE: at procedure exit */
	UCHAR* p = temp;
	if (!p)						/* NOMEM: */
		return isc_virmemexh;

	USHORT length;
	const ISC_STATUS status =
		caller(ACTION_get_segment, control, (USHORT) l, temp, &length);
		
	TEXT line[256];

	if (!status) {
		sprintf(line, "ACL version %d", (int) *p++);
		string_put(control, line);
		TEXT* out = line;

        bool all_wild;
		UCHAR c;
		while (c = *p++)
			switch (c) {
			case ACL_id_list:
				all_wild = true;
				*out++ = '\t';
				while ((c = *p++) != 0) {
					all_wild = false;
					sprintf(out, "%s%.*s, ", acl_ids[c], *p, p + 1);
					p += *p + 1;
					while (*out)
						++out;
				}
				if (all_wild) {
					sprintf(out, "all users: %s, ", WILD_CARD_UIC);
					while (*out)
						++out;
				}
				break;

			case ACL_priv_list:
				sprintf(out, "privileges: (");
				while (*out)
					++out;
				while (c = *p++)
					*out++ = acl_privs[c];
				*out++ = ')';
				*out = 0;
				string_put(control, line);
				out = line;
				break;
			}
	}

	control->ctl_data[1] = control->ctl_data[0];

	if (temp != buffer)
		gds__free(temp);

	return FB_SUCCESS;
}


ISC_STATUS filter_blr(USHORT action, ctl* control)
{
/**************************************
 *
 *	f i l t e r _ b l r
 *
 **************************************
 *
 * Functional description
 *	Get next segment from a blr blob. 
 *	Doctor up Rdb BLR blobs that omit
 *	the BLR eoc so the pretty printer
 *	doesn't complain.
 *
 **************************************/
	if (action != ACTION_open)
		return string_filter(action, control);

/* Initialize for retrieval */
	UCHAR buffer[512];
	const SLONG l = 1 + control->ctl_source_handle->ctl_total_length;
	UCHAR* const temp =
		(l <=(SLONG) sizeof(buffer)) ? buffer : (UCHAR*) gds__alloc((SLONG) l);
/* FREE: at procedure exit */
	if (!temp)					/* NOMEM: */
		return isc_virmemexh;

	USHORT length;
	const ISC_STATUS status =
		caller(ACTION_get_segment, control, (USHORT) l, temp, &length);

	if (!status) {
		if ((l > length) && (temp[length - 1] != blr_eoc))
			temp[length] = blr_eoc;
		gds__print_blr(temp, dump_blr, control, 0);
	}

	control->ctl_data[1] = control->ctl_data[0];

	if (temp != buffer)
		gds__free(temp);

	return FB_SUCCESS;
}


ISC_STATUS filter_format(USHORT action, ctl* control)
{
/**************************************
 *
 *	f i l t e r _ f o r m a t
 *
 **************************************
 *
 * Functional description
 *	Get next segment from a record format blob.
 *
 **************************************/
 
	/* Unless this is a get segment call, just return success */

	if (action != ACTION_get_segment)
		return FB_SUCCESS;

	/* Loop thru descriptors looking for one with a data type */
	
	OdsDesc desc;
	
	for (;;) 
		{
        USHORT length;
		const ISC_STATUS status = caller(ACTION_get_segment,
						control,
						sizeof(desc),
						reinterpret_cast<UCHAR*>(&desc), &length);
						
		if (status != FB_SUCCESS && status != isc_segment)
			return status;
			
		if (desc.dsc_dtype)
			break;
			
		++control->ctl_data[0];
		}

	int value = desc.dsc_scale;
	const TEXT* p = dtypes[desc.dsc_dtype];

	if (desc.dsc_dtype == dtype_text)
		value = desc.dsc_length;
	else if (desc.dsc_dtype == dtype_varying)
		value = desc.dsc_length - sizeof(SSHORT);
	else if (desc.dsc_dtype > dtype_array)
		{
		p = "data type %d unknown";
		value = desc.dsc_dtype;
		}

    TEXT temp1[64], temp2[64];
    
	if ((desc.dsc_dtype <= dtype_any_text) && (desc.dsc_ttype != 0)) 
		{
		sprintf(temp2, p, value);
		sprintf(temp1, "%s, sub-type %d", temp2, INTL_TTYPE(&desc));
		}
	else
		sprintf(temp1, p, value);

	sprintf(temp2, "%ld: %s", control->ctl_data[0]++, temp1);
	USHORT length = strlen(temp2);
	
	if (length > control->ctl_buffer_length)
		length = control->ctl_buffer_length;

	control->ctl_segment_length = length;
	move(temp2, reinterpret_cast < char *>(control->ctl_buffer), length);

	return FB_SUCCESS;
}


ISC_STATUS filter_runtime(USHORT action, ctl* control)
{
/**************************************
 *
 *	f i l t e r _ r u n t i m e
 *
 **************************************
 *
 * Functional description
 *	Get next segment from a relation runtime summary blob.
 *
 **************************************/
	if (action == ACTION_close)
		return string_filter(action, control);

/* Unless this is a get segment call, just return success */

	if (action != ACTION_get_segment)
		return FB_SUCCESS;

/* If there is a string filter active, use it first */

	if (control->ctl_data[0]) {
		const ISC_STATUS astatus = string_filter(action, control);
		if (astatus != isc_segstr_eof)
			return astatus;
		string_filter(ACTION_close, control);
	}

/* Loop thru descriptors looking for one with a data type */
	UCHAR temp[256];
	UCHAR* buff = temp;
	const USHORT buff_len = sizeof(temp);
	control->ctl_data[3] = 8;

	USHORT length;
	const ISC_STATUS status =
		caller(ACTION_get_segment, control, buff_len, buff, &length);

	if (status == isc_segment)
		return isc_segstr_eof;

	if (status)
		return status;

	buff[length] = 0;
	UCHAR* p = buff + 1;
	USHORT n;
	UCHAR* q = (UCHAR*) &n;
	q[0] = p[0];
	q[1] = p[1];
	bool blr = false;

	TEXT line[128];
	switch ((RSR_T) buff[0]) {
	case RSR_field_name:
		sprintf(line, "    name: %s", p);
		break;

	case RSR_field_id:
		sprintf(line, "Field id: %d", n);
		break;

	case RSR_dimensions:
		sprintf(line, "Array dimensions: %d", n);
		break;

	case RSR_array_desc:
		sprintf(line, "Array descriptor");
		break;

	case RSR_view_context:
		sprintf(line, "    view_context: %d", n);
		break;

	case RSR_base_field:
		sprintf(line, "    base_field: %s", p);
		break;

	case RSR_computed_blr:
		sprintf(line, "    computed_blr:");
		blr = true;
		break;

	case RSR_missing_value:
		sprintf(line, "    missing_value:");
		blr = true;
		break;

	case RSR_default_value:
		sprintf(line, "    default_value:");
		blr = true;
		break;

	case RSR_validation_blr:
		sprintf(line, "    validation_blr:");
		blr = true;
		break;

	case RSR_security_class:
		sprintf(line, "    security_class: %s", p);
		break;

	case RSR_trigger_name:
		sprintf(line, "    trigger_name: %s", p);
		break;

	default:
		sprintf(line, "*** unknown verb %d ***", (int) buff[0]);
	}

	if ((length = strlen(line)) > control->ctl_buffer_length) {
		/* The string is too long for the caller's buffer.  Save the
		   entire string for output by the string_filter routine. */

		string_put(control, line);
		length = 0;
	}

	if (blr) {
		gds__print_blr(p, dump_blr, control, 0);
		control->ctl_data[1] = control->ctl_data[0];
	}

	if (!length) {
		/* The string was too long for the caller's buffer.  Return
		   as much as possible to the user. */

		return string_filter(action, control);
	}

	control->ctl_segment_length = length;
	move(line, reinterpret_cast < char *>(control->ctl_buffer),
		 control->ctl_segment_length);

	return status;
}


ISC_STATUS filter_text(USHORT action, ctl* control)
{
/**************************************
 *
 *	f i l t e r _ t e x t
 *
 **************************************
 *
 * Functional description
 *	Get next segment from a text blob.  Handle embedded
 *	carriage returns as separate segments.
 *
 *	The usage of the variable ctl_data slots:
 *
 *	    ctl_data [0]	residual bytes
 *	    ctl_data [1]	address of temp space
 *	    ctl_data [2]	flag indicating residual bytes was segment
 *	    ctl_data [3]	length of temp space
 *
 **************************************/
	ctl* source;

	switch (action) {
	case ACTION_open:
		source = control->ctl_source_handle;
		control->ctl_total_length = source->ctl_total_length;
		control->ctl_max_segment = source->ctl_max_segment;
		control->ctl_number_segments = source->ctl_number_segments;
		control->ctl_data[0] = control->ctl_data[1] = control->ctl_data[2] =
			control->ctl_data[3] = 0;
		return FB_SUCCESS;

	case ACTION_close:
		if (control->ctl_data[1]) {
			gds__free((SLONG *) control->ctl_data[1]);
			control->ctl_data[1] = 0;
		}
		return FB_SUCCESS;

	case ACTION_get_segment:
		break;

	case ACTION_put_segment:
	case ACTION_create:
	case ACTION_seek:
		return isc_uns_ext;

	case ACTION_alloc:
	case ACTION_free:
		return FB_SUCCESS;

	default:
		BUGCHECK(289);			/* Unknown blob filter ACTION */
		return isc_uns_ext;
	}

/* Drop thru for ACTION_get_segment. */

	const TEXT* left_over = 0;
	USHORT left_length = 0;
	USHORT buffer_used = 0;

/* if there was any data left over from previous get, use as much as
   user's buffer will hold */

	const USHORT length = control->ctl_data[0];
	if (length) {
		buffer_used = MIN(length, control->ctl_buffer_length);
		MOVE_FAST((void *) control->ctl_data[1], control->ctl_buffer,
				  buffer_used);

		/* remember how much did not get used */

		if (length > buffer_used) {
			left_over = (TEXT *) control->ctl_data[1] + buffer_used;
			left_length = length - buffer_used;
		}
		else {
			left_over = 0;
			left_length = 0;
		}
	}

/* if there was no data left over from previous get or all the data
   left from previous get was used and there is still more of that segment
   not read, do a get segment */
	if ((buffer_used == 0) || (control->ctl_data[2] &&
					(control->ctl_buffer_length - buffer_used > 0)))
	{
		USHORT l = control->ctl_buffer_length - buffer_used;
		const ISC_STATUS status = caller(ACTION_get_segment, control, l,
						control->ctl_buffer + buffer_used, &l);
		if (status == isc_segment)
			control->ctl_data[2] = isc_segment;
		else if (status)
			return status;
		else
			control->ctl_data[2] = FB_SUCCESS;
		buffer_used += l;
	}

/* Search data for unprintable data or EOL */

	USHORT l = buffer_used;
	for (UCHAR* p = control->ctl_buffer; l; p++, --l)
	{
		if (*p == (UCHAR) '\n') {
			/* Found a newline.  First save what comes after the newline. */

			control->ctl_segment_length = p - control->ctl_buffer;
			control->ctl_data[0] = l - 1;

			/* if control buffer cannot accommodate what needs to be saved, free
			   the control buffer */

			if (control->ctl_data[1]
				&& (control->ctl_data[0] > control->ctl_data[3]))
			{
				gds__free((SLONG *) control->ctl_data[1]);
				control->ctl_data[1] = 0;
				control->ctl_data[3] = 0;
			}

			/* if there is no control buffer allocate one */

			if (!control->ctl_data[1]) {
				control->ctl_data[1] =
					(IPTR) gds__alloc((SLONG) control->ctl_buffer_length);
				/* FREE: above & ACTION_close in this procedure */
				if (!control->ctl_data[1])	/* NOMEM: */
					return isc_virmemexh;
				control->ctl_data[3] = control->ctl_buffer_length;
			}

			/* save data after found newline */

			MOVE_FAST(p + 1, (void *) control->ctl_data[1], l - 1);

			/* if there was data in control buffer not moved to user's buffer,
			   move it to be contiguous with what was saved from user's buffer
			   (note this can work with out more checking than is done here
			   because if there was left over:
			   1. we did not have to free & reallocate buffer
			   2. what we moved back from user's buffer was able to fit in
			   control buffer before what was left over. */

			if (left_over) {
				p = reinterpret_cast<UCHAR*>(control->ctl_data[1]) + l - 1;
				MOVE_FAST(left_over, p, left_length);
				control->ctl_data[0] += left_length;
			}
			return FB_SUCCESS;
		}

		/* replace unprintable characters */

		if (*p >= sizeof(char_tab) || !char_tab[*p])
			*p = '.';
	}

/* couldn't find a newline, return what there is, saving what was left
   in control buffer and setting return status based on existence of
   left over or whether or not control buffer did not hold end of segment */

	control->ctl_segment_length = buffer_used;
	if (left_over) {
		MOVE_FAST(left_over, (void *) control->ctl_data[1], left_length);
		control->ctl_data[0] = left_length;
		return isc_segment;
	}
	else {
		control->ctl_data[0] = 0;
		return control->ctl_data[2];
	}
}


ISC_STATUS filter_transliterate_text(USHORT action, ctl* control)
{
/**************************************
 *
 *	f i l t e r _ t r a n s l i t e r a t e _ t e x t
 *
 **************************************
 *
 * Functional description
 *	Get next segment from a text blob.  
 *	Convert the text from one character set to another.
 *
 *	The usage of the variable ctl_data slots:
 *	    ctl_data [0]	Pointer to ctlaux structure below,
 *
 **************************************/
	struct ctlaux {
		CsConvert ctlaux_obj1;	/* Intl object that does tx for us */
		BYTE *ctlaux_buffer1;	/* Temporary buffer for transliteration */
		ctl* ctlaux_subfilter;	/* For chaining transliterate filters */
		ISC_STATUS ctlaux_source_blob_status;	/* marks when source is EOF, etc */
		USHORT ctlaux_buffer1_len;	/* size of ctlaux_buffer1 in bytes */
		USHORT ctlaux_expansion_factor;	/* factor for text expand/contraction */
		USHORT ctlaux_buffer1_unused;	/* unused bytes in ctlaux_buffer1 */
	};

	thread_db* tdbb = NULL;
/* Note: Cannot pass tdbb without API change to user filters */

#define EXP_SCALE		128		/* to keep expansion non-floating */

	ctlaux* aux = (ctlaux*) control->ctl_data[0];
	
	ctl* source;
	ISC_STATUS status;
	SSHORT err_code;
	USHORT err_position;
	SSHORT source_cs, dest_cs;
	SSHORT i;
	USHORT result_length;

	switch (action) {
	case ACTION_open:
	case ACTION_create:
		for (i = 0; i < FB_NELEM(control->ctl_data); i++)
			control->ctl_data[i] = 0;
		aux = NULL;

		source = control->ctl_source_handle;
		control->ctl_number_segments = source->ctl_number_segments;

		source_cs = control->ctl_from_sub_type;
		dest_cs = control->ctl_to_sub_type;

		aux = (ctlaux*) gds__alloc((SLONG) sizeof(*aux));
		/* FREE: on ACTION_close in this routine */
		if (!aux)				/* NOMEM: */
			return isc_virmemexh;
#ifdef DEBUG_GDS_ALLOC
		/* BUG 7907: this is not freed in error cases */
		gds_alloc_flag_unfreed(aux);
#endif
		control->ctl_data[0] = (IPTR) aux;

		aux->ctlaux_source_blob_status = FB_SUCCESS;
		aux->ctlaux_buffer1_unused = 0;
		aux->ctlaux_expansion_factor = EXP_SCALE * 1;

		SET_TDBB(tdbb);
		aux->ctlaux_obj1 = INTL_convert_lookup(tdbb, dest_cs, source_cs);
		if (aux->ctlaux_obj1 == NULL) {
			/* Do the convert the hard way, via Unicode.
			   In this case, we become the filter from Unicode to
			   the destination format.  And we setup as our source
			   a filter from <source> to <Unicode>.
			   recursively using the same routine. */

			if (action == ACTION_open) {
				aux->ctlaux_obj1 =
					INTL_convert_lookup(tdbb, dest_cs, CS_UNICODE_UCS2);
			}
			else {
				aux->ctlaux_obj1 =
					INTL_convert_lookup(tdbb, CS_UNICODE_UCS2, source_cs);
			}

			if (aux->ctlaux_obj1 == NULL) {
				control->ctl_status[0] = isc_arg_gds;
				control->ctl_status[1] = isc_text_subtype;
				control->ctl_status[2] = isc_arg_number;
				control->ctl_status[3] = dest_cs;
				control->ctl_status[4] = isc_arg_end;
				return isc_text_subtype;
			}

			// ISC_STATUS to pointer!
			aux->ctlaux_subfilter =
				(ctl*) caller(ACTION_alloc, control, 0, NULL, NULL);

			/* This is freed in BLF_close_filter */

			*(aux->ctlaux_subfilter) = *control;

			control->ctl_source_handle = aux->ctlaux_subfilter;
			control->ctl_source = filter_transliterate_text;
			source = control->ctl_source_handle;

			if (action == ACTION_open) {
				control->ctl_from_sub_type = CS_UNICODE_UCS2;
				aux->ctlaux_subfilter->ctl_to_sub_type = CS_UNICODE_UCS2;
			}
			else {
				control->ctl_to_sub_type = CS_UNICODE_UCS2;
				aux->ctlaux_subfilter->ctl_from_sub_type = CS_UNICODE_UCS2;
			}


			/* Now that the new filter has been inserted, tell it to open */

			status = caller(action, control, 0, NULL, NULL);
			if (status)
				return status;
		}

		if (action == ACTION_open) {
			control->ctl_max_segment =
				aux->ctlaux_obj1.convert(NULL, 0, NULL,
						source->ctl_max_segment, &err_code, &err_position);

			if (source->ctl_max_segment && control->ctl_max_segment)
				aux->ctlaux_expansion_factor =
					(EXP_SCALE * control->ctl_max_segment) /
					source->ctl_max_segment;
			else
				aux->ctlaux_expansion_factor = (EXP_SCALE * 1);

			fb_assert(aux->ctlaux_expansion_factor != 0);

			control->ctl_total_length =
				source->ctl_total_length * aux->ctlaux_expansion_factor /
				EXP_SCALE;

			aux->ctlaux_buffer1_len =
				MAX(control->ctl_max_segment, source->ctl_max_segment);
			aux->ctlaux_buffer1_len =
				MAX(aux->ctlaux_buffer1_len,
					(80 * aux->ctlaux_expansion_factor) / EXP_SCALE);
		}
		else {					/* ACTION_create */
			/* In a create, the source->ctl_max_segment size isn't set (as
			 * nothing has been written!).  Therefore, take a best guess
			 * for an appropriate buffer size, allocate that, and re-allocate
			 * later if we guess wrong.
			 */
			const USHORT tmp = aux->ctlaux_obj1.convert( NULL, 0, NULL,
							128, &err_code, &err_position);
			aux->ctlaux_expansion_factor = (EXP_SCALE * tmp) / 128;

			fb_assert(aux->ctlaux_expansion_factor != 0);

			aux->ctlaux_buffer1_len =
				80 * aux->ctlaux_expansion_factor / EXP_SCALE;
		}

		/* Allocate the temporary buffer  - make sure it is big enough for
		 * either source or destination, and at least 80 bytes in size
		 */

		aux->ctlaux_buffer1_len = MAX(aux->ctlaux_buffer1_len, 80);
		fb_assert(aux->ctlaux_buffer1_len != 0);

		aux->ctlaux_buffer1 =
			(BYTE *) gds__alloc((SLONG) aux->ctlaux_buffer1_len);
		/* FREE: on ACTION_close in this procedure */
		if (!aux->ctlaux_buffer1)	/* NOMEM: */
			return isc_virmemexh;

#ifdef DEBUG_GDS_ALLOC
		/* BUG 7907: this is not freed in error cases */
		gds_alloc_flag_unfreed(aux->ctlaux_buffer1);
#endif

		return FB_SUCCESS;

	case ACTION_close:
		if (aux && aux->ctlaux_buffer1) {
			gds__free((SLONG *) aux->ctlaux_buffer1);
			aux->ctlaux_buffer1 = NULL;
			aux->ctlaux_buffer1_len = 0;
		}
		if (aux) {
			gds__free(aux);
			control->ctl_data[0] = 0;
			aux = NULL;
		}
		return FB_SUCCESS;

	case ACTION_get_segment:
		/* Fall through to handle get_segment below */
		break;

	case ACTION_put_segment:
		/* Now convert from the input buffer into the temporary buffer */

		/* How much space do we need to convert? */
		result_length = aux->ctlaux_obj1.convert(NULL, 0,
			control->ctl_buffer, control->ctl_buffer_length,
			&err_code, &err_position);

		/* Allocate a new buffer if we don't have enough */
		if (result_length > aux->ctlaux_buffer1_len) {
			gds__free((SLONG *) aux->ctlaux_buffer1);
			aux->ctlaux_buffer1_len = result_length;
			aux->ctlaux_buffer1 = (BYTE *) gds__alloc((SLONG) result_length);
			/* FREE: above & ACTION_close in this routine */
			if (!aux->ctlaux_buffer1)	/* NOMEM: */
				return isc_virmemexh;
		}

		/* convert the text */

		result_length = aux->ctlaux_obj1.convert(aux->ctlaux_buffer1,
			aux->ctlaux_buffer1_len, control->ctl_buffer,
			control->ctl_buffer_length, &err_code, &err_position);

		if (err_code)
			return isc_transliteration_failed;

		/* hand the text off to the next stage of the filter */

		status = caller(ACTION_put_segment, control, result_length,
						aux->ctlaux_buffer1, NULL);

		if (status)
			return status;

		/* update local control variables for segment length */

		if (result_length > control->ctl_max_segment)
			control->ctl_max_segment = result_length;

		control->ctl_total_length += result_length;
		control->ctl_number_segments++;

		return FB_SUCCESS;

	case ACTION_seek:
		return isc_uns_ext;

	case ACTION_alloc:
	case ACTION_free:
		return FB_SUCCESS;

	default:
		BUGCHECK(289);			/* Unknown blob filter ACTION */
		return isc_uns_ext;
	}

/* Drop thru for ACTION_get_segment. */

/* Do we already have enough bytes in temp buffer to fill output buffer? */

	bool can_use_more;
	USHORT length = aux->ctlaux_buffer1_unused;
	if (length) {
		if (control->ctl_buffer_length <
			(length * aux->ctlaux_expansion_factor / EXP_SCALE)) {
			/* No need to fetch more bytes, we have enough pending */
			can_use_more = false;
		}
		else
			can_use_more = true;

		/* Always keep a minimal count of bytes in the input buffer, 
		 * to prevent the case of truncated characters.
		 */
		if (length < 3)
			can_use_more = true;
	}

/* Load data into the temporary buffer if,
   a) we've finished the current segment (no data left in buffer)
   b) We haven't finished reading the current segment from the
      source and could use more data for the forthcoming convert
      (We don't want to blindly keep topping off this buffer if we
       already have more than we can use) */

	if (!length || can_use_more
		&& (aux->ctlaux_source_blob_status == isc_segment))
	{
		// Get a segment, or partial segment, from the source
		// into the temporary buffer

		USHORT bytes_read_from_source = 0;
		status = caller(ACTION_get_segment,
						control,
						(USHORT) (aux->ctlaux_buffer1_len - length),
						aux->ctlaux_buffer1 + length,
						&bytes_read_from_source);
		if (status == isc_segment)	/* source has more segment bytes */
			aux->ctlaux_source_blob_status = status;
		else if (status == isc_segstr_eof) {	/* source blob is finished */
			if (length == 0)	/* are we done too? */
				return isc_segstr_eof;
			aux->ctlaux_source_blob_status = FB_SUCCESS;
		}
		else if (status)		/* general error */
			return status;
		else					/* complete segment in buffer */
			aux->ctlaux_source_blob_status = FB_SUCCESS;
		length += bytes_read_from_source;
	}

/* Now convert from the temporary buffer into the destination buffer */

	result_length = aux->ctlaux_obj1.convert(control->ctl_buffer,
					control->ctl_buffer_length, aux->ctlaux_buffer1, length,
					&err_code, &err_position);

	if (err_code == CS_CONVERT_ERROR)
		return isc_transliteration_failed;

	if (err_code == CS_BAD_INPUT) {
		/* Bad input *might* be due to input buffer truncation in the middle
		   of a character, so shuffle bytes, add some more data, and try again.
		   If we already tried that then it's really some bad input */

		if (err_position == 0)
			return isc_transliteration_failed;
	}
	
	const USHORT unused_len = (err_code == 0) ?	0 : length - err_position;
	control->ctl_segment_length = result_length;
	if (unused_len) {
		move((TEXT *) (aux->ctlaux_buffer1 + err_position),
			 (TEXT *) aux->ctlaux_buffer1, unused_len);
	}
	aux->ctlaux_buffer1_unused = unused_len;

/* update local control variables for segment length */

	if (result_length > control->ctl_max_segment)
		control->ctl_max_segment = result_length;

/* No need up update ctl_number_segments as this filter doesn't change it */
/* No need to update ctl_total_length as we calculated an estimate on entry */

/* see if we still have data we couldn't send to application */

	if (unused_len)
		return isc_segment;		/* can't fit all data into user buffer */

/* We handed back all our data, but did we GET all the data 
 * from the source?
 */

	return (aux->ctlaux_source_blob_status == isc_segment) ?
		isc_segment : FB_SUCCESS;
}


ISC_STATUS filter_trans(USHORT action, ctl* control)
{
/**************************************
 *
 *	f i l t e r _ t r a n s
 *
 **************************************
 *
 * Functional description
 *	Pretty print a transaction description.
 *
 **************************************/
	if (action != ACTION_open)
		return string_filter(action, control);

/* Initialize for retrieval */
	UCHAR buffer[512];
	const SLONG l = control->ctl_source_handle->ctl_total_length;
	UCHAR* const temp =
		(l <= (SLONG) sizeof(buffer)) ? buffer : (UCHAR*) gds__alloc((SLONG) l);
	UCHAR* p = temp;
/* FREE: at procedure exit */
	if (!p)						/* NOMEM: */
		return isc_virmemexh;

	USHORT length;
	const ISC_STATUS status =
		caller(ACTION_get_segment, control, (USHORT) l, temp, &length);

	if (!status) {
        TEXT line[256];
		sprintf(line, "Transaction description version: %d", (int) *p++);
		string_put(control, line);
		TEXT* out = line;
		const UCHAR* const end = temp + length;

		while (p < end) {
			const UCHAR c = *p++;
			length = *p++;
			switch (c) {
			case TDR_HOST_SITE:
				sprintf(out, "Host site: %.*s", length, p);
				break;

			case TDR_DATABASE_PATH:
				sprintf(out, "Database path: %.*s", length, p);
				break;

			case TDR_REMOTE_SITE:
				sprintf(out, "    Remote site: %.*s", length, p);
				break;

			case TDR_TRANSACTION_ID:
				{
					const SLONG id = gds__vax_integer(p, length);
					sprintf(out, "    Transaction id: %ld", id);
					break;
				}

			default:
				sprintf(out, "item %d not understood", (int) p[-1]);
				string_put(control, line);
				goto break_out;
			}
			string_put(control, line);
			p = p + length;
		}
	}

  break_out:
	control->ctl_data[1] = control->ctl_data[0];

	if (temp != buffer)
		gds__free(temp);

	return FB_SUCCESS;
}


static ISC_STATUS caller(
					 USHORT action,
					 ctl* control,
					 USHORT buffer_length,
					 UCHAR* buffer, USHORT* return_length)
{
/**************************************
 *
 *	c a l l e r
 *
 **************************************
 *
 * Functional description
 *	Call next source filter.
 *
 **************************************/
	ctl* source = control->ctl_source_handle;
	source->ctl_status = control->ctl_status;
	source->ctl_buffer = buffer;
	source->ctl_buffer_length = buffer_length;

	// Warning: it will be pointer to ISC_STATUS when action == ACTION_alloc.
	const ISC_STATUS status = (*source->ctl_source) (action, source);

	if (return_length)
		*return_length = source->ctl_segment_length;

	return status;
}


static void dump_blr(void* arg, SSHORT offset, const char* line)
{
/**************************************
 *
 *	d u m p _ b l r
 *
 **************************************
 *
 * Functional description
 *	Callback routine for BLR dumping.
 *
 **************************************/
	ctl* control = reinterpret_cast<ctl*>(arg);
	TEXT buffer[256];

	const SLONG l = (USHORT) control->ctl_data[3] + strlen(line);
	TEXT* const temp = (l < (SLONG) sizeof(buffer)) ?
		buffer : (TEXT*) gds__alloc((SLONG) l);
/* FREE: at procedure exit */
	if (!temp) {				/* NOMEM: */
		/* No memory left - ignore the padding spaces and put the data */
		string_put(control, line);
		return;
	}

/* Pad out to indent length with spaces */
	TEXT* p = temp;
	for (const TEXT* const end = p + (USHORT) control->ctl_data[3]; p < end;)
		*p++ = ' ';

	sprintf(p, "%s", line);
	string_put(control, temp);

	if (temp != buffer)
		gds__free(temp);
}


static void move(const char* from, char* to, USHORT length)
{
/**************************************
 *
 *	m o v e
 *
 **************************************
 *
 * Functional description
 *	Move some bytes.
 *
 **************************************/
	if (length)
		do {
			*to++ = *from++;
		} while (--length);
}


static ISC_STATUS string_filter(USHORT action, ctl* control)
{
/**************************************
 *
 *	s t r i n g _ f i l t e r
 *
 **************************************
 *
 * Functional description
 *	Handle stuff for batched string filters.  This is used by filter_blr
 *	and filter_acl.
 *
 **************************************/
	TMP string;
	USHORT length;

	switch (action) {
	case ACTION_close:
		while (string = (TMP) control->ctl_data[0]) {
			control->ctl_data[0] = (IPTR) string->tmp_next;
			gds__free(string);
		}
		return FB_SUCCESS;

	case ACTION_get_segment:
		if (!(string = (TMP) control->ctl_data[1]))
			return isc_segstr_eof;
		length = string->tmp_length - control->ctl_data[2];
		if (length > control->ctl_buffer_length)
			length = control->ctl_buffer_length;
		move(string->tmp_string + (USHORT) control->ctl_data[2],
			 reinterpret_cast < char *>(control->ctl_buffer), length);
		control->ctl_data[2] += length;
		if (control->ctl_data[2] == string->tmp_length) {
			control->ctl_data[1] = (IPTR) string->tmp_next;
			control->ctl_data[2] = 0;
		}
		control->ctl_segment_length = length;
		return (length <= control->ctl_buffer_length) ? FB_SUCCESS : isc_segment;

	case ACTION_put_segment:
	case ACTION_create:
	case ACTION_seek:
	case ACTION_open:
		return isc_uns_ext;

	case ACTION_alloc:
	case ACTION_free:
		return FB_SUCCESS;

	default:
		BUGCHECK(289);			/* Unknown blob filter ACTION */
		return isc_uns_ext;
	}
}


static void string_put(ctl* control, const char* line)
{
/**************************************
 *
 *	s t r i n g _ p u t
 *
 **************************************
 *
 * Functional description
 *	Add a line of string to a string formatted blob.
 *
 **************************************/
	const USHORT l = strlen(line);
	TMP string = (TMP) gds__alloc((SLONG) (sizeof(tmp) + l));
/* FREE: on ACTION_close in string_filter() */
	if (!string) {				/* NOMEM: */
		fb_assert(FALSE);			/* out of memory */
		return;					/* & No error handling at this level */
	}
	string->tmp_next = NULL;
	string->tmp_length = l;
	move(line, string->tmp_string, l);

	TMP prior;
	if (prior = (TMP) control->ctl_data[1])
		prior->tmp_next = string;
	else
		control->ctl_data[0] = (IPTR) string;

	control->ctl_data[1] = (IPTR) string;
	++control->ctl_number_segments;
	control->ctl_total_length += l;
	control->ctl_max_segment = MAX(control->ctl_max_segment, l);
}

