/*
 *	PROGRAM:	InterBase International support
 *	MODULE:		cs_utffss.c
 *	DESCRIPTION:	Character set definition for Unicode FSS format
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

 /* Note: all routines have cousins in jrd/intl.c */

USHORT CS_UTFFSS_fss_to_unicode(CSCONVERT obj, UNICODE *dest_ptr, USHORT dest_len, NCHAR *src_ptr
								, USHORT src_len, SSHORT *err_code, USHORT *err_position);

USHORT CS_UTFFSS_unicode_to_fss(CSCONVERT obj, MBCHAR *fss_str, USHORT fss_len, UNICODE *unicode_str,
								USHORT unicode_len, SSHORT *err_code, USHORT *err_position);

SSHORT CS_UTFFSS_fss_mbtowc(TEXTTYPE *obj, UCS2_CHAR *wc, NCHAR *p, USHORT n);

typedef struct {
	int cmask;
	int cval;
	int shift;
	long lmask;
	long lval;
} Tab;

static const Tab tab[] = {
	{ 0x80, 0x00, 0 * 6, 0x7F, 0 },	/* 1 byte sequence */
	{ 0xE0, 0xC0, 1 * 6, 0x7FF, 0x80 },	/* 2 byte sequence */
	{ 0xF0, 0xE0, 2 * 6, 0xFFFF, 0x800 },	/* 3 byte sequence */
	{ 0xF8, 0xF0, 3 * 6, 0x1FFFFF, 0x10000 },	/* 4 byte sequence */
	{ 0xFC, 0xF8, 4 * 6, 0x3FFFFFF, 0x200000 },	/* 5 byte sequence */
	{ 0xFE, 0xFC, 5 * 6, 0x7FFFFFFF, 0x4000000 },	/* 6 byte sequence */
	{ 0, 0, 0, 0, 0}						/* end of table    */
};
