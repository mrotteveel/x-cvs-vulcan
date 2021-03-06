/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		ods.h
 *	DESCRIPTION:	On disk structure definitions
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
 *
 * 2000.11.29 Patrick J. P. Griffin: fixed bug SF #116733
 *	Add typedef struct gpg to properly document the layout of the generator page
 * 2002.08.26 Dmitry Yemanov: minor ODS change (new indices on system tables)
 *
 * 2002.10.29 Sean Leyne - Removed obsolete "Netware" port
 *
 * 2002.10.30 Sean Leyne - Removed support for obsolete "PC_PLATFORM" define
 *
 */

#ifndef JRD_ODS_H
#define JRD_ODS_H

#include "../jrd/RecordNumber.h"

/*  This macro enables the ability of the engine to connect to databases
 *  from ODS 8 up to the latest.  If this macro is undefined, the engine
 *  only opens a database of the current ODS major version.
 */
#define ODS_8_TO_CURRENT

/**********************************************************************
**
** NOTE:
**
**   ODS 5 was shipped with version 3.3 but no longer supported
**   ODS 6 and ODS 7 never went out the door
**   ODS 8 was shipped with version 4.0
**   ODS 9 is going to be shipped with version 4.5
**
***********************************************************************/

/* ODS major version -- major versions are not compatible */

const USHORT ODS_VERSION6	= 6;		/* on-disk structure as of v3.0 */
const USHORT ODS_VERSION7	= 7;		/* new on disk structure for fixing index bug */
const USHORT ODS_VERSION8	= 8;		/* new btree structure to support pc semantics */
const USHORT ODS_VERSION9	= 9;		/* btree leaf pages are always propogated up */
const USHORT ODS_VERSION10	= 10;		/* V6.0 features. SQL delimited idetifier,
									SQLDATE, and 64-bit exact numeric type */
const USHORT ODS_VERSION11	= 11;		/* Firebird 2.0 features */

/* ODS minor version -- minor versions ARE compatible, but may be
   increasingly functional.  Add new minor versions, but leave previous
   names intact */

/* Minor versions for ODS 6 */

const USHORT ODS_GRANT6		= 1;		/* adds fields for field level grant */
const USHORT ODS_INTEGRITY6	= 2;		/* adds fields for referential integrity */
const USHORT ODS_FUNCTIONS6	= 3;		/* modifies type of RDB$MODULE_NAME field */
const USHORT ODS_SQLNAMES6	= 4;		/* permits SQL security on > 27 SCHAR names */
const USHORT ODS_CURRENT6	= 4;

/* Minor versions for ODS 7 */

const USHORT ODS_FUNCTIONS7	= 1;		/* modifies type of RDB$MODULE_NAME field */
const USHORT ODS_SQLNAMES7	= 2;		/* permits SQL security on > 27 SCHAR names */
const USHORT ODS_CURRENT7	= 2;

/* Minor versions for ODS 8 */

const USHORT ODS_CASCADE_RI8	= 1;		/* permits cascading referential integrity */
				/* ODS 8.2 is the same as ODS 8.1 */
const USHORT ODS_CURRENT8		= 2;

/* Minor versions for ODS 9 */

const USHORT ODS_CURRENT_9_0	= 0;		/* SQL roles & Index garbage collection */
const USHORT ODS_SYSINDEX9		= 1;		/* Index on RDB$CHECK_CONSTRAINTS (RDB$TRIGGER_NAME) */
const USHORT ODS_CURRENT9		= 1;

/* Minor versions for ODS 10 */

const USHORT ODS_CURRENT10_0	= 0;		/* V6.0 features. SQL delimited identifier,
										SQLDATE, and 64-bit exact numeric type */
const USHORT ODS_SYSINDEX10	= 1;		/* New system indices */
const USHORT ODS_CURRENT10		= 1;

/* Minor versions for ODS 11 */

const USHORT ODS_CURRENT11_0	= 0;		/* Firebird 2.0 features. */
const USHORT ODS_CURRENT11		= 0;

/* useful ODS macros. These are currently used to flag the version of the
   system triggers and system indices in ini.e */

inline USHORT ENCODE_ODS(USHORT major, USHORT minor) {
	return (((major) << 4) | (minor));
}

