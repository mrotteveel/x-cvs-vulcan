/*
 *	PROGRAM:	Interactive SQL utility
 *	MODULE:		isql.h
 *	DESCRIPTION:	Component wide include file
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
 * Revision 1.2  2000/11/18 16:49:24  fsg
 * Increased PRINT_BUFFER_LENGTH to 2048 to show larger plans
 * Fixed Bug #122563 in extract.e get_procedure_args
 * Apparently this has to be done in show.e also,
 * but that is for another day :-)
 *
 * 2002.10.30 Sean Leyne - Removed support for obsolete "PC_PLATFORM" define
 *
 */

#ifndef _ISQL_ISQL_H_
#define _ISQL_ISQL_H_

#include "../jrd/flags.h"
#include <stdlib.h>

// Define lengths used in isql.e 

/* Increased this to allow display of somewhat lengthy plans
   hope this will last a year or so :-)
   FSG 17.Nov.2000
*/
const int PRINT_BUFFER_LENGTH	= 2048;
const int MAXTERM_SIZE			= 32;	// SQL termination character
const int USER_LENGTH 			= 128;
const int PASSWORD_LENGTH		= 128;
const int ROLE_LENGTH			= 128;

/* these constants are purely idiotic; there's no point in having
   a predefined constant with no meaning, but that's Ed Simon the
   master programmer for you! */

#define BUFFER_LENGTH128	128
#define BUFFER_LENGTH155	155
#define BUFFER_LENGTH256	256
#define BUFFER_LENGTH360	360
#define BUFFER_LENGTH400	400
#define BUFFER_LENGTH512	512
#define BUFFER_LENGTH80		 80
#define BUFFER_LENGTH60		 60
#define BUFFER_LENGTH120	120
#define BUFFER_LENGTH180	180

/* Define the possible states of processing commands */

#define FOUND_EOF EOF
#define CONT		0
#define EXIT		1
#define BACKOUT		2
#define ERR		3
#define END		4
#define SKIP		5
#define FAIL		6
#define EXTRACT		7
#define EXTRACTALL	8
#define FETCH		9

// Which blob subtypes to print 

const int ALL_BLOBS	= -2;
const int NO_BLOBS	= -1;

// Flag to decode all vs sql only objects
enum LegacyTables
{
	SQL_objects,
	ALL_objects
};

/* Show source for triggers or not */
#define NO_SOURCE 	0
#define SHOW_SOURCE	1


#define WORDLENGTH	32
#define DEFTERM		";"
#define DEFCHARSET	"NONE"
#define NULL_DISP_LEN    6

#define NULL_PTR	((void*) 0)

// Error codes 

const int MSG_LENGTH	= 1024;
const int ISQL_MSG_FAC	= 17;

