C  The contents of this file are subject to the Interbase Public
C  License Version 1.0 (the "License"); you may not use this file
C  except in compliance with the License. You may obtain a copy
C  of the License at http://www.Inprise.com/IPL.html
C
C  Software distributed under the License is distributed on an
C  "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express
C  or implied. See the License for the specific language governing
C  rights and limitations under the License.
C
C  The Original Code was created by Inprise Corporation
C  and its predecessors. Portions created by Inprise Corporation are
C  Copyright (C) Inprise Corporation.
C
C  All Rights Reserved.
C  Contributor(s): ______________________________________.
C
C         PROGRAM:        Preprocessor
C         MODULE:         gds.for
C         DESCRIPTION:    GDS constants, procedures, etc. for VMS
C 

C     PROCEDURE GDS__ATTACH_DATABASE (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*2       NAME_LENGTH,    { INPUT }
C        CHARACTER* (*)  FILE_NAME,      { INPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*2       DPB_LENGTH,     { INPUT }
C        CHARACTER* (*)  DPB             { INPUT }
C        )

      INTEGER*4 ISC_BADDRESS 
C     INTEGER*4 FUNCTION ISC_BADDRESS (
C        INTEGER*4       HANDLE          !( INPUT )
C        )

C     PROCEDURE GDS__CANCEL_BLOB (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       BLOB_HANDLE     { INPUT }
C        )

C     PROCEDURE GDS__CANCEL_EVENTS (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*4       ID              { INPUT }
C        )

C     PROCEDURE GDS__CLOSE_BLOB (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       BLOB_HANDLE     { INPUT }
C        )

C     PROCEDURE GDS__COMMIT_RETAINING (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       TRA_HANDLE      { INPUT / OUTPUT }
C        )

C     PROCEDURE GDS__COMMIT_TRANSACTION (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       TRA_HANDLE      { INPUT / OUTPUT }
C        )

C     PROCEDURE GDS__COMPILE_REQUEST (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*4       REQUEST_HANDLE, { INPUT / OUTPUT }
C        INTEGER*2       BLR_LENGTH,     { INPUT }
C        CHARACTER* (*)  BLR             { INPUT }
C        )

C     PROCEDURE GDS__COMPILE_REQUEST2 (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*4       REQUEST_HANDLE, { INPUT / OUTPUT }
C        INTEGER*2       BLR_LENGTH,     { INPUT }
C        CHARACTER* (*)  BLR             { INPUT }
C        )

C     PROCEDURE GDS__CREATE_BLOB (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*4       TRA_HANDLE,     { INPUT }
C        INTEGER*4       BLOB_HANDLE,    { INPUT / OUTPUT }
C        INTEGER*4       BLOB_ID(2)      { OUTPUT }
C        )

C     PROCEDURE GDS__CREATE_BLOB2 (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*4       TRA_HANDLE,     { INPUT }
C        INTEGER*4       BLOB_HANDLE,    { INPUT / OUTPUT }
C        INTEGER*4       BLOB_ID(2),     { OUTPUT }
C        CHARACTER* (*)  BPB             { INPUT }
C        )

C     PROCEDURE GDS__CREATE_DATABASE (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*2       NAME_LENGTH,    { INPUT }
C        CHARACTER* (*)  FILE_NAME,      { INPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*2       DPB_LENGTH,     { INPUT }
C        CHARACTER* (*)  DPB             { INPUT }
C        )

C     PROCEDURE GDS__DETACH_DATABASE (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE       { INPUT / OUTPUT }
C        )

      INTEGER*2 GDS__EVENT_BLOCK_A
C     PROCEDURE GDS__EVENT_BLOCK_A (
C        CHARACTER* (*)  EVENTS,         !{ INPUT/OUTPUT }
C        CHARACTER* (*)  BUFFER          !{ INPUT/OUTPUT }
C        INTEGER*2       COUNT,          !{ INPUT }
C        CHARACTER*31    EVENT_NAMES     !{ INPUT/OUTPUT }
C        )

C     PROCEDURE GDS__EVENT_WAIT (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*2       LENGTH,         { INPUT }
C        CHARACTER* (*)  EVENTS,         { INPUT/OUTPUT }
C        CHARACTER* (*)  BUFFER          { INPUT/OUTPUT }
C        )

      INTEGER*4 GDS__GET_SEGMENT 
C     INTEGER*4 FUNCTION GDS__GET_SEGMENT (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       BLOB_HANDLE,    { INPUT }
C        INTEGER*2       LENGTH,         { OUTPUT }
C        INTEGER*2       BUFFER_LENGTH,  { INPUT }
C        CHARACTER* (*)  BUFFER          { INPUT }
C        )

C     INTEGER*4 FUNCTION GDS__GET_SLICE (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*4       TRA_HANDLE,     { OUTPUT }
C        INTEGER*4       BLOB_HANDLE,    { INPUT }
C        INTEGER*2       SDL_LENGTH,     { INPUT }
C        CHARACTER* (*)  SDL,            { INPUT }
C        INTEGER*2       PARAM_LENGTH,   { INPUT }
C        CHARACTER* (*)  PARAM,          { INPUT }
C        INTEGER*4       SLICE_LENGTH,   { INPUT }
C        CHARACTER* (*)  SLICE,          { INPUT }
C        INTEGER*4       RETURN_LENGTH   { INPUT }
C        )

C     PROCEDURE GDS__OPEN_BLOB (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*4       TRA_HANDLE,     { INPUT }
C        INTEGER*4       BLOB_HANDLE,    { INPUT / OUTPUT }
C        INTEGER*4       BLOB_ID (2)     { INPUT }
C        )

C     PROCEDURE GDS__OPEN_BLOB2 (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*4       TRA_HANDLE,     { INPUT }
C        INTEGER*4       BLOB_HANDLE,    { INPUT / OUTPUT }
C        INTEGER*4       BLOB_ID (2),    { INPUT }
C        CHARACTER* (*)  BPB             { INPUT }
C        )

C      PROCEDURE GDS__COMMIT_TRANSACTION (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       TRA_HANDLE,     { INPUT / OUTPUT }
C        )

      INTEGER*4 GDS__PUT_SEGMENT 
C     INTEGER*4 FUNCTION GDS__PUT_SEGMENT (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       BLOB_HANDLE,    { INPUT }
C        INTEGER*2       LENGTH,         { INPUT }
C        CHARACTER* (*)  BUFFER,         { INPUT }
C        )

C     INTEGER*4 FUNCTION GDS__PUT_SLICE (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*4       TRA_HANDLE,     { OUTPUT }
C        INTEGER*4       BLOB_HANDLE,    { INPUT }
C        INTEGER*2       SDL_LENGTH,     { INPUT }
C        CHARACTER* (*)  SDL,            { INPUT }
C        INTEGER*2       PARAM_LENGTH,   { INPUT }
C        CHARACTER* (*)  PARAM,          { INPUT }
C        INTEGER*4       SLICE_LENGTH,   { INPUT }
C        CHARACTER* (*)  SLICE           { INPUT }
C        )

C     PROCEDURE GDS__QUE_EVENTS (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*4       ID,             { INPUT }
C        INTEGER*2       LENGTH,         { INPUT }
C        CHARACTER* (*)  EVENTS,         { INPUT/OUTPUT }
C        INTEGER*4       AST,            { INPUT }
C        INTEGER*4       ARG             { INPUT }
C        )

C     PROCEDURE GDS__RECEIVE (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       REQUEST_HANDLE, { INPUT }
C        INTEGER*2       MESSAGE_TYPE,   { INPUT }
C        INTEGER*2       MESSAGE_LENGTH, { INPUT }
C        CHARACTER* (*)  MESSAGE,        { INPUT }
C        INTEGER*2       INSTANTIATION   { INPUT }
C        )

C     PROCEDURE GDS__RELEASE_REQUEST (
C        INTEGER*4       STAT(20),        { OUTPUT }
C        INTEGER*4       REQUEST_HANDLE   { INPUT / OUTPUT }
C        )

C     PROCEDURE GDS__ROLLBACK_TRANSACTION (
C        INTEGER*4       STAT(20),         { OUTPUT }
C        INTEGER*4       TRA_HANDLE        { INPUT / OUTPUT }
C        )

C     PROCEDURE GDS__SEND (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       REQUEST_HANDLE, { INPUT }
C        INTEGER*2       MESSAGE_TYPE,   { INPUT }
C        INTEGER*2       MESSAGE_LENGTH, { INPUT }
C        CHARACTER* (*)  MESSAGE,        { INPUT }
C        INTEGER*2       INSTANTIATION   { INPUT }
C        )

C     PROCEDURE GDS__SET_DEBUG (
C        INTEGER*4       DEBUG_VAL       { INPUT }
C        )

C     PROCEDURE GDS__START_AND_SEND (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       REQUEST_HANDLE, { INPUT / OUTPUT }
C        INTEGER*4       TRA_HANDLE,     { INPUT }
C        INTEGER*2       MESSAGE_TYPE,   { INPUT }
C        INTEGER*2       MESSAGE_LENGTH, { INPUT }
C        CHARACTER* (*)  MESSAGE,        { INPUT }
C        INTEGER*2       INSTANTIATION   { INPUT }
C        )

C     PROCEDURE GDS__START_REQUEST (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       REQUEST_HANDLE, { INPUT  }
C        INTEGER*4       TRA_HANDLE,     { INPUT }
C        INTEGER*2       INSTANTIATION   { INPUT }
C        )
   
C     PROCEDURE GDS__START_TRANSACTION (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       TRA_HANDLE,     { INPUT / OUTPUT }
C        INTEGER*2       TRA_COUNT,      { INPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*2       TPB_LENGTH      { INPUT }
C        CHARACTER* (*)  TPB             { INPUT }
C        )

C     PROCEDURE GDS__UNWIND_REQUEST (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       REQUEST_HANDLE, { INPUT }
C        INTEGER*2       INSTANTIATION   { INPUT }
C        )

C     PROCEDURE gds__ftof (
C        CHARACTER* (*)  STRING1,        { INPUT }
C        INTEGER*2       LENGTH1,        { INPUT }
C        CHARACTER* (*)  STRING2,        { INPUT }
C        INTEGER*2       LENGTH2         { INPUT }
C        )

C     PROCEDURE gds__print_status (
C        INTEGER*4       STAT(20)        { INPUT }
C        )
 
C     INTEGER FUNCTION gds__sqlcode (
C        INTEGER*4       STAT(20)        { INPUT }
C        )
 
C     procedure gds__close (
C	out	stat		: gds__status_vector; 
C	in	cursor_name	: UNIV gds__string
C	); extern;

C     procedure gds__declare (
C	out	stat		: gds__status_vector; 
C	in	statement_name	: UNIV gds__string;
C	in	cursor_name	: UNIV gds__string
C	); extern;

C     procedure gds__describe (
C	out	stat		: gds__status_vector; 
C	in	statement_name	: UNIV gds__string;
C	in out	descriptor	: UNIV sqlda
C	); extern;

C     procedure gds__execute (
C	out	stat		: gds__status_vector; 
C	in	trans_handle	: gds__handle;
C	in	statement_name	: UNIV gds__string;
C	in out	descriptor	: UNIV sqlda
C	); extern;

C     procedure gds__execute_immediate (
C	out	stat		: gds__status_vector; 
C	in	db_handle	: gds__handle;
C	in	trans_handle	: gds__handle;
C	in	string_length	: integer;
C	in	string		: UNIV gds__string
C	); extern;

      INTEGER*4 GDS__FETCH
C     function gds__fetch (
C	out	stat		: gds__status_vector; 
C	in	cursor_name	: UNIV gds__string;
C	in out	descriptor	: UNIV sqlda
C	) : integer32; extern;

C     procedure gds__open (
C	out	stat		: gds__status_vector; 
C	in	trans_handle	: gds__handle;
C	in	cursor_name	: UNIV gds__string;
C	in out	descriptor	: UNIV sqlda
C	); extern;

C     procedure gds__prepare (
C	out	stat		: gds__status_vector; 
C	in	db_handle	: gds__handle;
C	in	trans_handle	: gds__handle;
C	in	statement_name	: UNIV gds__string;
C	in	string_length	: integer;
C	in	string		: UNIV gds__string;
C	in out	descriptor	: UNIV sqlda
C	); extern;

C     procedure pyxis__compile_map (
C	out	stat		: gds__status_vector; 
C	in 	form_handle	: gds__handle;
C	out 	map_handle	: gds__handle;
C	in	length		: integer16;
C	in	map		: UNIV gds__string
C	); extern;

C     procedure pyxis__compile_sub_map (
C	out	stat		: gds__status_vector; 
C	in 	form_handle	: gds__handle;
C	out 	map_handle	: gds__handle;
C	in	length		: integer16;
C	in	map		: UNIV gds__string
C	); extern;

C     procedure pyxis__create_window (
C	in out	window_handle	: gds__handle;
C	in	name_length	: integer16;
C	in	file_name	: UNIV gds__string; 
C	in out	width		: integer16;
C	in out	height		: integer16
C	); extern;

C     procedure pyxis__delete_window (
C	in out	window_handle	: gds__handle
C	); extern;

C     procedure pyxis__drive_form (
C	out	stat		: gds__status_vector; 
C	in	db_handle	: gds__handle;
C	in	trans_handle	: gds__handle;
C	in out	window_handle	: gds__handle;
C	in 	map_handle	: gds__handle;
C	in	input		: UNIV gds__string;
C	in	output		: UNIV gds__string
C	); extern;

C     procedure pyxis__fetch (
C	out	stat		: gds__status_vector; 
C	in	db_handle	: gds__handle;
C	in	trans_handle	: gds__handle;
C	in 	map_handle	: gds__handle;
C	in	output		: UNIV gds__string
C	); extern;

C     procedure pyxis__insert (
C	out	stat		: gds__status_vector; 
C	in	db_handle	: gds__handle;
C	in	trans_handle	: gds__handle;
C	in 	map_handle	: gds__handle;
C	in	input		: UNIV gds__string
C	); extern;

C     procedure pyxis__load_form (
C	out	stat		: gds__status_vector; 
C	in	db_handle	: gds__handle;
C	in 	trans_handle	: gds__handle;
C	in out	form_handle	: gds__handle;
C	in	name_length	: integer16;
C	in	form_name	: UNIV gds__string 
C	); extern;

      INTEGER*2 PYXIS__MENU
C     function pyxis__menu (
C	in out	window_handle	: gds__handle;
C	in out	menu_handle	: gds__handle;
C	in	length		: integer16;
C	in	menu		: UNIV gds__string
C	) : integer16; extern;

C     procedure pyxis__pop_window (
C	in 	window_handle	: gds__handle
C	); extern;

C     procedure pyxis__reset_form (
C	out	stat		: gds__status_vector; 
C	in	window_handle	: gds__handle
C	); extern;

C     procedure pyxis__drive_menu (
C	in out	window_handle	: gds__handle;
C	in out	menu_handle	: gds__handle;
C	in	blr_length	: integer16;
C	in	blr_source	: UNIV gds__string;
C	in	title_length	: integer16;
C	in	title		: UNIV gds__string;
C	out	terminator	: integer16;
C	out	entree_length	: integer16;
C	out	entree_text	: UNIV gds__string;
C	out	entree_value	: integer32
C	); extern;

C     procedure pyxis__get_entree (
C	in	menu_handle	: gds__handle;
C	out	entree_length	: integer16;
C	out	entree_text	: UNIV gds__string;
C	out	entree_value	: integer32;
C	out	entree_end	: integer16
C	); extern;

C     procedure pyxis__initialize_menu (
C	in out	menu_handle	: gds__handle
C	); extern;

C     procedure pyxis__put_entree (
C	in	menu_handle	: gds__handle;
C	in	entree_length	: integer16;
C	in	entree_text	: UNIV gds__string;
C	in	entree_value	: integer32
C	); extern;

C     PROCEDURE isc_exec_procedure (
C        INTEGER*4       STAT(20),       { OUTPUT }
C        INTEGER*4       DB_HANDLE,      { INPUT }
C        INTEGER*4       TRA_HANDLE,     { INPUT }
C        INTEGER*2       NAME_LENGTH,    { INPUT }
C        CHARACTER* (*)  PROC_NAME,      { INPUT }
C        INTEGER*2       BLR_LENGTH,     { INPUT }
C        CHARACTER* (*)  BLR,             { INPUT }
C        INTEGER*2       IN_MESSAGE_LENGTH, { INPUT }
C        CHARACTER* (*)  IN_MESSAGE,        { INPUT }
C        INTEGER*2       OUT_MESSAGE_LENGTH, { INPUT }
C        CHARACTER* (*)  OUT_MESSAGE        { INPUT / OUTPUT }
C        )

       INTEGER*2 GDS__TRUE, GDS__FALSE
       PARAMETER (
     +    GDS__TRUE = 1,
     +    GDS__FALSE = 0)

       INTEGER*2  BLR_BLOB
       PARAMETER  (BLR_BLOB = 261)

       CHARACTER*1 
     +    BLR_TEXT, BLR_SHORT, BLR_LONG, BLR_QUAD, BLR_FLOAT,
     +    BLR_DOUBLE, BLR_D_FLOAT, BLR_DATE, BLR_VARYING, BLR_CSTRING,
     +    BLR_VERSION4, BLR_EOC, BLR_END

       PARAMETER (
     +    BLR_TEXT = CHAR(14),
     +    BLR_SHORT = CHAR(7),
     +    BLR_LONG = CHAR(8),
     +    BLR_QUAD = CHAR(9),
     +    BLR_FLOAT = CHAR(10),
     +    BLR_DOUBLE = CHAR(27),
     +    BLR_D_FLOAT = CHAR(11),
     +    BLR_DATE = CHAR(35),
     +    BLR_VARYING = CHAR(37),
     +    BLR_CSTRING = CHAR(40),
     +    BLR_VERSION4 = CHAR(4),
     +    BLR_EOC = CHAR(76),
     +    BLR_END = CHAR(255))

      CHARACTER*1
     +    BLR_ASSIGNMENT, BLR_BEGIN, BLR_MESSAGE, BLR_ERASE, BLR_FETCH,
     +    BLR_FOR, BLR_IF, BLR_LOOP, BLR_MODIFY, BLR_HANDLER, 
     +    BLR_RECEIVE, BLR_SELECT, BLR_SEND, BLR_STORE, BLR_LABEL,
     +    BLR_LEAVE


      PARAMETER (
     +    BLR_ASSIGNMENT = CHAR(1),
     +    BLR_BEGIN = CHAR(2),
     +    BLR_MESSAGE = CHAR(4),
     +    BLR_ERASE = CHAR(5),
     +    BLR_FETCH = CHAR(6),
     +    BLR_FOR = CHAR(7),
     +    BLR_IF = CHAR(8),
     +    BLR_LOOP = CHAR(9),
     +    BLR_MODIFY = CHAR(10),
     +    BLR_HANDLER = CHAR(11),
     +    BLR_RECEIVE = CHAR(12),
     +    BLR_SELECT = CHAR(13),
     +    BLR_SEND = CHAR(14),
     +    BLR_STORE = CHAR(15),
     +    BLR_LABEL = CHAR(17),
     +    BLR_LEAVE = CHAR(18))

      CHARACTER*1
     +    BLR_LITERAL, BLR_DBKEY, BLR_FIELD, BLR_FID, BLR_PARAMETER,
     +    BLR_VARIABLE, BLR_AVERAGE, BLR_COUNT, BLR_MAXIMUM, 
     +    BLR_MINIMUM, BLR_TOTAL

      PARAMETER (
     +    BLR_LITERAL = CHAR(21),
     +    BLR_DBKEY = CHAR(22),
     +    BLR_FIELD = CHAR(23),
     +    BLR_FID = CHAR(24),
     +    BLR_PARAMETER = CHAR(25),
     +    BLR_VARIABLE = CHAR(26),
     +    BLR_AVERAGE = CHAR(27),
     +    BLR_COUNT = CHAR(28),
     +    BLR_MAXIMUM = CHAR(29),
     +    BLR_MINIMUM = CHAR(30),
     +    BLR_TOTAL = CHAR(31))

      CHARACTER*1
     +    BLR_ADD, BLR_SUBTRACT, BLR_MULTIPLY, BLR_DIVIDE,
     +    BLR_NEGATE, BLR_CONCATENATE, BLR_SUBSTRING, 
     +    BLR_PARAMETER2, BLR_FROM, BLR_VIA, BLR_NULL
     

      PARAMETER (
     +    BLR_ADD = CHAR(34),
     +    BLR_SUBTRACT = CHAR(35),
     +    BLR_MULTIPLY = CHAR(36),
     +    BLR_DIVIDE = CHAR(37),
     +    BLR_NEGATE = CHAR(38),
     +    BLR_CONCATENATE = CHAR(39),
     +    BLR_SUBSTRING = CHAR(40),
     +    BLR_PARAMETER2 = CHAR(41),
     +    BLR_FROM = CHAR(42),
     +    BLR_VIA = CHAR(43),
     +    BLR_NULL = CHAR(45))

      CHARACTER*1
     +    BLR_EQL, BLR_NEQ, BLR_GTR, BLR_GEQ, BLR_LSS, BLR_LEQ, 
     +    BLR_CONTAINING, BLR_MATCHING, BLR_STARTING, BLR_BETWEEN, 
     +    BLR_OR, BLR_AND, BLR_NOT, BLR_ANY, BLR_MISSING, BLR_LIKE,
     +    BLR_UNIQUE

       PARAMETER (
     +    BLR_EQL = CHAR(47),
     +    BLR_NEQ = CHAR(48),
     +    BLR_GTR = CHAR(49),
     +    BLR_GEQ = CHAR(50),
     +    BLR_LSS = CHAR(51),
     +    BLR_LEQ = CHAR(52),
     +    BLR_CONTAINING = CHAR(53),
     +    BLR_MATCHING = CHAR(54),
     +    BLR_STARTING = CHAR(55),
     +    BLR_BETWEEN = CHAR(56),
     +    BLR_OR = CHAR(57),
     +    BLR_AND = CHAR(58),
     +    BLR_NOT = CHAR(59),
     +    BLR_ANY = CHAR(60),
     +    BLR_MISSING = CHAR(61),
     +    BLR_UNIQUE = CHAR(62),
     +    BLR_LIKE = CHAR(63))

      CHARACTER*1
     +    BLR_RSE, BLR_FIRST, BLR_PROJECT, BLR_SORT, BLR_BOOLEAN, 
     +    BLR_ASCENDING, BLR_DESCENDING, BLR_RELATION, BLR_RID,
     +    BLR_UNION, BLR_MAP, BLR_GROUP_BY, BLR_AGGREGATE,
     +    BLR_JOIN_TYPE


      PARAMETER (
     +    BLR_RSE = CHAR(67),
     +    BLR_FIRST = CHAR(68),
     +    BLR_PROJECT = CHAR(69),
     +    BLR_SORT = CHAR(70),
     +    BLR_BOOLEAN = CHAR(71),
     +    BLR_ASCENDING = CHAR(72),
     +    BLR_DESCENDING = CHAR(73),
     +    BLR_RELATION = CHAR(74),
     +    BLR_RID = CHAR(75),
     +    BLR_UNION = CHAR(76),
     +    BLR_MAP = CHAR(77),
     +    BLR_GROUP_BY = CHAR(78),     
     +    BLR_AGGREGATE = CHAR(79),
     +    BLR_JOIN_TYPE = CHAR(80))

      CHARACTER*1
     +    BLR_INNER, BLR_LEFT, BLR_RIGHT, BLR_FULL

      PARAMETER (
     +    BLR_INNER = CHAR(0),
     +    BLR_LEFT = CHAR(1),
     +    BLR_RIGHT = CHAR(2),
     +    BLR_FULL = CHAR(3))

      CHARACTER*1
     +    BLR_AGG_COUNT, BLR_AGG_MAX, BLR_AGG_MIN, BLR_AGG_TOTAL,
     +    BLR_AGG_AVERAGE, BLR_RUN_COUNT, BLR_RUN_MAX, BLR_RUN_MIN,
     +    BLR_RUN_TOTAL, BLR_RUN_AVERAGE

      PARAMETER (
     +    BLR_AGG_COUNT = CHAR(83),     
     +    BLR_AGG_MAX = CHAR(84),
     +    BLR_AGG_MIN = CHAR(85),
     +    BLR_AGG_TOTAL = CHAR(86),
     +    BLR_AGG_AVERAGE = CHAR(87),
     +    BLR_RUN_COUNT = CHAR(118),
     +    BLR_RUN_MAX = CHAR(89),
     +    BLR_RUN_MIN = CHAR(90),
     +    BLR_RUN_TOTAL = CHAR(91),
     +    BLR_RUN_AVERAGE = CHAR(92)) 

      CHARACTER*1
     +    BLR_FUNCTION, BLR_GEN_ID, BLR_PROT_MASK, BLR_UPCASE,
     +    BLR_LOCK_STATE, BLR_VALUE_IF, BLR_MATCHING2, BLR_INDEX

      PARAMETER (
     +    BLR_FUNCTION = CHAR(100),
     +    BLR_GEN_ID = CHAR(101),
     +    BLR_PROT_MASK = CHAR(102),
     +    BLR_UPCASE = CHAR(103),
     +    BLR_LOCK_STATE = CHAR(104),
     +    BLR_VALUE_IF = CHAR(105),
     +    BLR_MATCHING2 = CHAR(106),
     +    BLR_INDEX = CHAR(107))

      CHARACTER*1
     +    BLR_ANSI_LIKE, BLR_BOOKMARK, BLR_CRACK,
     +    BLR_FORCE_CRACK, BLR_SEEK, BLR_FIND,
     +    BLR_LOCK_RELATION, BLR_LOCK_RECORD,
     +    BLR_SET_BOOKMARK, BLR_GET_BOOKMARK

      PARAMETER (
     +    BLR_ANSI_LIKE = CHAR(108),
     +    BLR_BOOKMARK = CHAR(109),
     +    BLR_CRACK = CHAR(110),
     +    BLR_FORCE_CRACK = CHAR(111),
     +    BLR_SEEK = CHAR(112),
     +    BLR_FIND = CHAR(113),
     +    BLR_LOCK_RELATION = CHAR(114),
     +    BLR_LOCK_RECORD = CHAR(115),
     +    BLR_SET_BOOKMARK = CHAR(116),
     +    BLR_GET_BOOKMARK = CHAR(117))

      CHARACTER*1
     +    BLR_RS_STREAM, BLR_EXEC_PROC, BLR_BEGIN_RANGE,
     +    BLR_END_RANGE, BLR_DELETE_RANGE, BLR_PROCEDURE,
     +    BLR_PID, BLR_EXEC_PID, BLR_SINGULAR, BLR_ABORT,
     +    BLR_BLOCK, BLR_ERROR_HANDLER, BLR_PARAMETER3,
     +    BLR_CAST

      PARAMETER (
     +    BLR_PARAMETER3 = CHAR(88),
     +    BLR_RS_STREAM = CHAR(119),
     +    BLR_EXEC_PROC = CHAR(120),
     +    BLR_BEGIN_RANGE = CHAR(121),
     +    BLR_END_RANGE = CHAR(122),
     +    BLR_DELETE_RANGE = CHAR(123),
     +    BLR_PROCEDURE = CHAR(124),
     +    BLR_PID = CHAR(125),
     +    BLR_EXEC_PID = CHAR(126),
     +    BLR_SINGULAR = CHAR(127),
     +    BLR_ABORT = CHAR(128),
     +    BLR_BLOCK = CHAR(129),
     +    BLR_ERROR_HANDLER = CHAR(130),
     +    BLR_CAST = CHAR(131))

      CHARACTER*1
     +    BLR_GDS_CODE, BLR_SQL_CODE, BLR_EXCEPTION,
     +    BLR_TRIGGER_CODE, BLR_DEFAULT_CODE

      PARAMETER (
     +    BLR_GDS_CODE = CHAR(0),
     +    BLR_SQL_CODE = CHAR(1),
     +    BLR_EXCEPTION = CHAR(2),
     +    BLR_TRIGGER_CODE = CHAR(3),
     +    BLR_DEFAULT_CODE = CHAR(4))

C     database parameter block stuff 

      CHARACTER*1
     +     GDS__DPB_CDD_PATHNAME, GDS__DPB_ALLOCATION,
     +     GDS__DPB_JOURNAL, GDS__DPB_PAGE_SIZE,
     +     GDS__DPB_BUFFERS,  GDS__DPB_BUFFER_LENGTH,
     +     GDS__DPB_DEBUG, GDS__DPB_GARBAGE_COLLECT,
     +     GDS__DPB_VERIFY,  GDS__DPB_SWEEP,
     +     GDS__DPB_ENABLE_JOURNAL, GDS__DPB_DISABLE_JOURNAL,
     +     GDS__DPB_DBKEY_SCOPE, GDS__DPB_NUMBER_OF_USERS,
     +     GDS__DPB_TRACE, GDS__DPB_NO_GARBAGE_COLLECT,
     +     GDS__DPB_DAMAGED, GDS__DPB_LICENSE, GDS__DPB_SYS_USER_NAME,
     +     GDS__DPB_ENCRYPT_KEY, GDS__DPB_ACTIVATE_SHADOW,
     +     GDS__DPB_SWEEP_INTERVAL, GDS__DPB_DELETE_SHADOW

      PARAMETER (
     +     GDS__DPB_CDD_PATHNAME       = CHAR (1), !{NOT IMPLEMENTED}
     +     GDS__DPB_ALLOCATION         = CHAR (2), !{NOT IMPLEMENTED}
     +     GDS__DPB_JOURNAL            = CHAR (3), !{NOT IMPLEMENTED}
     +     GDS__DPB_PAGE_SIZE          = CHAR (4),
     +     GDS__DPB_BUFFERS            = CHAR (5),
     +     GDS__DPB_BUFFER_LENGTH      = CHAR (6), !{NOT IMPLEMENTED}
     +     GDS__DPB_DEBUG              = CHAR (7), !{NOT IMPLEMENTED}
     +     GDS__DPB_GARBAGE_COLLECT    = CHAR (8), !{NOT IMPLEMENTED}
     +     GDS__DPB_VERIFY             = CHAR (9),
     +     GDS__DPB_SWEEP              = CHAR (10),
     +     GDS__DPB_ENABLE_JOURNAL     = CHAR (11),
     +     GDS__DPB_DISABLE_JOURNAL    = CHAR (12),
     +     GDS__DPB_DBKEY_SCOPE        = CHAR (13), !{NOT IMPLEMENTED}
     +     GDS__DPB_NUMBER_OF_USERS    = CHAR (14), !{NOT IMPLEMENTED}
     +     GDS__DPB_TRACE              = CHAR (15), !{NOT IMPLEMENTED}
     +     GDS__DPB_NO_GARBAGE_COLLECT = CHAR (16), !{NOT IMPLEMENTED}
     +     GDS__DPB_DAMAGED            = CHAR (17), !{NOT IMPLEMENTED}
     +     GDS__DPB_LICENSE            = CHAR (18),
     +     GDS__DPB_SYS_USER_NAME      = CHAR (19))

      PARAMETER (
     +     GDS__DPB_ENCRYPT_KEY           = CHAR (20),
     +     GDS__DPB_ACTIVATE_SHADOW       = CHAR (21),
     +     GDS__DPB_SWEEP_INTERVAL        = CHAR (22),
     +     GDS__DPB_DELETE_SHADOW         = CHAR (23))

      CHARACTER*1
     +     GDS__DPB_PAGES, GDS__DPB_RECORDS, 
     +     GDS__DPB_INDICES, GDS__DPB_TRANSACTIONS, 
     +     GDS__DPB_NO_UPDATE, GDS__DPB_REPAIR

      PARAMETER (
     +     GDS__DPB_PAGES = CHAR(1),
     +     GDS__DPB_RECORDS = CHAR(2),
     +     GDS__DPB_INDICES = CHAR(4),
     +     GDS__DPB_TRANSACTIONS = CHAR(8),
     +     GDS__DPB_NO_UPDATE = CHAR(16),
     +     GDS__DPB_REPAIR = CHAR(32))



C     Transaction parameter block stuff 

      CHARACTER*1
     + GDS__TPB_VERSION3, GDS__TPB_CONSISTENCY, GDS__TPB_CONCURRENCY,
     + GDS__TPB_SHARED, GDS__TPB_PROTECTED, GDS__TPB_EXCLUSIVE,
     + GDS__TPB_WAIT, GDS__TPB_NOWAIT, GDS__TPB_READ, GDS__TPB_WRITE,
     + GDS__TPB_LOCK_READ, GDS__TPB_LOCK_WRITE, GDS__TPB_VERB_TIME,
     + GDS__TPB_COMMIT_TIME                            


      PARAMETER (
     + GDS__TPB_VERSION3     = CHAR (3),
     + GDS__TPB_CONSISTENCY  = CHAR (1),
     + GDS__TPB_CONCURRENCY  = CHAR (2),
     + GDS__TPB_SHARED       = CHAR (3),
     + GDS__TPB_PROTECTED    = CHAR (4),
     + GDS__TPB_EXCLUSIVE    = CHAR (5),
     + GDS__TPB_WAIT         = CHAR (6),
     + GDS__TPB_NOWAIT       = CHAR (7),
     + GDS__TPB_READ         = CHAR (8),
     + GDS__TPB_WRITE        = CHAR (9),
     + GDS__TPB_LOCK_READ    = CHAR (10),
     + GDS__TPB_LOCK_WRITE   = CHAR (11),
     + GDS__TPB_VERB_TIME    = CHAR (12),
     + GDS__TPB_COMMIT_TIME  = CHAR (13))

C Blob Parameter Block

      CHARACTER*1
     +  gds__bpb_version1, gds__bpb_source_type, gds__bpb_target_type,
     +  gds__bpb_type, gds__bpb_type_segmented, gds__bpb_type_stream

      PARAMETER (
     +  gds__bpb_version1	= CHAR (1),
     +  gds__bpb_source_type	= CHAR (1),
     +  gds__bpb_target_type	= CHAR (2),
     +  gds__bpb_type		= CHAR (3),

     +  gds__bpb_type_segmented	= CHAR (0),
     +  gds__bpb_type_stream	= CHAR (1))

C     Information parameters

C Common, structural codes 

      CHARACTER*1  GDS__INFO_END, GDS__INFO_TRUNCATED,
     +    GDS__INFO_ERROR

      PARAMETER (
     +    GDS__INFO_END                  = CHAR(1),
     +    GDS__INFO_TRUNCATED            = CHAR(2),
     +    GDS__INFO_ERROR                = CHAR(3))

C DATABASE INFORMATION ITEMS 

      CHARACTER*1 GDS__INFO_ID, GDS__INFO_READS, GDS__INFO_WRITES,
     +    GDS__INFO_FETCHES, GDS__INFO_MARKS, GDS__INFO_IMPLEMENTATION,
     +    GDS__INFO_VERSION, GDS__INFO_BASE_LEVEL, GDS__INFO_PAGE_SIZE,
     +    GDS__INFO_NUM_BUFFERS, GDS__INFO_LIMBO,
     +    GDS__INFO_CURRENT_MEMORY, GDS__INFO_MAX_MEMORY,
     +    GDS__INFO_WINDOW_TURNS, GDS__INFO_DB_ID

      PARAMETER (
     +    GDS__INFO_DB_ID                = CHAR(4),
     +    GDS__INFO_READS                = CHAR(5),
     +    GDS__INFO_WRITES               = CHAR(6),
     +    GDS__INFO_FETCHES              = CHAR(7),
     +    GDS__INFO_MARKS                = CHAR(8),
     +    GDS__INFO_IMPLEMENTATION       = CHAR(11),
     +    GDS__INFO_VERSION              = CHAR(12),
     +    GDS__INFO_BASE_LEVEL           = CHAR(13),
     +    GDS__INFO_PAGE_SIZE            = CHAR(14),
     +    GDS__INFO_NUM_BUFFERS          = CHAR(15),
     +    GDS__INFO_LIMBO                = CHAR(16),
     +    GDS__INFO_CURRENT_MEMORY       = CHAR(17),
     +    GDS__INFO_MAX_MEMORY           = CHAR(18),
     +    GDS__INFO_WINDOW_TURNS         = CHAR(19))

      CHARACTER*1 GDS__INFO_LICENSE, GDS__INFO_ALLOCATION,
     +    GDS__INFO_ATTACHMENT_ID, GDS__INFO_READ_SEQ_COUNT,
     +    GDS__INFO_READ_IDX_COUNT, GDS__INFO_INSERT_COUNT,
     +    GDS__INFO_UPDATE_COUNT,
     +    GDS__INFO_DELETE_COUNT, GDS__INFO_BACKOUT_COUNT,
     +    GDS__INFO_PURGE_COUNT, GDS__INFO_EXPUNGE_COUNT,
     +    GDS__INFO_SWEEP_INTERVAL, GDS__INFO_ODS_VERSION,
     +    GDS__INFO_ODS_MINOR_VERSION

      PARAMETER (
     +    GDS__INFO_LICENSE              = CHAR(20),
     +    GDS__INFO_ALLOCATION           = CHAR(21),
     +    GDS__INFO_ATTACHMENT_ID        = CHAR(22),
     +    GDS__INFO_READ_SEQ_COUNT       = CHAR(23),
     +    GDS__INFO_READ_IDX_COUNT       = CHAR(24), 
     +    GDS__INFO_INSERT_COUNT          = CHAR(25),
     +    GDS__INFO_UPDATE_COUNT         = CHAR(26),
     +    GDS__INFO_DELETE_COUNT         = CHAR(27), 
     +    GDS__INFO_BACKOUT_COUNT        = CHAR(28),
     +    GDS__INFO_PURGE_COUNT          = CHAR(29), 
     +    GDS__INFO_EXPUNGE_COUNT        = CHAR(30),
     +    GDS__INFO_SWEEP_INTERVAL       = CHAR(31), 
     +    GDS__INFO_ODS_VERSION          = CHAR(32),
     +    GDS__INFO_ODS_MINOR_VERSION    = CHAR(33))

C REQUEST INFORMATION ITEMS 

      CHARACTER*1 GDS__INFO_NUMBER_MESSAGES, GDS__INFO_MAX_MESSAGE,
     +    GDS__INFO_MAX_SEND, GDS__INFO_MAX_RECEIVE, GDS__INFO_STATE,
     +    GDS__INFO_MESSAGE_NUMBER, GDS__INFO_MESSAGE_SIZE,
     +    GDS__INFO_REQUEST_COST, GDS__INFO_REQ_ACTIVE,
     +    GDS__INFO_REQ_INACTIVE, GDS__INFO_REQ_SEND,
     +    GDS__INFO_REQ_RECEIVE, GDS__INFO_REQ_SELECT

      PARAMETER (
     +    GDS__INFO_NUMBER_MESSAGES     = CHAR(4),
     +    GDS__INFO_MAX_MESSAGE         = CHAR(5),
     +    GDS__INFO_MAX_SEND            = CHAR(6),
     +    GDS__INFO_MAX_RECEIVE         = CHAR(7),
     +    GDS__INFO_STATE               = CHAR(8),
     +    GDS__INFO_MESSAGE_NUMBER      = CHAR(9),
     +    GDS__INFO_MESSAGE_SIZE        = CHAR(10),
     +    GDS__INFO_REQUEST_COST        = CHAR(11),
     +    GDS__INFO_REQ_ACTIVE          = CHAR(2),
     +    GDS__INFO_REQ_INACTIVE        = CHAR(3),
     +    GDS__INFO_REQ_SEND            = CHAR(4),
     +    GDS__INFO_REQ_RECEIVE         = CHAR(5),
     +    GDS__INFO_REQ_SELECT          = CHAR(6))

C BLOB INFORMATION ITEMS 

      CHARACTER*1 GDS__INFO_BLOB_NUM_SEGMENTS,
     +    GDS__INFO_BLOB_MAX_SEGMENT,
     +    GDS__INFO_BLOB_TOTAL_LENGTH,
     +    GDS__INFO_BLOB_TYPE

      PARAMETER (
     +    GDS__INFO_BLOB_NUM_SEGMENTS   = CHAR(4),
     +    GDS__INFO_BLOB_MAX_SEGMENT    = CHAR(5),
     +    GDS__INFO_BLOB_TOTAL_LENGTH   = CHAR(6),
     +    GDS__INFO_BLOB_TYPE           = CHAR(7))

C TRANSACTION INFORMATION ITEMS 

      CHARACTER*1 GDS__INFO_TRA_ID
      PARAMETER (
     +    GDS__INFO_TRA_ID               = CHAR(4))

C Dynamic SQL datatypes

      INTEGER*2
     + SQL_TEXT,
     + SQL_VARYING,
     + SQL_SHORT,
     + SQL_LONG,
     + SQL_FLOAT,
     + SQL_DOUBLE,
     + SQL_D_FLOAT,
     + SQL_DATE,
     + SQL_BLOB

      PARAMETER (
     + SQL_TEXT = 452,
     + SQL_VARYING = 448,
     + SQL_SHORT = 500,
     + SQL_LONG = 496,
     + SQL_FLOAT = 482,
     + SQL_DOUBLE = 480,
     + SQL_D_FLOAT = 530,
     + SQL_DATE = 510,
     + SQL_BLOB = 520)

C  Forms Package definitions 

C  Map definition block definitions 

      INTEGER*2
     + PYXIS__MAP_VERSION1,
     + PYXIS__MAP_FIELD2,
     + PYXIS__MAP_FIELD1,
     + PYXIS__MAP_MESSAGE,
     + PYXIS__MAP_TERMINATOR,
     + PYXIS__MAP_TERMINATING_FIELD,
     + PYXIS__MAP_OPAQUE,
     + PYXIS__MAP_TRANSPARENT,
     + PYXIS__MAP_TAG,
     + PYXIS__MAP_SUB_FORM,
     + PYXIS__MAP_ITEM_INDEX,
     + PYXIS__MAP_SUB_FIELD,
     + PYXIS__MAP_END

      PARAMETER (
     + PYXIS__MAP_VERSION1                    = 1,
     + PYXIS__MAP_FIELD2                      = 2,
     + PYXIS__MAP_FIELD1                      = 3,
     + PYXIS__MAP_MESSAGE                     = 4,
     + PYXIS__MAP_TERMINATOR                  = 5,
     + PYXIS__MAP_TERMINATING_FIELD           = 6,
     + PYXIS__MAP_OPAQUE                      = 7,
     + PYXIS__MAP_TRANSPARENT                 = 8,
     + PYXIS__MAP_TAG                         = 9,
     + PYXIS__MAP_SUB_FORM                    = 10,
     + PYXIS__MAP_ITEM_INDEX                  = 11,
     + PYXIS__MAP_SUB_FIELD                   = 12,
     + PYXIS__MAP_END                         = -1)

C  Field option flags for display options 

      INTEGER*2
     + PYXIS__OPT_DISPLAY,
     + PYXIS__OPT_UPDATE,
     + PYXIS__OPT_WAKEUP,
     + PYXIS__OPT_POSITION

      PARAMETER (
     + PYXIS__OPT_DISPLAY                     = 1,
     + PYXIS__OPT_UPDATE                      = 2,
     + PYXIS__OPT_WAKEUP                      = 4,
     + PYXIS__OPT_POSITION                    = 8)

C  Field option values following display 

      INTEGER*2
     + PYXIS__OPT_NULL,
     + PYXIS__OPT_DEFAULT,
     + PYXIS__OPT_INITIAL,
     + PYXIS__OPT_USER_DATA

      PARAMETER (
     + PYXIS__OPT_NULL                        = 1,
     + PYXIS__OPT_DEFAULT                     = 2,
     + PYXIS__OPT_INITIAL                     = 3,
     + PYXIS__OPT_USER_DATA                   = 4)

C  Pseudo key definitions 

      INTEGER*2
     + PYXIS__KEY_DELETE,
     + PYXIS__KEY_UP,
     + PYXIS__KEY_DOWN,
     + PYXIS__KEY_RIGHT,
     + PYXIS__KEY_LEFT,
     + PYXIS__KEY_PF1,
     + PYXIS__KEY_PF2,
     + PYXIS__KEY_PF3,
     + PYXIS__KEY_PF4,
     + PYXIS__KEY_PF5,
     + PYXIS__KEY_PF6,
     + PYXIS__KEY_PF7,
     + PYXIS__KEY_PF8,
     + PYXIS__KEY_PF9,
     + PYXIS__KEY_ENTER,
     + PYXIS__KEY_SCROLL_TOP,
     + PYXIS__KEY_SCROLL_BOTTOM

      PARAMETER (
     + PYXIS__KEY_DELETE                      = 127,
     + PYXIS__KEY_UP                          = 128,
     + PYXIS__KEY_DOWN                        = 129,
     + PYXIS__KEY_RIGHT                       = 130,
     + PYXIS__KEY_LEFT                        = 131,
     + PYXIS__KEY_PF1                         = 132,
     + PYXIS__KEY_PF2                         = 133,
     + PYXIS__KEY_PF3                         = 134,
     + PYXIS__KEY_PF4                         = 135,
     + PYXIS__KEY_PF5                         = 136,
     + PYXIS__KEY_PF6                         = 137,
     + PYXIS__KEY_PF7                         = 138,
     + PYXIS__KEY_PF8                         = 139,
     + PYXIS__KEY_PF9                         = 140,
     + PYXIS__KEY_ENTER                       = 141,
     + PYXIS__KEY_SCROLL_TOP                  = 146,
     + PYXIS__KEY_SCROLL_BOTTOM               = 147)

C  Menu definition stuff 

      INTEGER*2
     + PYXIS__MENU_VERSION1,
     + PYXIS__MENU_LABEL,
     + PYXIS__MENU_ENTREE,
     + PYXIS__MENU_OPAQUE,
     + PYXIS__MENU_TRANSPARENT,
     + PYXIS__MENU_HORIZONTAL,
     + PYXIS__MENU_VERTICAL,
     + PYXIS__MENU_END

      PARAMETER (
     + PYXIS__MENU_VERSION1                   = 1,
     + PYXIS__MENU_LABEL                      = 2,
     + PYXIS__MENU_ENTREE                     = 3,
     + PYXIS__MENU_OPAQUE                     = 4,
     + PYXIS__MENU_TRANSPARENT                = 5,
     + PYXIS__MENU_HORIZONTAL                 = 6,
     + PYXIS__MENU_VERTICAL                   = 7,
     + PYXIS__MENU_END                        = -1)


C     Error Codes

      INTEGER*4
     + GDS_FACILITY,
     + GDS_ERR_BASE,
     + GDS_ERR_FACTOR

      INTEGER*4
     + GDS_ARG_END,          !{ end of argument list }
     + GDS_ARG_GDS,          !{ generic DSRI status value }
     + GDS_ARG_STRING,       !{ string argument }
     + GDS_ARG_CSTRING,      !{ count & string argument }
     + GDS_ARG_NUMBER,       !{ numeric argument (long) }
     + GDS_ARG_INTERPRETED,  !{ interpreted status code (string) }
     + GDS_ARG_VMS,          !{ VAX/VMS status code (long) }
     + GDS_ARG_UNIX,         !{ UNIX error code }
     + GDS_ARG_DOMAIN        !{ Apollo/Domain error code }


      PARAMETER (
     + GDS_FACILITY    = 20,
     + GDS_ERR_BASE    = 335544320,
     + GDS_ERR_FACTOR  = 1)

      PARAMETER (
     + GDS_ARG_END          = 0,   !{ end of argument list }
     + GDS_ARG_GDS          = 1,   !{ generic DSRI status value }
     + GDS_ARG_STRING       = 2,   !{ string argument }
     + GDS_ARG_CSTRING      = 3,   !{ count & string argument }
     + GDS_ARG_NUMBER       = 4,   !{ numeric argument (long) }
     + GDS_ARG_INTERPRETED  = 5,   !{ interpreted status code (string) }
     + GDS_ARG_VMS          = 6,   !{ VAX/VMS status code (long) }
     + GDS_ARG_UNIX         = 7,   !{ UNIX error code }
     + GDS_ARG_DOMAIN       = 8)   !{ Apollo/Domain error code }


      INTEGER*4 GDS__arith_except        
      PARAMETER (GDS__arith_except         = 335544321)
      INTEGER*4 GDS__bad_dbkey           
      PARAMETER (GDS__bad_dbkey            = 335544322)
      INTEGER*4 GDS__bad_db_format       
      PARAMETER (GDS__bad_db_format        = 335544323)
      INTEGER*4 GDS__bad_db_handle       
      PARAMETER (GDS__bad_db_handle        = 335544324)
      INTEGER*4 GDS__bad_dpb_content     
      PARAMETER (GDS__bad_dpb_content      = 335544325)
      INTEGER*4 GDS__bad_dpb_form        
      PARAMETER (GDS__bad_dpb_form         = 335544326)
      INTEGER*4 GDS__bad_req_handle      
      PARAMETER (GDS__bad_req_handle       = 335544327)
      INTEGER*4 GDS__bad_segstr_handle   
      PARAMETER (GDS__bad_segstr_handle    = 335544328)
      INTEGER*4 GDS__bad_segstr_id       
      PARAMETER (GDS__bad_segstr_id        = 335544329)
      INTEGER*4 GDS__bad_tpb_content     
      PARAMETER (GDS__bad_tpb_content      = 335544330)
      INTEGER*4 GDS__bad_tpb_form        
      PARAMETER (GDS__bad_tpb_form         = 335544331)
      INTEGER*4 GDS__bad_trans_handle    
      PARAMETER (GDS__bad_trans_handle     = 335544332)
      INTEGER*4 GDS__bug_check           
      PARAMETER (GDS__bug_check            = 335544333)
      INTEGER*4 GDS__convert_error       
      PARAMETER (GDS__convert_error        = 335544334)
      INTEGER*4 GDS__db_corrupt          
      PARAMETER (GDS__db_corrupt           = 335544335)
      INTEGER*4 GDS__deadlock            
      PARAMETER (GDS__deadlock             = 335544336)
      INTEGER*4 GDS__from_no_match       
      PARAMETER (GDS__from_no_match        = 335544338)
      INTEGER*4 GDS__infinap             
      PARAMETER (GDS__infinap              = 335544339)
      INTEGER*4 GDS__infona              
      PARAMETER (GDS__infona               = 335544340)
      INTEGER*4 GDS__infunk              
      PARAMETER (GDS__infunk               = 335544341)
      INTEGER*4 GDS__integ_fail          
      PARAMETER (GDS__integ_fail           = 335544342)
      INTEGER*4 GDS__invalid_blr         
      PARAMETER (GDS__invalid_blr          = 335544343)
      INTEGER*4 GDS__io_error            
      PARAMETER (GDS__io_error             = 335544344)
      INTEGER*4 GDS__lock_conflict       
      PARAMETER (GDS__lock_conflict        = 335544345)
      INTEGER*4 GDS__metadata_corrupt    
      PARAMETER (GDS__metadata_corrupt     = 335544346)
      INTEGER*4 GDS__not_valid           
      PARAMETER (GDS__not_valid            = 335544347)
      INTEGER*4 GDS__no_cur_rec          
      PARAMETER (GDS__no_cur_rec           = 335544348)
      INTEGER*4 GDS__no_dup              
      PARAMETER (GDS__no_dup               = 335544349)
      INTEGER*4 GDS__no_finish           
      PARAMETER (GDS__no_finish            = 335544350)
      INTEGER*4 GDS__no_meta_update      
      PARAMETER (GDS__no_meta_update       = 335544351)
      INTEGER*4 GDS__no_priv             
      PARAMETER (GDS__no_priv              = 335544352)
      INTEGER*4 GDS__no_recon            
      PARAMETER (GDS__no_recon             = 335544353)
      INTEGER*4 GDS__no_record           
      PARAMETER (GDS__no_record            = 335544354)
      INTEGER*4 GDS__no_segstr_close     
      PARAMETER (GDS__no_segstr_close      = 335544355)
      INTEGER*4 GDS__obsolete_metadata   
      PARAMETER (GDS__obsolete_metadata    = 335544356)
      INTEGER*4 GDS__open_trans          
      PARAMETER (GDS__open_trans           = 335544357)
      INTEGER*4 GDS__port_len            
      PARAMETER (GDS__port_len             = 335544358)
      INTEGER*4 GDS__read_only_field     
      PARAMETER (GDS__read_only_field      = 335544359)
      INTEGER*4 GDS__read_only_rel       
      PARAMETER (GDS__read_only_rel        = 335544360)
      INTEGER*4 GDS__read_only_trans     
      PARAMETER (GDS__read_only_trans      = 335544361)
      INTEGER*4 GDS__read_only_view      
      PARAMETER (GDS__read_only_view       = 335544362)
      INTEGER*4 GDS__req_no_trans        
      PARAMETER (GDS__req_no_trans         = 335544363)
      INTEGER*4 GDS__req_sync            
      PARAMETER (GDS__req_sync             = 335544364)
      INTEGER*4 GDS__req_wrong_db        
      PARAMETER (GDS__req_wrong_db         = 335544365)
      INTEGER*4 GDS__segment             
      PARAMETER (GDS__segment              = 335544366)
      INTEGER*4 GDS__segstr_eof          
      PARAMETER (GDS__segstr_eof           = 335544367)
      INTEGER*4 GDS__segstr_no_op        
      PARAMETER (GDS__segstr_no_op         = 335544368)
      INTEGER*4 GDS__segstr_no_read      
      PARAMETER (GDS__segstr_no_read       = 335544369)
      INTEGER*4 GDS__segstr_no_trans     
      PARAMETER (GDS__segstr_no_trans      = 335544370)
      INTEGER*4 GDS__segstr_no_write     
      PARAMETER (GDS__segstr_no_write      = 335544371)
      INTEGER*4 GDS__segstr_wrong_db     
      PARAMETER (GDS__segstr_wrong_db      = 335544372)
      INTEGER*4 GDS__sys_request         
      PARAMETER (GDS__sys_request          = 335544373)
      INTEGER*4 GDS__unavailable         
      PARAMETER (GDS__unavailable          = 335544375)
      INTEGER*4 GDS__unres_rel           
      PARAMETER (GDS__unres_rel            = 335544376)
      INTEGER*4 GDS__uns_ext             
      PARAMETER (GDS__uns_ext              = 335544377)
      INTEGER*4 GDS__wish_list           
      PARAMETER (GDS__wish_list            = 335544378)
      INTEGER*4 GDS__wrong_ods           
      PARAMETER (GDS__wrong_ods            = 335544379)
      INTEGER*4 GDS__wronumarg           
      PARAMETER (GDS__wronumarg            = 335544380)
      INTEGER*4 GDS__imp_exc             
      PARAMETER (GDS__imp_exc              = 335544381)
      INTEGER*4 GDS__random              
      PARAMETER (GDS__random               = 335544382)
      INTEGER*4 GDS__fatal_conflict      
      PARAMETER (GDS__fatal_conflict       = 335544383)

C     Minor codes subject to change 

      INTEGER*4 GDS__badblk              
      PARAMETER (GDS__badblk               = 335544384)
      INTEGER*4 GDS__invpoolcl           
      PARAMETER (GDS__invpoolcl            = 335544385)
      INTEGER*4 GDS__nopoolids           
      PARAMETER (GDS__nopoolids            = 335544386)
      INTEGER*4 GDS__relbadblk           
      PARAMETER (GDS__relbadblk            = 335544387)
      INTEGER*4 GDS__blktoobig           
      PARAMETER (GDS__blktoobig            = 335544388)
      INTEGER*4 GDS__bufexh              
      PARAMETER (GDS__bufexh               = 335544389)
      INTEGER*4 GDS__syntaxerr           
      PARAMETER (GDS__syntaxerr            = 335544390)
      INTEGER*4 GDS__bufinuse            
      PARAMETER (GDS__bufinuse             = 335544391)
      INTEGER*4 GDS__bdbincon            
      PARAMETER (GDS__bdbincon             = 335544392)
      INTEGER*4 GDS__reqinuse            
      PARAMETER (GDS__reqinuse             = 335544393)
      INTEGER*4 GDS__badodsver           
      PARAMETER (GDS__badodsver            = 335544394)
      INTEGER*4 GDS__relnotdef           
      PARAMETER (GDS__relnotdef            = 335544395)
      INTEGER*4 GDS__fldnotdef           
      PARAMETER (GDS__fldnotdef            = 335544396)
      INTEGER*4 GDS__dirtypage           
      PARAMETER (GDS__dirtypage            = 335544397)
      INTEGER*4 GDS__waifortra           
      PARAMETER (GDS__waifortra            = 335544398)
      INTEGER*4 GDS__doubleloc           
      PARAMETER (GDS__doubleloc            = 335544399)
      INTEGER*4 GDS__nodnotfnd           
      PARAMETER (GDS__nodnotfnd            = 335544400)
      INTEGER*4 GDS__dupnodfnd           
      PARAMETER (GDS__dupnodfnd            = 335544401)
      INTEGER*4 GDS__locnotmar           
      PARAMETER (GDS__locnotmar            = 335544402)
      INTEGER*4 GDS__badpagtyp           
      PARAMETER (GDS__badpagtyp            = 335544403)
      INTEGER*4 GDS__corrupt             
      PARAMETER (GDS__corrupt              = 335544404)
      INTEGER*4 GDS__badpage             
      PARAMETER (GDS__badpage              = 335544405)
      INTEGER*4 GDS__badindex            
      PARAMETER (GDS__badindex             = 335544406)
      INTEGER*4 GDS__dbbnotzer           
      PARAMETER (GDS__dbbnotzer            = 335544407)
      INTEGER*4 GDS__tranotzer           
      PARAMETER (GDS__tranotzer            = 335544408)
      INTEGER*4 GDS__trareqmis           
      PARAMETER (GDS__trareqmis            = 335544409)
      INTEGER*4 GDS__badhndcnt           
      PARAMETER (GDS__badhndcnt            = 335544410)
      INTEGER*4 GDS__wrotpbver           
      PARAMETER (GDS__wrotpbver            = 335544411)
      INTEGER*4 GDS__wroblrver           
      PARAMETER (GDS__wroblrver            = 335544412)
      INTEGER*4 GDS__wrodpbver           
      PARAMETER (GDS__wrodpbver            = 335544413)
      INTEGER*4 GDS__blobnotsup          
      PARAMETER (GDS__blobnotsup           = 335544414)
      INTEGER*4 GDS__badrelation         
      PARAMETER (GDS__badrelation          = 335544415)
      INTEGER*4 GDS__nodetach            
      PARAMETER (GDS__nodetach             = 335544416)
      INTEGER*4 GDS__notremote           
      PARAMETER (GDS__notremote            = 335544417)
      INTEGER*4 GDS__trainlim            
      PARAMETER (GDS__trainlim             = 335544418)
      INTEGER*4 GDS__notinlim            
      PARAMETER (GDS__notinlim             = 335544419)
      INTEGER*4 GDS__traoutsta           
      PARAMETER (GDS__traoutsta            = 335544420)
      INTEGER*4 GDS__connect_reject      
      PARAMETER (GDS__connect_reject       = 335544421)
      INTEGER*4 GDS__dbfile              
      PARAMETER (GDS__dbfile               = 335544422)
      INTEGER*4 GDS__orphan              
      PARAMETER (GDS__orphan               = 335544423)
      INTEGER*4 GDS__no_lock_mgr         
      PARAMETER (GDS__no_lock_mgr          = 335544424)
      INTEGER*4 GDS__ctxinuse            
      PARAMETER (GDS__ctxinuse             = 335544425)
      INTEGER*4 GDS__ctxnotdef           
      PARAMETER (GDS__ctxnotdef            = 335544426)
      INTEGER*4 GDS__datnotsup           
      PARAMETER (GDS__datnotsup            = 335544427)
      INTEGER*4 GDS__badmsgnum           
      PARAMETER (GDS__badmsgnum            = 335544428)
      INTEGER*4 GDS__badparnum           
      PARAMETER (GDS__badparnum            = 335544429)
      INTEGER*4 GDS__virmemexh           
      PARAMETER (GDS__virmemexh            = 335544430)
      INTEGER*4 GDS__blocking_signal     
      PARAMETER (GDS__blocking_signal      = 335544431)
      INTEGER*4 GDS__lockmanerr          
      PARAMETER (GDS__lockmanerr           = 335544432)
      INTEGER*4 GDS__journerr            
      PARAMETER (GDS__journerr             = 335544433)
      INTEGER*4 GDS__wrodynver           
      PARAMETER (GDS__wrodynver            = 335544437)
      INTEGER*4 gds__obj_in_use
      PARAMETER (gds__obj_in_use           = 335544453)
      INTEGER*4 gds__no_filter
      PARAMETER (gds__no_filter            = 335544454)
      INTEGER*4 gds__shadow_accessed
      PARAMETER (gds__shadow_accessed      = 335544455)
      INTEGER*4 gds__invalid_sdl
      PARAMETER (gds__invalid_sdl          = 335544456)
      INTEGER*4 gds__out_of_bounds
      PARAMETER (gds__out_of_bounds        = 335544457)
      INTEGER*4 gds__invalid_dimension
      PARAMETER (gds__invalid_dimension    = 335544458)
      INTEGER*4 gds__rec_in_limbo
      PARAMETER (gds__rec_in_limbo = 335544459)
      INTEGER*4 gds__shadow_missing
      PARAMETER (gds__shadow_missing = 335544460)
      INTEGER*4 gds__cant_validate
      PARAMETER (gds__cant_validate = 335544461)
      INTEGER*4 gds__cant_start_journal
      PARAMETER (gds__cant_start_journal = 335544462)
      INTEGER*4 gds__gennotdef
      PARAMETER (gds__gennotdef = 335544463)
      INTEGER*4 gds__cant_start_logging
      PARAMETER (gds__cant_start_logging = 335544464)
      INTEGER*4 gds__bad_segstr_type
      PARAMETER (gds__bad_segstr_type = 335544465)
      INTEGER*4 gds__foreign_key
      PARAMETER (gds__foreign_key = 335544466)
      INTEGER*4 gds__high_minor
      PARAMETER (gds__high_minor = 335544467)
      INTEGER*4 gds__tra_state
      PARAMETER (gds__tra_state = 335544468)
      INTEGER*4 gds__trans_invalid
      PARAMETER (gds__trans_invalid = 335544469)
      INTEGER*4 gds__buf_invalid
      PARAMETER (gds__buf_invalid = 335544470)
      INTEGER*4 gds__indexnotdefined
      PARAMETER (gds__indexnotdefined = 335544471)
      INTEGER*4 gds__login
      PARAMETER (gds__login = 335544472)
      INTEGER*4 gds__invalid_bookmark
      PARAMETER (gds__invalid_bookmark = 335544473)
      INTEGER*4 gds__bad_lock_level
      PARAMETER (gds__bad_lock_level = 335544474)
      INTEGER*4 gds__relation_lock
      PARAMETER (gds__relation_lock = 335544475)
      INTEGER*4 gds__record_lock
      PARAMETER (gds__record_lock = 335544476)
      INTEGER*4 gds__max_idx
      PARAMETER (gds__max_idx = 335544477)
      INTEGER*4 gds__jrn_enable
      PARAMETER (gds__jrn_enable = 335544478)
      INTEGER*4 gds__old_failure
      PARAMETER (gds__old_failure = 335544479)
      INTEGER*4 gds__old_in_porgress
      PARAMETER (gds__old_in_progress = 335544480)
      INTEGER*4 gds__old_no_space
      PARAMETER (gds__old_no_space = 335544481)
      INTEGER*4 gds__no_wal_no_jrn
      PARAMETER (gds__no_wal_no_jrn = 335544482)
      INTEGER*4 gds__num_old_files
      PARAMETER (gds__num_old_files = 335544483)
      INTEGER*4 gds__wal_file_open
      PARAMETER (gds__wal_file_open = 335544484)
      INTEGER*4 gds__bad_stmt_handle
      PARAMETER (gds__bad_stmt_handle = 335544485)
      INTEGER*4 gds__wal_failure
      PARAMETER (gds__wal_failue = 335544486)
      INTEGER*4 gds__walw_err
      PARAMETER (gds__walw_err = 335544487)
      INTEGER*4 gds__logh_small
      PARAMETER (gds__logh_small = 335544488)
      INTEGER*4 gds__logh_inv_version
      PARAMETER (gds__logh_inv_version = 335544489)
      INTEGER*4 gds__logh_open_flag
      PARAMETER (gds__logh_open_flag = 335544490)
      INTEGER*4 gds__logh_open_flag2
      PARAMETER (gds__logh_open_flag2 = 335544491)
      INTEGER*4 gds__logh_diff_dbname
      PARAMETER (gds__logh_diff_dbname = 335544492)
      INTEGER*4 gds__logf_unexpected_eof
      PARAMETER (gds__logf_unexpected_eof = 335544493)
      INTEGER*4 gds__logr_incomplete
      PARAMETER (gds__logr_incomplete = 335544494)
      INTEGER*4 gds__logr_header_small
      PARAMETER (gds__logr_header_small = 335544495)
      INTEGER*4 gds__logb_small
      PARAMETER (gds__logb_small = 335544496)
      INTEGER*4 gds__wal_illegal_attach
      PARAMETER (gds__wal_illegal_attach = 335544497)
      INTEGER*4 gds__wal_invalid_wpb
      PARAMETER (gds__wal_invalid_wpb = 335544498)
      INTEGER*4 gds__wal_err_rollover
      PARAMETER (gds__wal_err_rollover = 335544499)
      INTEGER*4 gds__no_wal
      PARAMETER (gds__no_wal = 335544500)
      INTEGER*4 gds__drop_wal
      PARAMETER (gds__drop_wal = 335544501)
      INTEGER*4 gds__stream_not_defined
      PARAMETER (gds__stream_not_defined = 335544502)
      INTEGER*4 gds__unknown_183
      PARAMETER (gds__unknown_183 = 335544503)
      INTEGER*4 gds__wal_subsys_error
      PARAMETER (gds__wal_subsys_error = 335544504)
      INTEGER*4 gds__wal_subsys_corrupt
      PARAMETER (gds__wal_subsys_corrupt = 335544505)
      INTEGER*4 gds__no_archive
      PARAMETER (gds__no_archive = 335544506)
      INTEGER*4 gds__store_proc_failed
      PARAMETER (gds__store_proc_failed = 335544507)
      INTEGER*4 gds__range_in_use
      PARAMETER (gds__range_in_use = 335544508)
      INTEGER*4 gds__range_not_found
      PARAMETER (gds__range_not_found = 335544508)
      INTEGER*4 gds__charset_not_found
      PARAMETER (gds__charset_not_found = 335544510)
      INTEGER*4 gds__lock_timeout
      PARAMETER (gds__lock_timeout = 335544511)
      INTEGER*4 gds__prcnotdef
      PARAMETER (gds__prcnotdef = 335544512)
      INTEGER*4 gds__prcmismat
      PARAMETER (gds__prcmismat = 335544513)