const USHORT ODS_8_0		= ENCODE_ODS(ODS_VERSION8, 0);
const USHORT ODS_8_1		= ENCODE_ODS(ODS_VERSION8, 1);
const USHORT ODS_9_0		= ENCODE_ODS(ODS_VERSION9, 0);
const USHORT ODS_9_1		= ENCODE_ODS(ODS_VERSION9, 1);
const USHORT ODS_10_0		= ENCODE_ODS(ODS_VERSION10, 0);
const USHORT ODS_10_1		= ENCODE_ODS(ODS_VERSION10, 1);
const USHORT ODS_11_0		= ENCODE_ODS(ODS_VERSION11, 0);

static const int ODS_FIREBIRD_FLAG	= 0x8000;

/* Decode ODS version to Major and Minor parts. The 4 LSB's are minor and 
   the next 11 bits are major version number */
   
inline USHORT DECODE_ODS_MAJOR(USHORT ods_version) {
	return ((ods_version & 0x7FF0) >> 4);
}

inline USHORT DECODE_ODS_MINOR(USHORT ods_version) {
	return (ods_version & 0x000F);
}

inline USHORT DECODE_FIREBIRD_FLAG(USHORT ods_version) {
	return ((ods_version & ODS_FIREBIRD_FLAG) >> 15);
}

/* Set current ODS major and minor version */

const USHORT ODS_VERSION	= ODS_VERSION11;	/* current ods major version -- always 
											the highest */
const USHORT ODS_CURRENT	= ODS_CURRENT11;	/* the highest defined minor version
											number for this ODS_VERSION!! */
const USHORT ODS_CURRENT_VERSION	= ODS_11_0;		/* Current ODS version in use which includes 
												both Major and Minor ODS versions! */
// ODS types.
//const USHORT ODS_TYPE_MASK		= 0xFF00;
const USHORT ODS_TYPE_INTERBASE	= 0x0000; // Interbase ODS (we support it up to ODS10)
const USHORT ODS_TYPE_FIREBIRD	= 0x0100; // Official Firebird ODS (used since ODS11)
const USHORT ODS_TYPE_TRANSIENT	= 0x0200; // Firebird ODS with some unfinished changes
const USHORT ODS_TYPE_PRIVATE_0	= 0x8000; // ODS used for private versions, such as BroadView builds


// ODS types in range 0x80-0xFF are reserved for private builds and forks

const USHORT ODS_TYPE_CURRENT	= ODS_TYPE_TRANSIENT;
//const USHORT ODS_TYPE_CURRENT	= ODS_TYPE_PRIVATE_0;

const USHORT USER_REL_INIT_ID_ODS8		= 31;	/* ODS <= 8 */
const USHORT USER_DEF_REL_INIT_ID		= 128;	/* ODS >= 9 */

/***
static inline bool ODS_SUPPORTED(USHORT hdr_version) {
	USHORT ods_version = hdr_version & ~ODS_TYPE_MASK;
	USHORT ods_type = hdr_version & ODS_TYPE_MASK;

#ifdef ODS_8_TO_CURRENT
	// Support Interbase ODS from 8 to 10;
	if (ods_type == ODS_TYPE_INTERBASE)
		return ods_version >= ODS_VERSION8 && ods_version <= ODS_VERSION10;

	// This is for future ODS versions
	if (ods_type == ODS_TYPE_FIREBIRD &&
		ods_version > ODS_VERSION10 && ods_version <= ODS_VERSION - 1)
		return true;
#endif

	// Support current ODS of the engine
	if (ods_type == ODS_TYPE_CURRENT && ods_version == ODS_VERSION)
		return true;

	// Do not support anything else
	return false;
}
***/

/* Page types */

const SCHAR pag_undefined		= 0;
const SCHAR pag_header		= 1;		/* Database header page */
const SCHAR pag_pages			= 2;		/* Page inventory page */
const SCHAR pag_transactions	= 3;		/* Transaction inventory page */
const SCHAR pag_pointer		= 4;		/* Pointer page */
const SCHAR pag_data			= 5;		/* Data page */
const SCHAR pag_root			= 6;		/* Index root page */
const SCHAR pag_index			= 7;		/* Index (B-tree) page */
const SCHAR pag_blob			= 8;		/* Blob data page */
const SCHAR pag_ids			= 9;		/* Gen-ids */
const SCHAR pag_log			= 10;		// Write ahead log information DEPRECATED
const SCHAR pag_max			= 10;		/* Max page type */