const int GEN_ERR 					= 0;	// General non-zero SQLCODE error 
const int SWITCH 					= 2;	// Bad command line arg
const int NO_DB 					= 3;	// No database specified 
const int FILE_OPEN_ERR				= 4;	// Can't open specified file 
const int COMMIT_PROMPT				= 5;	// Commit or rollback question 
const int COMMIT_MSG				= 6;	// Committing ... 
const int ROLLBACK_MSG				= 7;	// Rolling back 
const int CMD_ERR					= 8;	// Unknown frontend command 
const int ADD_PROMPT				= 9;	// Prompt for add function 
const int VERSION					= 10;		// Version string for -z 
const int NUMBER_PAGES				= 12;		// Number of DB pages allocated = %d \n
const int SWEEP_INTERV				= 13;		// Sweep interval = %d \n 
const int NUM_WAL_BUFF				= 14;		// Number of wal buffers = %d \n
const int WAL_BUFF_SIZE				= 15;		// Wal buffer size = %d \n
const int CKPT_LENGTH				= 16;		// Check point length = %d \n 
const int CKPT_INTERV				= 17;		// Check point interval = %d \n 
const int WAL_GRPC_WAIT				= 18;		// Wal group commit wait = %d \n
const int BASE_LEVEL				= 19;		// Base level = %d \n 
const int LIMBO						= 20;		// Transaction in limbo = %d \n 
// Help list	
const int HLP_FRONTEND				= 21;		// Frontend commands:\n 
const int HLP_BLOBVIEW				= 22;		// BLOBVIEW [<blobid as high:low>] -- edit a blob\n
const int HLP_BLOBDMP				= 23;		// BLOBDUMP <blobid as high:low> <file> -- dump blob to a file\n
const int HLP_EDIT					= 24;		// EDIT [<filename>] -- edit and read a SQL file\n\tWithout file name, edits current command buffer\n
const int HLP_INPUT					= 25;		// INput <filename> -- enter a named SQL file\n
const int HLP_OUTPUT				= 26;		// OUTput [<filename>] -- write output to named file\n
const int HLP_SHELL					= 27;		// SHELL <shell command> -- execute command shell\n
const int HLP_HELP					= 28;		// HELP -- Displays this menu\n 
const int HLP_SETCOM				= 29;		// "Set commands: " 
const int HLP_SET					= 30;		// \tSET -- Display current options \n 
const int HLP_SETAUTO				= 31;		// \tSET AUTOcommit  -- toggle autocommit of DDL statments\n 
const int HLP_SETBLOB				= 32;		// \tSET BLOBdisplay [ALL|N]-- Display blobs of type N\n\t SET BLOB turns off blob display\n
const int HLP_SETCOUNT				= 33;		// \tSET COUNT  -- toggle count of selected rows on/off \n 
const int HLP_SETECHO				= 34;		// \tSET ECHO  -- toggle command echo on/off \n 
const int HLP_SETSTAT				= 35;		// \tSET STATs -- toggles performance statistics display\n 
const int HLP_SETTERM				= 36;		// \tSET TERM <string> -- changes termination character\n 
const int HLP_SHOW					= 37;		// SHOW <object type> [<object name>] -- displays information on metadata\n 
const int HLP_OBJTYPE				= 38;		// "  <object type> = 'DB', 'TABLE', 'PROCedure', 'INDEX',\n   'GRANT', 'DOMAIN', 'VERSION'\n"
const int HLP_EXIT					= 39;		// EXIT -- Exit program and commit changes\n 
const int HLP_QUIT					= 40;		// QUIT -- Exit program and rollback changes\n\n 
const int HLP_ALL					= 41;		// All commands may be abbreviated to letters in CAPs\n 
const int HLP_SETSCHEMA				= 42;		// \tSET SCHema/DB <db name> -- changes current database\n
const int YES_ANS					= 43;		// Yes 
const int REPORT1					= 44;		// Current memory = !c\nDelta memory = !d\nMax memory = !x\nElapsed time= !e sec\n 
#if (defined WIN_NT)
const int REPORT2					= 93;		// Buffers = !b\nReads = !r\nWrites = !w\nFetches = !f\n 
#else
const int REPORT2					= 45;		// Cpu = !u sec\nBuffers = !b\nReads = !r\nWrites = !w\nFetches = !f\n 
#endif
const int BLOB_SUBTYPE				= 46;		// Blob display set to subtype %d. This blob: subtype = %d\n 
const int BLOB_PROMPT				= 47;		// Blob: %s, type 'edit' or filename to load>
const int DATE_PROMPT				= 48;		// Enter %s as M/D/Y>
const int NAME_PROMPT				= 49;		// Enter %s>
const int DATE_ERR					= 50;		// Bad date %s\n 
const int CON_PROMPT				= 51;		// "CON> "
const int HLP_SETLIST				= 52;		// \tSET LIST -- toggles column or table display\n 
const int NOT_FOUND				= 53;		// %s not found\n
const int COPY_ERR 					= 54;		// Errors occured(possibly duplicate domains) in creating %s in %s\n" 
const int SERVER_TOO_OLD			= 55;		// Server version too old to support the isql command 
const int REC_COUNT 				= 56;		// Total returned: %ld 
const int UNLICENSED				= 57;		// Unlicensed for database "%s" 
const int HLP_SETWIDTH				= 58;		// \tSET WIDTH <column name> [<width>] --sets/unsets print width for column name
const int HLP_SETPLAN				= 59;		// Toggle display of query access plan 
const int HLP_SETTIME				= 60;		// Toggle display of timestamp with DATE values 
const int HLP_EDIT2					= 61;		// edits current command buffer\n 
const int HLP_OUTPUT2				= 62;		// \tWithout file name, returns output to stdout\n 
const int HLP_SETNAMES				= 63;		// Set current character set 
const int HLP_OBJTYPE2				= 64;		// More objects 
const int HLP_SETBLOB2				= 65;		// \t SET BLOB turns off blob display\n 
const int HLP_SET_ROOT				= 66;		// (Use HELP SET for set options) 
const int NO_TABLES					= 67;		// There are no tables in this database 
const int NO_TABLE					= 68;		// There is no table %s in this database 
const int NO_VIEWS					= 69;		// There are no views in this database 
const int NO_VIEW					= 70;		// There is no view %s in this database 
const int NO_INDICES_ON_REL			= 71;		// There are no indices on table %s in this database 
const int NO_REL_OR_INDEX			= 72;		// There is no table or index %s in this database 
const int NO_INDICES				= 73;		// There are no indices in this database 
const int NO_DOMAIN					= 74;		// There is no domain %s in this database 
const int NO_DOMAINS				= 75;		// There are no domains in this database 
const int NO_EXCEPTION				= 76;		// There is no exception %s in this database 
const int NO_EXCEPTIONS				= 77;		// There are no exceptions in this database 
const int NO_FILTER					= 78;		// There is no filter %s in this database 
const int NO_FILTERS				= 79;		// There are no filters in this database 
const int NO_FUNCTION				= 80;		// There is no user-defined function %s in this database 
const int NO_FUNCTIONS				= 81;		// There are no user-defined functions in this database 
const int NO_GEN					= 82;		// There is no generator %s in this database 
const int NO_GENS					= 83;		// There are no generators in this database 
const int NO_GRANT_ON_REL			= 84;		// There is no privilege granted on table %s in this database 
const int NO_GRANT_ON_PROC			= 85;		// There is no privilege granted on stored procedure %s in this database 
const int NO_REL_OR_PROC			= 86;		// There is no table or stored procedure %s in this database 
const int NO_PROC					= 87;		// There is no stored procedure %s in this database 
const int NO_PROCS					= 88;		// There are no stored procedures in this database 
const int NO_TRIGGERS_ON_REL		= 89;		// There are no triggers on table %s in this database 
const int NO_REL_OR_TRIGGER			= 90;		// There is no table or trigger %s in this database 
const int NO_TRIGGERS				= 91;		// There are no triggers in this database 
const int NO_TRIGGER                = 121;      // There is no trigger %s in this database
const int NO_CHECKS_ON_REL			= 92;		// There are no check constraints on table %s in this database 
const int NO_COMMENTS				= 115;		// There are no comments for objects in this database.
const int BUFFER_OVERFLOW			= 94;		// An isql command exceeded maximum buffer size 
#ifdef SCROLLABLE_CURSORS
const int HLP_SETFETCH				= 95;		// \tSET AUTOfetch  -- toggle autofetch of records\n
#endif

