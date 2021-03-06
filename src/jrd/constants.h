/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		constants.h
 *	DESCRIPTION:	Misc system constants
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
 * 2001.10.08 Claudio Valderrama: frb_sysflag enum with numbering
 *   for automatically created triggers that aren't system triggers.
 */

#ifndef JRD_CONSTANTS_H
#define JRD_CONSTANTS_H

/* BLOb Subtype definitions */

/* Subtypes < 0  are user defined 
 * Subtype  0    means "untyped" 
 * Subtypes > 0  are InterBase defined 
 */

#define BLOB_untyped	0

/* InterBase defined BLOb subtypes */

#define BLOB_text	1
#define BLOB_blr	2
#define BLOB_acl	3
#define BLOB_ranges	4
#define BLOB_summary	5
#define BLOB_format	6
#define BLOB_tra	7
#define BLOB_extfile	8



/* Column Limits */

const ULONG MAX_COLUMN_SIZE	= 32767;	/* Bytes */

/* Misc constant values */

const int USERNAME_LENGTH	= 31;	/* Bytes */

static const int MAX_SQL_IDENTIFIER_SIZE = 32;
static const int MAX_SQL_IDENTIFIER_LEN = 31;
typedef TEXT SqlIdentifier[MAX_SQL_IDENTIFIER_SIZE];

static const char* NULL_ROLE = "NONE";

static const char* PRIMARY_KEY		= "PRIMARY KEY";
static const char* FOREIGN_KEY		= "FOREIGN KEY";
static const char* UNIQUE_CNSTRT		= "UNIQUE";
static const char* CHECK_CNSTRT		= "CHECK";
static const char* NOT_NULL_CNSTRT	= "NOT NULL";

/* literal strings in rdb$ref_constraints to be used to identify
   the cascade actions for referential constraints. Used
   by isql/show and isql/extract for now. */

static const char* RI_ACTION_CASCADE = "CASCADE";
static const char* RI_ACTION_NULL    = "SET NULL";
static const char* RI_ACTION_DEFAULT = "SET DEFAULT";
static const char* RI_ACTION_NONE    = "NO ACTION";
static const char* RI_RESTRICT       = "RESTRICT";

static const char* IMPLICIT_DOMAIN_PREFIX = "RDB$";
static const int IMPLICIT_DOMAIN_PREFIX_LEN = 4;

static const char* SQL_SECCLASS_PREFIX = "SQL$";
static const int SQL_SECCLASS_PREFIX_LEN = 4;


/******************************************/
/* System flag meaning - mainly Firebird. */
/******************************************/

enum frb_sysflag {
	frb_sysflag_user = 0,
	frb_sysflag_system = 1,
	frb_sysflag_qli = 2,
	frb_sysflag_check_constraint = 3,
	frb_sysflag_referential_constraint = 4,
	frb_sysflag_view_check = 5
};


/* UDF Arguments are numbered from 0 to MAX_UDF_ARGUMENTS --
   argument 0 is reserved for the return-type of the UDF */

const int MAX_UDF_ARGUMENTS	= 10;

// Maximum length of single line returned from pretty printer
const int PRETTY_BUFFER_SIZE = 1024;

const int MAX_INDEX_SEGMENTS = 16;

// Maximum index key length
// AB: If the maximum key-size will change, don't forget dyn.h and dba.epp 
// which cannot use these defines.
const int MAX_KEY			= 4096;		// Maximum page size possible divide by 4 (16384 / 4)
const int MAX_KEY_PRE_ODS11	= 255;		// Max key-size before ODS11

const char SQL_MATCH_1_CHAR		= '_';	/* Not translatable */
const char SQL_MATCH_ANY_CHARS	= '%';	/* Not translatable */

const size_t MAX_SORT_ITEMS = 255; // ORDER BY f1,...,f255

const int MAX_CONTEXT_VARS	= 1000;		// Maximum number of context variables allowed for a single object

#endif /* JRD_CONSTANTS_H */