const SCHAR HEADER_PAGE		= 0;
const SCHAR LOG_PAGE			= 2;

const USHORT MIN_PAGE_SIZE		= 1024;
const USHORT MAX_PAGE_SIZE		= 16384;
const USHORT DEFAULT_PAGE_SIZE	= 4096;

/* Basic page header */

struct pag 
{
	SCHAR pag_type;
	SCHAR pag_flags;
	USHORT pag_checksum;
	ULONG pag_generation;
	// We renamed pag_seqno for SCN number usage to avoid major ODS version bump
	ULONG pag_scn;			/* WAL seqno of last update */
	ULONG reserved;			/* Was used for WAL */
};

typedef pag *PAG;

/* Blob page */

struct blob_page 
{
	struct pag blp_header;
	SLONG blp_lead_page;		/* First page of blob (for redundancy only) */
	SLONG blp_sequence;			/* Sequence within blob */
	USHORT blp_length;			/* Bytes on page */
	USHORT blp_pad;				/* Unused */
	SLONG blp_page[1];			/* Page number if level 1 */
};

#define blp_data	blp_page	/* Formerly defined field */
#define BLP_SIZE	OFFSETA(blob_page*, blp_page)

const SCHAR blp_pointers	= 1;		/* Blob pointer page, not data page */


/* B-tree node */

struct btree_nod
{
	UCHAR btn_prefix;			/* size of compressed prefix */
	UCHAR btn_length;			/* length of data in node */
	UCHAR btn_number[4];		/* page or record number */
	UCHAR btn_data[1];
};

const int BTN_SIZE	= 6;

// B-tree page ("bucket")
struct btree_page 
{
	struct pag btr_header;
	SLONG btr_sibling;			// right sibling page
	SLONG btr_left_sibling;		// left sibling page
	SLONG btr_prefix_total;		// sum of all prefixes on page
	USHORT btr_relation;		// relation id for consistency
	USHORT btr_length;			// length of data in bucket
	UCHAR btr_id;				// index id for consistency
	UCHAR btr_level;			// index level (0 = leaf)
	struct btree_nod btr_nodes[1];
};

// Firebird B-tree nodes
const USHORT BTN_LEAF_SIZE	= 6;
const USHORT BTN_PAGE_SIZE	= 10;

struct IndexJumpInfo 
{
	USHORT firstNodeOffset;		// offset to node in page
	USHORT jumpAreaSize;		// size area before a new jumpnode is made
	UCHAR  jumpers;				// nr of jump-nodes in page, with a maximum of 255
};

// pag_flags
const SCHAR btr_dont_gc			= 1;	// Don't garbage-collect this page
const SCHAR btr_not_propogated	= 2;	// page is not propogated upward
const SCHAR btr_descending		= 8;	// Page/bucket is part of a descending index
const SCHAR btr_all_record_number	= 16;	// Non-leaf-nodes will contain record number information
const SCHAR btr_large_keys		= 32;	// AB: 2003-index-structure enhancement
const SCHAR btr_jump_info		= 64;	// AB: 2003-index-structure enhancement

const SCHAR BTR_FLAG_COPY_MASK = (btr_descending | btr_all_record_number | btr_large_keys | btr_jump_info);

/* Data Page */

struct data_page 
{
	struct pag dpg_header;
	SLONG dpg_sequence;			/* Sequence number in relation */
	USHORT dpg_relation;		/* Relation id */
	USHORT dpg_count;			/* Number of record segments on page */
	struct dpg_repeat {
		USHORT dpg_offset;		/* Offset of record fragment */
		USHORT dpg_length;		/* Length of record fragment */
	} dpg_rpt[1];
};

#ifdef __cplusplus
#define DPG_SIZE	(sizeof (data_page) - sizeof (data_page::dpg_repeat))
#else
#define DPG_SIZE	(sizeof (struct data_page) - sizeof(struct dpg_repeat))
#endif