const int NO_ROLES					= 95;		// There are no roles in this database
const int NO_REL_OR_PROC_OR_ROLE	= 96;		// There is no table, stored procedure, or
											// role %s in this database
const int NO_GRANT_ON_ROL			= 97;		// There is no membership privilege granted
											// on %s in this database
const int UNEXPECTED_EOF			= 98;		// Expected end of statement, encountered EOF
const int TIME_ERR					= 101;		// Bad TIME: %s\n 
const int HLP_OBJTYPE3				= 102;		// Some more objects 
const int NO_ROLE					= 103;		// There is no role %s in this database
const int INCOMPLETE_STR			= 105;		// Incomplete string in %s
const int HLP_SETSQLDIALECT			= 106;		// \tSET SQL DIALECT <n>    -- set sql dialect to <n>
const int NO_GRANT_ON_ANY			= 107;		// There is no privilege granted in this database. 
const int HLP_SETPLANONLY			= 108;		// toggle display of query plan without executing
const int HLP_SETHEADING			= 109;		// toggle display of query column titles
const int HLP_SETBAIL				= 110;		// toggle bailing out on errors in non-interactive mode
const int TIME_PROMPT				= 112;		// Enter %s as H:M:S>
const int TIMESTAMP_PROMPT			= 113;		// Enter %s as Y/MON/D H:MIN:S[.MSEC]>
const int TIMESTAMP_ERR				= 114;		// Bad TIMESTAMP: %s\n
const int ONLY_FIRST_BLOBS			= 116;		// Printing only the first %d blobs.
const int MSG_TABLES				= 117;		// Tables:
const int MSG_FUNCTIONS				= 118;		// Functions:
const int EXACTLINE					= 119;		// At line %d in file %s
const int AFTERLINE					= 120;		// After line %d in file %s
const int USAGE						= 1;		// usage: syntax
const int USAGE_NOARG				= 142;		// usage: missing argument for "%s"
const int USAGE_NOTINT				= 143;		// usage: argument "%s" for switch "%s" is not an integer
const int USAGE_RANGE				= 144;		// usage: value "%s" for switch "%s" is out of range
const int USAGE_DUPSW				= 145;		// usage: switch "%s" or its equivalent used more than once
const int USAGE_DUPDB				= 146;		// usage: more than one database name: "%s", "%s"
const int HLP_SETSQLSTATE        = 148;      // toggle display of SQLSTATE
#ifndef DEBUG
#define Trace		0
#endif