// pag_flags
const SCHAR dpg_orphan	= 1;		/* Data page is NOT in pointer page */
const SCHAR dpg_full	= 2;		/* Pointer page is marked FULL */
const SCHAR dpg_large	= 4;		/* Large object is on page */


/* Index root page */

struct index_root_page 
{
	struct pag irt_header;
	USHORT irt_relation;		/* relation id (for consistency) */
	USHORT irt_count;			/* Number of indices */
	struct irt_repeat {
		SLONG irt_root;			/* page number of index root */
		union {
			float irt_selectivity;	/* selectivity of index - NOT USED since ODS11 */
			SLONG irt_transaction;	/* transaction in progress */
		} irt_stuff;
		USHORT irt_desc;		/* offset to key descriptions */
		UCHAR irt_keys;			/* number of keys in index */
		UCHAR irt_flags;
	} irt_rpt[1];
};

/* key descriptor */

struct irtd_ods10 {
	USHORT irtd_field;
	USHORT irtd_itype;
};

struct irtd : public irtd_ods10 {
	float irtd_selectivity;
};

typedef irtd IRTD;

const USHORT irt_unique		= 1;
const USHORT irt_descending	= 2;
const USHORT irt_in_progress = 4;
const USHORT irt_foreign	= 8;
const USHORT irt_primary	= 16;
const USHORT irt_expression	= 32;

#define STUFF_COUNT		4
//const int STUFF_COUNT		= 4;
const SLONG END_LEVEL		= -1;
const SLONG END_BUCKET		= -2;

/* Header page */

struct header_page 
{
	struct pag hdr_header;
	USHORT hdr_page_size;		/* Page size of database */
	USHORT hdr_ods_version;		/* Version of on-disk structure */
	SLONG hdr_PAGES;			/* Page number of PAGES relation */
	ULONG hdr_next_page;		/* Page number of next hdr page */
	SLONG hdr_oldest_transaction;	/* Oldest interesting transaction */
	SLONG hdr_oldest_active;	/* Oldest transaction thought active */
	SLONG hdr_next_transaction;	/* Next transaction id */
	USHORT hdr_sequence;		/* sequence number of file */
	USHORT hdr_flags;			/* Flag settings, see below */
	SLONG hdr_creation_date[2];	/* Date/time of creation */
	SLONG hdr_attachment_id;	/* Next attachment id */
	SLONG hdr_shadow_count;		/* Event count for shadow synchronization */
	SSHORT hdr_implementation;	/* Implementation number */
	USHORT hdr_ods_minor;		/* Update version of ODS */
	USHORT hdr_ods_minor_original;	/* Update version of ODS at creation */
	USHORT hdr_end;				/* offset of HDR_end in page */
	ULONG hdr_page_buffers;		/* Page buffers for database cache */
	SLONG hdr_bumped_transaction;	/* Bumped transaction id for log optimization */
	SLONG hdr_oldest_snapshot;	/* Oldest snapshot of active transactions */
	SLONG hdr_backup_pages; /* The amount of pages in files locked for backup */
	SLONG hdr_misc[3];			/* Stuff to be named later */
	UCHAR hdr_data[1];			/* Misc data */
};

#define HDR_SIZE        OFFSETA(header_page*, hdr_data)

/* Header page clumplets */

/* Data items have the format

	<type_byte> <length_byte> <data...>
*/

const UCHAR HDR_end				= 0;
const UCHAR HDR_root_file_name	= 1;	/* Original name of root file */
const UCHAR HDR_journal_server	= 2;	// Name of journal server
const UCHAR HDR_file			= 3;	/* Secondary file */
const UCHAR HDR_last_page		= 4;	/* Last logical page number of file */
const UCHAR HDR_unlicensed		= 5;	// Count of unlicensed activity
const UCHAR HDR_sweep_interval	= 6;	/* Transactions between sweeps */
const UCHAR HDR_log_name		= 7;	/* replay log name */
const UCHAR HDR_journal_file	= 8;	// Intermediate journal file
const UCHAR HDR_password_file_key	= 9;	/* Key to compare to password db */
const UCHAR HDR_backup_info		= 10;	// WAL backup information
const UCHAR HDR_cache_file		= 11;	// Shared cache file
const UCHAR HDR_difference_file	= 12;	/* Delta file that is used during backup lock */
const UCHAR HDR_backup_guid		= 13;	/* UID generated on each switch into backup mode */
const UCHAR HDR_max				= 14;	/* Maximum HDR_clump value */