#ifdef DEBUG
#define Trace		1
#endif

#ifdef ISQL_MAIN
#define EXTERN
#else
#define EXTERN	extern
#endif

/* Utility transaction handle */

EXTERN isc_tr_handle M__trans;
EXTERN isc_tr_handle D__trans;
EXTERN isc_stmt_handle Stmt;
EXTERN FILE *Ofp, *Out, *Ifp, *Errfp;
EXTERN SCHAR Term[MAXTERM_SIZE];
EXTERN SCHAR Db_name[128];
EXTERN SCHAR Target_db[128];
EXTERN SCHAR User[128], Password[128], Role[128];
EXTERN SCHAR Numbufs[16];		/* # of cache buffers on connect */
EXTERN SSHORT Merge_stderr;
EXTERN USHORT SQL_dialect;
EXTERN USHORT db_SQL_dialect;
EXTERN USHORT major_ods;
EXTERN USHORT minor_ods;
#undef EXTERN

typedef vary VARY;

typedef struct sqltypes {
	SSHORT type;
	SCHAR type_name[WORDLENGTH];
} SQLTYPES;

#define MAXSUBTYPES 8			/* Top of subtypes array */

#define SMALLINT	7
#define INTEGER		8
#define QUAD		9
#define FLOAT		10
#define FCHAR		14
#define DOUBLE_PRECISION 27
#define DATE		35
#define VARCHAR		37
#define CSTRING		40
#define BLOB_ID		45
#define BLOB		261

#ifdef VMS
#include <descrip.h>
#define LIB$_INPSTRTRU	0x15821c
#endif

#ifdef UNIX
#define UNIX_LINE	1
#endif

#if (defined WIN_NT)
#include <io.h>
#define UNIX_LINE	1
#define PC_FILE_SEEK
#endif

#ifdef SMALL_FILE_NAMES
static const char* SCRATCH		= "fb_q";
#else
static const char* SCRATCH		= "fb_query_";
#endif

inline void STDERROUT(const char* st, bool cr = true)
{
	fprintf (Errfp, "%s", st);
	if (cr)
		fprintf (Errfp, "\n");
	fflush (Errfp);
}

#ifndef ISQL_ALLOC
#define ISQL_ALLOC(x)     gds__alloc (x)
#endif

#ifndef ISQL_FREE
#define ISQL_FREE(x)     {isc_free ((char*) x); x = NULL;}
#endif

static const char* NEWLINE			= "\n";
static const char* TAB_AS_SPACES	= "        ";

#endif /* _ISQL_ISQL_H_ */