/* Header page flags */

const USHORT hdr_active_shadow		= 0x1;	/* 1    file is an active shadow file */
const USHORT hdr_force_write		= 0x2;	/* 2    database is forced write */
const USHORT hdr_short_journal		= 0x4;	  // 4    short-term journalling
const USHORT hdr_long_journal		= 0x8;	  // 8    long-term journalling
const USHORT hdr_no_checksums		= 0x10;	/* 16   don't calculate checksums */
const USHORT hdr_no_reserve			= 0x20;	/* 32   don't reserve space for versions */
const USHORT hdr_disable_cache		= 0x40;	// 64   disable using shared cache file
const USHORT hdr_shutdown			= 0x80;	  // 128  database is shutdown
const USHORT hdr_SQL_dialect_3		= 0x100;	/* 256  database SQL dialect 3 */
const USHORT hdr_read_only			= 0x200;	/* 512  Database in ReadOnly. If not set, DB is RW */
/* backup status mask - see bit values in nbak.h */
const USHORT hdr_backup_mask		= 0xC00;
const USHORT hdr_shutdown_mask		= 0x1080;

// Values for shutdown mask
const USHORT hdr_shutdown_none		= 0x0;
const USHORT hdr_shutdown_multi		= 0x80;
const USHORT hdr_shutdown_full		= 0x1000;
const USHORT hdr_shutdown_single	= 0x1080;

typedef struct sfd {
	SLONG sfd_min_page;			/* Minimum page number */
	SLONG sfd_max_page;			/* Maximum page number */
	UCHAR sfd_index;			/* Sequence of secondary file */
	UCHAR sfd_file[1];			/* Given file name */
} SFD; 

/* Page Inventory Page */

struct page_inv_page 
{
	struct pag pip_header;
	SLONG pip_min;				/* Lowest (possible) free page */
	UCHAR pip_bits[1];
};


/* Pointer Page */

struct pointer_page 
{
	struct pag ppg_header;
	SLONG ppg_sequence;			/* Sequence number in relation */
	SLONG ppg_next;				/* Next pointer page in relation */
	USHORT ppg_count;			/* Number of slots active */
	USHORT ppg_relation;		/* Relation id */
	USHORT ppg_min_space;		/* Lowest slot with space available */
	USHORT ppg_max_space;		/* Highest slot with space available */
	SLONG ppg_page[1];			/* Data page vector */
};

#define ppg_bits	ppg_page

// pag_flags
const USHORT ppg_eof		= 1;			/* Last pointer page in relation */

/* Transaction Inventory Page */

struct tx_inv_page 
{
	struct pag tip_header;
	SLONG tip_next;				/* Next transaction inventory page */
	UCHAR tip_transactions[1];
};


/* Generator Page */

struct generator_page 
{
	struct pag gpg_header;
	SLONG gpg_sequence;			/* Sequence number */
	SLONG gpg_waste1;			/* overhead carried for backward compatibility */
	USHORT gpg_waste2;			/* overhead carried for backward compatibility */
	USHORT gpg_waste3;			/* overhead carried for backward compatibility */
	USHORT gpg_waste4;			/* overhead carried for backward compatibility */
	USHORT gpg_waste5;			/* overhead carried for backward compatibility */
	SINT64 gpg_values[1];		/* Generator vector */
};


/* Record header */

struct rhd {
	SLONG rhd_transaction;		/* transaction id */
	SLONG rhd_b_page;			/* back pointer */
	USHORT rhd_b_line;			/* back line */
	USHORT rhd_flags;			/* flags, etc */
	UCHAR rhd_format;			/* format version */
	UCHAR rhd_data[1];
};

typedef rhd *RHD;

#define RHD_SIZE	OFFSETA (rhd*, rhd_data)

/* Record header for fragmented record */

struct rhdf {
	SLONG rhdf_transaction;		/* transaction id */
	SLONG rhdf_b_page;			/* back pointer */
	USHORT rhdf_b_line;			/* back line */
	USHORT rhdf_flags;			/* flags, etc */
	UCHAR rhdf_format;			/* format version */
	SLONG rhdf_f_page;			/* next fragment page */
	USHORT rhdf_f_line;			/* next fragment line */
	UCHAR rhdf_data[1];			/* Blob data */
};

typedef rhdf *RHDF;

#define RHDF_SIZE	OFFSETA (rhdf*, rhdf_data)

/* Record header for blob header */

struct blh {
	SLONG blh_lead_page;		/* First data page number */
	SLONG blh_max_sequence;		/* Number of data pages */
	USHORT blh_max_segment;		/* Longest segment */
	USHORT blh_flags;
	UCHAR blh_level;			/* Number of address levels */
	SLONG blh_count;			/* Total number of segments */
	SLONG blh_length;			/* Total length of data */
	USHORT blh_sub_type;		/* Blob sub-type */
	USHORT blh_unused;
	SLONG blh_page[1];			/* Page vector for blob pages */
};

typedef blh *BLH;

#define blh_data	blh_page
#define BLH_SIZE	OFFSETA (blh*, blh_page)

// rhdf_flags

const USHORT rhd_deleted		= 1;		/* record is logically deleted */
const USHORT rhd_chain			= 2;		/* record is an old version */
const USHORT rhd_fragment		= 4;		/* record is a fragment */
const USHORT rhd_incomplete		= 8;		/* record is incomplete */
const USHORT rhd_blob			= 16;		/* isn't a record but a blob */
const USHORT rhd_stream_blob	= 32;		/* blob is a stream mode blob */
const USHORT rhd_delta			= 32;		/* prior version is differences only */
const USHORT rhd_large			= 64;		/* object is large */
const USHORT rhd_damaged		= 128;		/* object is known to be damaged */
const USHORT rhd_gc_active		= 256;		/* garbage collecting dead record version */

/* additions for write ahead log */

//const int CTRL_FILE_LEN		= 255;	/* Pre allocated size of file name */
const USHORT CLUMP_ADD			= 0;
const USHORT CLUMP_REPLACE		= 1;
const USHORT CLUMP_REPLACE_ONLY= 2;

/* Log page */

typedef struct ctrl_pt {
	SLONG cp_seqno;
	SLONG cp_offset;
	SLONG cp_p_offset;
	SSHORT cp_fn_length;
} CP;

struct log_info_page {
	struct pag log_header;
	SLONG log_flags;			/* flags            */
	CP log_cp_1;				/* control point 1      */
	CP log_cp_2;				/* control point 2      */
	CP log_file;				/* current file         */
	SLONG log_next_page;		/* Next log page        */
	SLONG log_mod_tip;			/* tip of modify transaction    */
	SLONG log_mod_tid;			/* transaction id of modify process */
	SLONG log_creation_date[2];	/* Date/time of log creation    */
	SLONG log_free[4];			/* some extra space for later use */
	USHORT log_end;				/* similar to hdr_end   */
	UCHAR log_data[1];
};

#define LIP_SIZE	OFFSETA (log_info_page*, log_data);

/* log flags */

#define log_recover		1		/* recovery required */
#define log_no_ail		2		/* dummy! will be used till df work */
#define log_delete		4		/* log files have been deleted */
#define log_add			8		/* log files have been added */
#define log_rec_in_progress	16	/* recovery is in progress */
#define log_partial_rebuild	32	/* partial recovery in progress */
#define log_recovery_done 	64	/* long term recovery just finished */

#define CTRL_FILE_LEN		255	/* Pre allocated size of file name */
#define CLUMP_ADD		0
#define CLUMP_REPLACE		1
#define CLUMP_REPLACE_ONLY	2

/* Log Clumplet types */

const UCHAR LOG_end			= HDR_end;
//const int LOG_ctrl_file1		1	// file name of 2nd last control pt
//const int LOG_ctrl_file2		2	// file name of last ctrl pt
//const int LOG_logfile			3	// Primary WAL file name
//const int LOG_backup_info		4	// Journal backup directory
//const int LOG_chkpt_len		5	// checkpoint length
//const int LOG_num_bufs		6	// Number of log buffers
//const int LOG_bufsize			7	// Buffer size
//const int LOG_grp_cmt_wait	8	// Group commit wait time
const int LOG_max			= 8;	// Maximum LOG_clump value

#endif /* JRD_ODS_H */

