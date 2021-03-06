/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		jrd.cpp
 *	DESCRIPTION:	User visible entrypoints
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
 * 2001.07.06 Sean Leyne - Code Cleanup, removed "#ifdef READONLY_DATABASE"
 *                         conditionals, as the engine now fully supports
 *                         readonly databases.
 * 2001.07.09 Sean Leyne - Restore default setting to Force Write = "On", for
 *                         Windows NT platform, for new database files. This was changed
 *                         with IB 6.0 to OFF and has introduced many reported database
 *                         corruptions.
 *
 * 2002.10.29 Sean Leyne - Removed obsolete "Netware" port
 *
 */

#include "fbdev.h"
#include "../jrd/ib_stdio.h"
#include <string.h>
#include <stdlib.h>
#include "../jrd/common.h"

#ifdef THREAD_SCHEDULER
#include "../jrd/os/thd_priority.h"
#endif

#include <stdarg.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#include <errno.h>

#define JRD_MAIN
#include "../jrd/ibase.h"
#include "../jrd/jrd.h"
#include "../jrd/Relation.h"
#include "../jrd/irq.h"
#include "../jrd/isc.h"
#include "../jrd/drq.h"
#include "../jrd/req.h"
#include "../jrd/tra.h"
#include "../jrd/blb.h"
#include "../jrd/scl.h"
#include "../jrd/license.h"
#include "../jrd/os/pio.h"
#include "../jrd/ods.h"
#include "../jrd/exe.h"
#include "../jrd/val.h"
#include "../jrd/rse.h"
#include "../jrd/jrn.h"
#include "../jrd/all.h"
#include "../jrd/log.h"
#include "../jrd/fil.h"
#include "../jrd/sbm.h"
//#include "../jrd/jrd_pwd.h"
#include "OSRIException.h"
#include "PageCache.h"
#include "DatabaseManager.h"
#include "Sync.h"
#include "Format.h"
#include "Attachment.h"

#ifdef SERVICES
#include "../jrd/svc.h"
#endif

#include "../jrd/sdw.h"
#include "../jrd/lls.h"
#include "../jrd/cch.h"
#include "../jrd/nbak.h"
#include "../jrd/iberr.h"
#include "../jrd/jrd_time.h"
#include "../intlcpp/charsets.h"
#include "../jrd/sort.h"

#include "../jrd/all_proto.h"
#include "../jrd/blb_proto.h"
#include "../jrd/cch_proto.h"
#include "../jrd/cmp_proto.h"
#include "../jrd/dbg_proto.h"
#include "../jrd/dls_proto.h"
#include "../jrd/dyn_proto.h"
#include "../jrd/err_proto.h"
#include "../jrd/exe_proto.h"
#include "../jrd/ext_proto.h"
#include "../jrd/fun_proto.h"
#include "../jrd/gds_proto.h"
#include "../jrd/iberr_proto.h"
#include "../jrd/inf_proto.h"
#include "../jrd/ini_proto.h"
#include "../jrd/intl_proto.h"
#include "../jrd/inuse_proto.h"
#include "../jrd/isc_f_proto.h"
#include "../jrd/isc_proto.h"
#include "../jrd/jrd_proto.h"
#include "../jrd/jrn_proto.h"

#include "../jrd/lck_proto.h"
#include "../jrd/log_proto.h"
#include "../jrd/met_proto.h"
#include "../jrd/mov_proto.h"
#include "../jrd/pag_proto.h"
#include "../jrd/par_proto.h"
#include "../jrd/os/pio_proto.h"
#include "../jrd/sch_proto.h"
#include "../jrd/scl_proto.h"
#include "../jrd/sdw_proto.h"
#include "../jrd/shut_proto.h"
#include "../jrd/sort_proto.h"

#ifdef SERVICES
#include "../jrd/svc_proto.h"
#endif

#include "../jrd/thd_proto.h"
#include "../jrd/tra_proto.h"
#include "../jrd/val_proto.h"
#include "../jrd/file_params.h"
#include "../jrd/event_proto.h"
#include "../jrd/flags.h"

//#define PLUGIN_MANAGER

#ifdef PLUGIN_MANAGER
#include "../jrd/plugin_manager.h"
#endif

#include "../jrd/ThreadData.h"

#include "ConfObj.h"
#include "ConfObject.h"
#include "Configuration.h"
#include "Parameters.h"
#include "PathName.h"

#include "Connect.h"
#include "PStatement.h"
#include "RSet.h"
#include "EngineInfo.h"

#ifdef GARBAGE_THREAD
#include "vio_proto.h"
#endif

#ifdef SERVER_SHUTDOWN
typedef struct dbf {
	struct dbf* dbf_next;
	USHORT dbf_length;
	TEXT dbf_data[2];
} *DBF;

//#include "../jrd/sort.h"
#endif /* SERVER_SHUTDOWN */

#define	WAIT_PERIOD	-1

#ifdef SUPPORT_RAW_DEVICES
#define unlink PIO_unlink
#endif

#ifdef SUPERSERVER
#define V4_THREADING

#endif /* SUPERSERVER */

# ifdef V4_THREADING
#  undef V4_INIT
#  undef V4_GLOBAL_MUTEX_LOCK
#  undef V4_GLOBAL_MUTEX_UNLOCK
#  undef V4_MUTEX_INIT
#  undef V4_MUTEX_LOCK
#  undef V4_MUTEX_UNLOCK
#  undef V4_MUTEX_DESTROY
#  undef V4_JRD_MUTEX_LOCK
#  undef V4_JRD_MUTEX_UNLOCK
/***
#  define V4_INIT			THD_INIT
#  define V4_GLOBAL_MUTEX_LOCK	{THREAD_EXIT; THD_GLOBAL_MUTEX_LOCK; THREAD_ENTER;}
#  define V4_GLOBAL_MUTEX_UNLOCK	THD_GLOBAL_MUTEX_UNLOCK
#  define V4_MUTEX_INIT(mutx)	THD_MUTEX_INIT (mutx)
#  define V4_MUTEX_LOCK(mutx)	{THREAD_EXIT; THD_MUTEX_LOCK (mutx); THREAD_ENTER;}
#  define V4_MUTEX_UNLOCK(mutx)	THD_MUTEX_UNLOCK (mutx)
#  define V4_MUTEX_DESTROY(mutx)	THD_MUTEX_DESTROY (mutx)
#  ifndef SUPERSERVER
#   define V4_JRD_MUTEX_LOCK(mutx)	{THREAD_EXIT; THD_JRD_MUTEX_LOCK (mutx); THREAD_ENTER;}
#   define V4_JRD_MUTEX_UNLOCK(mutx) THD_JRD_MUTEX_UNLOCK (mutx)
#  endif // SUPERSERVER
***/
# endif /* V4_THREADING */

// BRS. 03/23/2003
// Those two macros are defined in thd.h when V4_THREADING is true, but thd.h is included
// before V4_THREADING is defined, so the two must be defined as empty if not defined 
// to allow the use of V4_THREADING ifdefs
// The include chain is
// os/thd_priority.h -> thd.h

//#if defined(V4_THREADING) && !defined(V4_RW_LOCK_DESTROY_N)
//#   define V4_RW_LOCK_DESTROY_N(wlck,n)
//#   define V4_RW_LOCK_INIT_N(wlck,n)
//#endif

#ifdef SUPERSERVER

//extern SLONG trace_pools;
//static REC_MUTX_T databases_rec_mutex;

// BRS. 03/23/2003
// Those empty defines was substituted with #if defined(V4_THREADING) && !defined(SUPERSERVER)
//#define V4_JRD_MUTEX_LOCK(mutx)
//#define V4_JRD_MUTEX_UNLOCK(mutx)

/***
#define JRD_SS_INIT_MUTEX       THD_rec_mutex_init (&databases_rec_mutex)
#define JRD_SS_DESTROY_MUTEX    THD_rec_mutex_destroy (&databases_rec_mutex)
#define JRD_SS_MUTEX_LOCK       {THREAD_EXIT;\
                                 THD_rec_mutex_lock (&databases_rec_mutex);\
                                 THREAD_ENTER;}
#define JRD_SS_MUTEX_UNLOCK     THD_rec_mutex_unlock (&databases_rec_mutex)
#define JRD_SS_THD_MUTEX_LOCK   THD_rec_mutex_lock (&databases_rec_mutex)
#define JRD_SS_THD_MUTEX_UNLOCK THD_rec_mutex_unlock (&databases_rec_mutex)

#else
#define JRD_SS_INIT_MUTEX
#define JRD_SS_DESTROY_MUTEX
#define JRD_SS_MUTEX_LOCK
#define JRD_SS_MUTEX_UNLOCK
#define JRD_SS_THD_MUTEX_LOCK
#define JRD_SS_THD_MUTEX_UNLOCK
***/
#endif

#ifdef  WIN_NT
#include <windows.h>
/* these should stop a most annoying warning */
#undef TEXT
#define TEXT    SCHAR
#endif	// WIN_NT


#ifdef WIN_NT
#define	SYS_ERR		isc_arg_win32
#endif


#ifndef SYS_ERR
#define SYS_ERR		isc_arg_unix
#endif


/* Option block for database parameter block */

typedef struct dpb
{
	TEXT*	dpb_sys_user_name;
	TEXT*	dpb_user_name;
	TEXT*	dpb_password;
	TEXT*	dpb_password_enc;
	TEXT*	dpb_role_name;
	TEXT*	dpb_journal;
	TEXT*	dpb_key;
	TEXT*	dpb_log;
	TEXT*	dpb_wal_backup_dir;
	USHORT	dpb_wal_action;
	USHORT	dpb_online_dump;
	ULONG	dpb_old_file_size;
	USHORT	dpb_old_num_files;
	ULONG	dpb_old_start_page;
	ULONG	dpb_old_start_seqno;
	USHORT	dpb_old_start_file;
	TEXT*	dpb_old_file[MAX_OLD_FILES];
	USHORT	dpb_old_dump_id;
	SLONG	dpb_wal_chkpt_len;
	SSHORT	dpb_wal_num_bufs;
	USHORT	dpb_wal_bufsize;
	SLONG	dpb_wal_grp_cmt_wait;
	SLONG	dpb_sweep_interval;
	ULONG	dpb_page_buffers;
	BOOLEAN	dpb_set_page_buffers;
	ULONG dpb_buffers;
	USHORT	dpb_debug;
	USHORT	dpb_verify;
	USHORT	dpb_sweep;
	USHORT	dpb_trace;
	USHORT	dpb_disable;
	USHORT	dpb_dbkey_scope;
	USHORT	dpb_page_size;
	bool	dpb_activate_shadow;
	bool	dpb_delete_shadow;
	USHORT	dpb_no_garbage;
	USHORT	dpb_quit_log;
	USHORT	dpb_shutdown;
	SSHORT	dpb_shutdown_delay;
	USHORT	dpb_online;
	SSHORT	dpb_force_write;
	UCHAR	dpb_set_force_write;
	UCHAR	dpb_no_reserve;
	UCHAR	dpb_set_no_reserve;
	SSHORT	dpb_interp;
	TEXT*	dpb_lc_messages;
	TEXT*	dpb_lc_ctype;
	USHORT	dpb_single_user;
	BOOLEAN	dpb_overwrite;
	BOOLEAN	dpb_sec_attach;
	BOOLEAN	dpb_disable_wal;
	SLONG	dpb_connect_timeout;
	SLONG	dpb_dummy_packet_interval;
	SSHORT	dpb_db_readonly;
	BOOLEAN	dpb_set_db_readonly;
	BOOLEAN	dpb_gfix_attach;
	BOOLEAN	dpb_gstat_attach;
	TEXT*	dpb_gbak_attach;
	TEXT*	dpb_working_directory;
	USHORT	dpb_sql_dialect;
	USHORT	dpb_set_db_sql_dialect;
	TEXT*	dpb_set_db_charset;
} DPB;

static blb*		check_blob(thread_db*, ISC_STATUS*, blb**);
static ISC_STATUS	check_database(thread_db*, Attachment*);
static void		cleanup(void*);
static ISC_STATUS	commit(ISC_STATUS*, Transaction**, const bool);
static STR		copy_string(const TEXT*, int);
static bool		drop_files(const File*);
static ISC_STATUS	error(OSRIException *exception, ISC_STATUS*);
static void		find_intl_charset(thread_db*, Attachment*, const DPB*);
static Transaction* find_transaction(thread_db*, Transaction*, ISC_STATUS);
static void		get_options(thread_db* tdbb, const UCHAR*, USHORT, TEXT**, ULONG, DPB*);
static SLONG	get_parameter(const UCHAR**);
static TEXT*	get_string_parameter(const UCHAR**, TEXT**, ULONG*);
static ISC_STATUS	handle_error(ISC_STATUS*, ISC_STATUS, thread_db*);
//static void		verify_request_synchronization(Request*& request, SSHORT level);
static bool		verify_database_name(const TEXT*, ISC_STATUS*);


#if defined (WIN_NT)
#ifdef SERVER_SHUTDOWN
static void		ExtractDriveLetter(const TEXT*, ULONG*);
#else // SERVER_SHUTDOWN
static void		setup_NT_handlers(void);
static BOOLEAN	handler_NT(SSHORT);
#endif	// SERVER_SHUTDOWN
#endif	// WIN_NT

static DBB		init(thread_db*, ISC_STATUS*, const TEXT*, bool, ConfObject *configObject);
static ISC_STATUS	prepare(thread_db*, Transaction*, ISC_STATUS*, USHORT, const UCHAR*);
static void		release_attachment(thread_db* tdbb, Attachment*);
static ISC_STATUS	return_success(thread_db*);
static BOOLEAN	rollback(thread_db*, Transaction*, ISC_STATUS*, const bool);

static void		shutdown_database(thread_db* tdbb, const bool);
static void		strip_quotes(const TEXT*, TEXT*);
static void		purge_attachment(thread_db*, ISC_STATUS*, Attachment*, const bool);

static bool				initialized = false;
static DatabaseManager	databaseManager;
static Mutex			init_mutex;


#ifdef GOVERNOR
#define ATTACHMENTS_PER_USER 1
static ULONG JRD_max_users = 0;
static ULONG num_attached = 0;
#endif /* GOVERNOR */


#if !defined(REQUESTER)

int		debug;

#endif	// !REQUESTER


//____________________________________________________________
//
// check whether we need to perform an autocommit;
// do it here to prevent committing every record update
// in a statement
//
static void check_autocommit(JRD_REQ request, struct thread_db* tdbb)
{
	/* dimitr: we should ignore autocommit for requests
			   created by EXECUTE STATEMENT */
	if (request->req_transaction->tra_callback_count > 0)
		return;

	if (request->req_transaction->tra_flags & TRA_perform_autocommit)
	{
		request->req_transaction->tra_flags &= ~TRA_perform_autocommit;
		TRA_commit(tdbb, request->req_transaction, true);
	}
}

inline static void api_entry_point_init(ISC_STATUS* user_status)
{
	user_status[0] = isc_arg_gds;
	user_status[1] = FB_SUCCESS;
	user_status[2] = isc_arg_end;
}

inline static struct thread_db* set_thread_data(struct thread_db& thd_context)
{
	struct thread_db* tdbb = &thd_context;
	MOVE_CLEAR(tdbb, sizeof(struct thread_db));
	JRD_set_context(tdbb);
	return tdbb;
}


#undef GET_THREAD_DATA
#undef CHECK_DBB
#undef GET_DBB
#undef SET_TDBB

static thread_db* get_thread_data()
{
	THDD p1 = (THDD)(PLATFORM_GET_THREAD_DATA);

	return (thread_db*)p1;
}

inline static void CHECK_DBB(DBB dbb)
{
#ifdef DEV_BUILD
	//fb_assert(dbb && MemoryPool::blk_type(dbb) == type_dbb);
#endif	// DEV_BUILD
}

inline static void check_tdbb(thread_db* tdbb)
{
/***
#ifdef DEV_BUILD
	fb_assert(tdbb &&
			(reinterpret_cast<THDD>(tdbb)->thdd_type == THDD_TYPE_TDBB) &&
			(!tdbb->tdbb_database ||
				MemoryPool::blk_type(tdbb->tdbb_database) == type_dbb));
#endif	// DEV_BUILD
***/
}

inline static DBB get_dbb()
{
	return get_thread_data()->tdbb_database;
}

static void SET_TDBB(thread_db*& tdbb)
{
	if (tdbb == NULL) {
		tdbb = get_thread_data();
	}
	check_tdbb(tdbb);
}

#ifdef SHARED_CACHE
#define lockAST(x)
#define unlockAST(x)
#else
static void lockAST(Database *dbb)
{
	dbb->syncConnection.lock(NULL, Exclusive);
	dbb->syncAst.lock(NULL, Exclusive);
}

static void unlockAST(Database *dbb)
{
	dbb->syncAst.unlock();
	dbb->syncConnection.unlock();
}
#endif

#define CHECK_HANDLE(blk,type,error)					\
	if (!blk )	\
		return handle_error (user_status, error, threadData)

//	if (!blk || MemoryPool::blk_type(blk) != type)	\

#define NULL_CHECK(ptr,code)									\
	if (*ptr) return handle_error (user_status, code, threadData)



#define SWEEP_INTERVAL		20000
#define	DPB_EXPAND_BUFFER	2048

#define	DBL_QUOTE			'\042'
#define	SINGLE_QUOTE		'\''
#define	BUFFER_LENGTH128	128
BOOLEAN invalid_client_SQL_dialect = FALSE;

#define GDS_ATTACH_DATABASE		jrd8_attach_database
#define GDS_BLOB_INFO			jrd8_blob_info
#define GDS_CANCEL_BLOB			jrd8_cancel_blob
#define GDS_CANCEL_EVENTS		jrd8_cancel_events
#define GDS_CANCEL_OPERATION	jrd8_cancel_operation
#define GDS_CLOSE_BLOB			jrd8_close_blob
#define GDS_COMMIT				jrd8_commit_transaction
#define GDS_COMMIT_RETAINING	jrd8_commit_retaining
#define GDS_COMPILE				jrd8_compile_request
#define GDS_CREATE_BLOB2		jrd8_create_blob2
#define GDS_CREATE_DATABASE		jrd8_create_database
#define GDS_DATABASE_INFO		jrd8_database_info
#define GDS_DDL					jrd8_ddl
#define GDS_DETACH				jrd8_detach_database
#define GDS_DROP_DATABASE		jrd8_drop_database
#define GDS_ENGINE_INFO			jrd8_engine_info
#define GDS_GET_SEGMENT			jrd8_get_segment
#define GDS_GET_SLICE			jrd8_get_slice
#define GDS_OPEN_BLOB2			jrd8_open_blob2
#define GDS_PREPARE				jrd8_prepare_transaction
#define GDS_PUT_SEGMENT			jrd8_put_segment
#define GDS_PUT_SLICE			jrd8_put_slice
#define GDS_QUE_EVENTS			jrd8_que_events
#define GDS_RECONNECT			jrd8_reconnect_transaction
#define GDS_RECEIVE				jrd8_receive
#define GDS_RELEASE_REQUEST		jrd8_release_request
#define GDS_REQUEST_INFO		jrd8_request_info
#define GDS_ROLLBACK			jrd8_rollback_transaction
#define GDS_ROLLBACK_RETAINING	jrd8_rollback_retaining
#define GDS_SEEK_BLOB			jrd8_seek_blob
#define GDS_SEND				jrd8_send
#define GDS_SERVICE_ATTACH		jrd8_service_attach
#define GDS_SERVICE_DETACH		jrd8_service_detach
#define GDS_SERVICE_QUERY		jrd8_service_query
#define GDS_SERVICE_START		jrd8_service_start
#define GDS_START_AND_SEND		jrd8_start_and_send
#define GDS_START				jrd8_start_request
#define GDS_START_MULTIPLE		jrd8_start_multiple
#define GDS_START_TRANSACTION	jrd8_start_transaction
#define GDS_TRANSACT_REQUEST	jrd8_transact_request
#define GDS_TRANSACTION_INFO	jrd8_transaction_info
#define GDS_UNWIND				jrd8_unwind_request


/* External hook definitions */

/* dimitr: just uncomment the following line to use this feature.
		   Requires support from the PIO modules. Only Win32 is 100% ready
		   for this so far. Note that the database encryption code in the
		   PIO layer seems to be incompatible with the SUPERSERVER_V2 code.
		   2003.02.09 */
//#define ISC_DATABASE_ENCRYPTION

static const char* CRYPT_IMAGE = "fbcrypt";
static const char* ENCRYPT = "encrypt";
static const char* DECRYPT = "decrypt";


ISC_STATUS GDS_ATTACH_DATABASE(ISC_STATUS* user_status, 
								const TEXT* orgName, 
								const TEXT* translatedName, 
								Attachment** handle, 
								SSHORT dpb_length, 
								const UCHAR* dpb,
								ConfObject* databaseConfiguration,
								ConfObject* providerConfiguration)
{
/**************************************
 *
 *	g d s _ $ a t t a c h _ d a t a b a s e
 *
 **************************************
 *
 * Functional description
 *	Attach a moldy, grungy, old database
 *	sullied by user data.
 *
 **************************************/
	api_entry_point_init(user_status);

	if (*handle)
		return handle_error(user_status, isc_bad_db_handle, 0);

	JString expandedName = PathName::expandFilename (translatedName, dpb_length, dpb);
	ThreadData threadData (user_status);
	
	/* Unless we're already attached, do some initialization */

	if (databaseConfiguration)
		databaseConfiguration->setChain (providerConfiguration);

	DBB dbb = init(threadData, user_status, expandedName, true, databaseConfiguration);
	
	if (!dbb) 
		{
		JRD_restore_context();
		return user_status[1];
		}

	lockAST(dbb);

	/* Hand off database existence lock to local Sync object */
	
#ifdef SHARED_CACHE
	Sync sync(&dbb->syncExistence, "jrd8_attach_database");
	sync.lock(dbb->syncExistence.getState());
	dbb->syncExistence.unlock();
#endif
			
	dbb->dbb_flags |= DBB_being_opened;
	threadData.setDatabase (dbb);
	Attachment* attachment = NULL;
	dbb->incrementUseCount();
	
	try 
		{
		TEXT	opt_buffer[DPB_EXPAND_BUFFER];
		TEXT*	opt_ptr = opt_buffer;

		/* Process database parameter block */
	
		DPB options;
		get_options(threadData, dpb, dpb_length, &opt_ptr, DPB_EXPAND_BUFFER, &options);

#ifndef NO_NFS
		/* Don't check nfs if single user */

		if (!options.dpb_single_user)
#endif
			{
			/* Check to see if the database is truly local or if it just looks
			that way */
	      
			if (ISC_check_if_remote(expandedName, true)) 
				ERR_post(isc_unavailable, 0);
			}

		/* Worry about encryption key */

		if (dbb->dbb_decrypt) 
			if (!dbb->dbb_filename.IsEmpty() && (dbb->dbb_encrypt_key || options.dpb_key)) 
				{
				if ((dbb->dbb_encrypt_key && !options.dpb_key) ||
					 (!dbb->dbb_encrypt_key && options.dpb_key) ||
					 strcmp(options.dpb_key, reinterpret_cast<char*>(dbb->dbb_encrypt_key->str_data)))
					ERR_post(isc_no_priv,
							isc_arg_string, "encryption",
							isc_arg_string, "database",
							isc_arg_string,  orgName, 
							0);
				}
			else if (options.dpb_key) 
				dbb->dbb_encrypt_key = copy_string(options.dpb_key, strlen(options.dpb_key));

		attachment = dbb->createAttachment();
		threadData.setAttachment (attachment);
		dbb->dbb_flags &= ~DBB_being_opened;

		attachment->att_charset = options.dpb_interp;

		const time_t clock = time(NULL);
		const tm times = *localtime(&clock);
		isc_encode_timestamp(&times, &attachment->att_timestamp);

		if (options.dpb_lc_messages) 
			attachment->att_lc_messages = copy_string(options.dpb_lc_messages, strlen(options.dpb_lc_messages));

		if (options.dpb_no_garbage)
			attachment->att_flags |= ATT_no_cleanup;

		if (options.dpb_gbak_attach)
			attachment->att_flags |= ATT_gbak_attachment;

		if (options.dpb_gstat_attach)
			attachment->att_flags |= ATT_gstat_attachment;

		if (options.dpb_gfix_attach)
			attachment->att_flags |= ATT_gfix_attachment;

		if (options.dpb_working_directory)
			attachment->att_working_directory = options.dpb_working_directory;

		/* If we're a not a secondary attachment, initialize some stuff */

		bool first = false;
		LCK_init(threadData, LCK_OWNER_attachment);	/* For the attachment */
		attachment->att_flags |= ATT_lck_init_done;

		if (!dbb->dbb_file)
			{
			first = true;

			/* Extra LCK_init() done to keep the lock table until the
			   database is shutdown() after the last detach. */
			   
			LCK_init(threadData, LCK_OWNER_database);
			dbb->dbb_flags |= DBB_lck_init_done;
			INI_init(threadData);
			FUN_init();
			dbb->dbb_file = PIO_open(threadData, expandedName, options.dpb_trace, orgName, dbb->fileShared);
			SHUT_init(threadData, dbb);
			PAG_header(threadData, expandedName);
			INI_init2(threadData);
			PAG_init(threadData);
			
			if (options.dpb_set_page_buffers)
				dbb->dbb_page_buffers = options.dpb_page_buffers;

			dbb->pageCache->initialize(threadData, options.dpb_buffers);
			
			// Initialize backup difference subsystem. This must be done before WAL and shadowing
			// is enabled because nbackup it is a lower level subsystem

			dbb->backup_manager = FB_NEW(*dbb->dbb_permanent) BackupManager(threadData, dbb, nbak_state_unknown);
			PAG_init2(threadData, 0);		

			// AB: The bitmap isn't used, but i leave it at the place
			PageBitmap* sbm_recovery = NULL;

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
			LOG_init(expanded_name, length_expanded);
#endif

			/* initialize shadowing as soon as the database is ready for it
			   but before any real work is done */

			SDW_init(threadData, options.dpb_activate_shadow, options.dpb_delete_shadow, sbm_recovery);

			/* dimitr: disabled due to unreliable behaviour of minor ODS upgrades
				a) in the case of any failure it's impossible to attach the database
				b) there's no way to handle failures properly, because upgrade is
				being made in the context of system transaction which doesn't use
				the backout logic
			INI_update_database();
			*/
			}

		/* Attachments to a ReadOnly database need NOT do garbage collection */
    
		if (dbb->dbb_flags & DBB_read_only)
			attachment->att_flags |= ATT_no_cleanup;

		if (options.dpb_buffers && !dbb->dbb_page_buffers)
			CCH_EXPAND(threadData.threadData, (ULONG) options.dpb_buffers);

		if (!options.dpb_verify && CCH_EXCLUSIVE(threadData.threadData, LCK_PW, LCK_NO_WAIT))
			TRA_cleanup(threadData);

		if (invalid_client_SQL_dialect)
			ERR_post(isc_inv_client_dialect_specified, isc_arg_number,
					options.dpb_sql_dialect,
					isc_arg_gds, isc_valid_client_dialects,
					isc_arg_string, "1, or 3", 0);
			
		invalid_client_SQL_dialect = FALSE;

		if (options.dpb_role_name)
			{
			switch (options.dpb_sql_dialect)
				{
				case 0:
					/*
					** V6 Client --> V6 Server, dummy client SQL dialect 0 was passed
					**   It means that client SQL dialect was not set by user
					**   and takes DB SQL dialect as client SQL dialect
					*/
					if (ENCODE_ODS(dbb->dbb_ods_version, dbb->dbb_minor_original) >= ODS_10_0)
						{
						if (dbb->dbb_flags & DBB_DB_SQL_dialect_3) 
							// DB created in IB V6.0 by client SQL dialect 3
							options.dpb_sql_dialect = SQL_DIALECT_V6;
						else 
							// old DB was gbaked in IB V6.0
							options.dpb_sql_dialect = SQL_DIALECT_V5;
						}
					else
						options.dpb_sql_dialect = SQL_DIALECT_V5;
					break;
					
				// V5 Client --> V6 Server, old client has no concept of dialect
				
				case 99:
					options.dpb_sql_dialect = SQL_DIALECT_V5;
					break;
					
				default:
					// V6 Client --> V6 Server, but client SQL dialect was set
					break;
				}

		TEXT local_role_name[BUFFER_LENGTH128];

		switch (options.dpb_sql_dialect)
			{
			case SQL_DIALECT_V5:
				{
				strip_quotes(options.dpb_role_name, local_role_name);
				const size_t len = strlen(local_role_name);
				UCHAR* p1 = reinterpret_cast<UCHAR*>(options.dpb_role_name);
				
				for (size_t cnt = 0; cnt < len; cnt++) 
					*p1++ = UPPER7(local_role_name[cnt]);
					
				*p1 = '\0';
				}
				break;
				
			case SQL_DIALECT_V6_TRANSITION:
			case SQL_DIALECT_V6:
				{
				if (*options.dpb_role_name == DBL_QUOTE || *options.dpb_role_name == SINGLE_QUOTE)
					{
					const UCHAR* p1 = reinterpret_cast<UCHAR*>(options.dpb_role_name);
					UCHAR* p2 = reinterpret_cast<UCHAR*>(local_role_name);
					int cnt = 1;
					bool delimited_done = false;
					const TEXT	end_quote = *p1;
					++p1;
					
					/* remove the delimited quotes and escape quote
					   from ROLE name */
					   
					while (*p1 && !delimited_done && cnt < BUFFER_LENGTH128 - 1)
						{
						if (*p1 == end_quote)
							{
							cnt++;
							*p2++ = *p1++;
							
							if (*p1 && *p1 == end_quote && cnt < BUFFER_LENGTH128 - 1)
								p1++;	/* skip the escape quote here */
							else
								{
								delimited_done = true;
								p2--;
								}
							}
						else
							{
							cnt++;
							*p2++ = *p1++;
							}
						}
						
					*p2 = '\0';
					strcpy(options.dpb_role_name, local_role_name);
					}
				else
					{
					strcpy(local_role_name, options.dpb_role_name);
					const size_t len = strlen(local_role_name);
					UCHAR* p1 = reinterpret_cast<UCHAR*>(options.dpb_role_name);
					
					for (size_t cnt = 0; cnt < len; cnt++)
						*p1++ = UPPER7(local_role_name[cnt]);
						
					*p1 = '\0';
					}
				}
				break;
				
			default:
				break;
				}
			}

	options.dpb_sql_dialect = 0;
	
	if (dbb->securityDatabase == translatedName)
		ERR_post(isc_login, 0);

	attachment->authenticateUser(threadData, dpb_length, dpb);
	SCL_init(false, attachment, dbb->securityDatabase, threadData);
			/***
			options.dpb_sys_user_name,
			options.dpb_user_name,
			options.dpb_password,
			options.dpb_password_enc,
			options.dpb_role_name,
			securityDatabase,
			threadData,
			internal);
			***/

#ifdef SHARED_CACHE
	if (sync.state == Exclusive)
		sync.downGrade(Shared);
#endif
		
	if (options.dpb_shutdown || options.dpb_online)
		{
		/* By releasing the DBB_MUTX_init_fini mutex here, we would be allowing
		   other threads to proceed with their detachments, so that shutdown does
		   not timeout for exclusive access and other threads don't have to wait
		   behind shutdown */

#ifdef SHARED_CACHE
		sync.unlock();
#endif
		
		if (!SHUT_database(dbb, options.dpb_shutdown, options.dpb_shutdown_delay)) 
			{
			if (user_status[1] != FB_SUCCESS)
				ERR_punt();

			ERR_post(isc_no_priv,
						isc_arg_string, "shutdown or online",
						isc_arg_string, "database",
						isc_arg_string, (const char*) expandedName,
                        0);
			}
		
#ifdef SHARED_CACHE
		sync.lock(Shared);
#endif
		}

#ifdef SUPERSERVER
	/* Check if another attachment has or is requesting exclusive database access.
	   If this is an implicit attachment for the security (password) database, don't
	   try to get exclusive attachment to avoid a deadlock condition which happens
	   when a client tries to connect to the security database itself. */

	if (!options.dpb_sec_attach) 
		{
		CCH_EXCLUSIVE_ATTACHMENT(threadData.threadData, LCK_none, LCK_WAIT);

		if (attachment->att_flags & ATT_shutdown)
			ERR_post(isc_shutdown, isc_arg_string, (const char*) expandedName, 0);
		}
#endif

	/* If database is shutdown then kick 'em out. */

	if (dbb->dbb_ast_flags & (DBB_shut_attach | DBB_shut_tran))
		ERR_post(isc_shutinprog, isc_arg_string, (const char*) expandedName, 0);

	if (dbb->dbb_ast_flags & DBB_shutdown &&
		!(attachment->userFlags & (USR_locksmith | USR_owner)))
		ERR_post(isc_shutdown, isc_arg_string, (const char*) expandedName, 0);

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	if (options.dpb_quit_log) 
		LOG_disable();
#endif

	/* Figure out what character set & collation this attachment prefers */

	find_intl_charset(threadData, attachment, &options);

	if (options.dpb_verify)
		{
		if (!CCH_EXCLUSIVE(threadData.threadData, LCK_PW, WAIT_PERIOD))
			ERR_post(isc_bad_dpb_content, isc_arg_gds, isc_cant_validate, 0);

#ifdef GARBAGE_THREAD
		/* Can't allow garbage collection during database validation. */

		VIO_fini(threadData);
#endif
		
		if (!VAL_validate(threadData, options.dpb_verify)) 
			ERR_punt();
		}

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
		if (options.dpb_log) 
			{
			if (first) 
				{
				if (!CCH_EXCLUSIVE(threadData.threadData, LCK_EX, WAIT_PERIOD))
					ERR_post(isc_lock_timeout, isc_arg_gds, isc_obj_in_use,
							isc_arg_string, 
							ERR_string(file_name, file_length),
							0);
				LOG_enable(options.dpb_log, strlen(options.dpb_log));
				}
			else 
				ERR_post(isc_bad_dpb_content, isc_arg_gds, isc_cant_start_logging, 0);
			}
#endif

		if (options.dpb_set_db_sql_dialect)
			PAG_set_db_SQL_dialect(threadData, dbb, options.dpb_set_db_sql_dialect);

		if (options.dpb_sweep_interval != -1) 
			{
			PAG_sweep_interval(threadData, options.dpb_sweep_interval);
			dbb->dbb_sweep_interval = options.dpb_sweep_interval;
			}

		if (options.dpb_set_force_write)
			PAG_set_force_write(threadData, dbb, options.dpb_force_write);

		if (options.dpb_set_no_reserve)
			PAG_set_no_reserve(threadData, dbb, options.dpb_no_reserve);

		if (options.dpb_set_page_buffers) 
			PAG_set_page_buffers(threadData, options.dpb_page_buffers);

		if (options.dpb_set_db_readonly) 
			{
			if (!CCH_EXCLUSIVE(threadData.threadData, LCK_EX, WAIT_PERIOD)) 
				ERR_post(isc_lock_timeout, isc_arg_gds, isc_obj_in_use,
						isc_arg_string, (const char*) expandedName,
						0); 
			PAG_set_db_readonly(threadData, dbb, options.dpb_db_readonly);
			}

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
		/* don't record the attach until now in case the log is added during the attach */

		LOG_call(log_attach2, file_length, file_name, *handle, dpb_length, dpb, expanded_filename);
#endif

#ifdef GARBAGE_THREAD
		VIO_init(threadData);
#endif

		CCH_RELEASE_EXCLUSIVE(threadData.threadData);

#ifdef GOVERNOR
		if (!options.dpb_sec_attach) 
			{
			if (JRD_max_users) 
				{
				if (num_attached < (JRD_max_users * ATTACHMENTS_PER_USER)) 
					num_attached++;
				else 
					ERR_post(isc_max_att_exceeded, 0);
				}
			}
		else 
			attachment->att_flags |= ATT_security_db;
#endif /* GOVERNOR */

		/* if there was an error, the status vector is all set */

		if (options.dpb_sweep & isc_dpb_records)
			{
#ifdef SHARED_CACHE
			sync.unlock();
#endif

			if (!(TRA_sweep(threadData, 0)))
				ERR_punt();
			}

		if (options.dpb_dbkey_scope) 
			attachment->att_dbkey_trans = TRA_start(threadData, attachment, 0, 0);
		
		// Recover database after crash during backup difference file merge
		
		dbb->backup_manager->end_backup(threadData, true/*do recovery*/); 
		
		*handle = attachment;	

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
		LOG_call(log_handle_returned, *handle);
#endif

		unlockAST(dbb);

		return return_success(threadData);
		}
	catch (OSRIException& exception)
		{
		ISC_STATUS_ARRAY temp_status;
		try
			{
#ifdef SHARED_CACHE
			if (sync.state > None)
				sync.unlock();
#endif
			threadData.setStatusVector (temp_status);
			dbb->dbb_flags &= ~DBB_being_opened;
			release_attachment(threadData, attachment);

			/* At this point, mutex dbb->dbb_mutexes [DBB_MUTX_init_fini] has been
			   unlocked and mutex databases_mutex has been locked. */
			   
			unlockAST(dbb);

			if (!dbb->dbb_attachments)
				shutdown_database(threadData, true);
			else if (attachment)
				delete attachment;
				
			}
		catch (OSRIException& secondary)
			{ 
			secondary; 
			}
			
		threadData.setStatusVector (user_status);
		
		return error(&exception, user_status);
		}
}


ISC_STATUS GDS_BLOB_INFO(ISC_STATUS*	user_status,
						blb**	blob_handle,
						SSHORT	item_length,
						const UCHAR*	items,
						SSHORT	buffer_length,
						UCHAR*	buffer)
{
/**************************************
 *
 *	g d s _ $ b l o b _ i n f o
 *
 **************************************
 *
 * Functional description
 *	Provide information on blob object.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);
	const blb* blob = check_blob(threadData, user_status, blob_handle);
	if (!blob) {
		return user_status[1];
	}

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_blob_info2, *blob_handle, item_length, items, buffer_length);
#endif

	try {
		////tdbb->tdbb_status_vector = user_status;

		INF_blob_info(blob, items, item_length, buffer, buffer_length);
	}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}

	return return_success(threadData);
}


ISC_STATUS GDS_CANCEL_BLOB(ISC_STATUS * user_status, blb** blob_handle)
{
/**************************************
 *
 *	g d s _ $ c a n c e l _ b l o b
 *
 **************************************
 *
 * Functional description
 *	Abort a partially completed blob.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	if (*blob_handle) {
		blb* blob = check_blob(threadData, user_status, blob_handle);
		if (!blob)
			return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
		LOG_call(log_cancel_blob, *blob_handle);
#endif

		try {
			////tdbb->tdbb_status_vector = user_status;

			BLB_cancel(threadData, blob);
			*blob_handle = NULL;
		}
		catch (OSRIException& exception)
			{
			return error(&exception, user_status);
			}
	}

	return return_success(threadData);
}


ISC_STATUS GDS_CANCEL_EVENTS(ISC_STATUS* user_status,
							Attachment** handle,
							SLONG* id)
{
/**************************************
 *
 *	g d s _ $ c a n c e l _ e v e n t s
 *
 **************************************
 *
 * Functional description
 *	Cancel an outstanding event.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	if (check_database(threadData, *handle)) 
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_cancel_events, *handle, *id);
#endif

	try
		{
		////tdbb->tdbb_status_vector = user_status;

		EVENT_cancel(*id);
		}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}

	return return_success(threadData);
}


#ifdef CANCEL_OPERATION
ISC_STATUS GDS_CANCEL_OPERATION(ISC_STATUS* user_status,
								Attachment** handle,
								USHORT option)
{
/**************************************
 *
 *	g d s _ $ c a n c e l _ o p e r a t i o n
 *
 **************************************
 *
 * Functional description
 *	Try to cancel an operation.
 *
 **************************************/
	api_entry_point_init(user_status);

	Attachment* attachment = *handle;

	/* Check out the database handle.  This is mostly code from
	   the routine "check_database" */

	DBB dbb;
	
	if (!attachment ||!(dbb = attachment->att_database))
		return handle_error(user_status, isc_bad_db_handle, 0);

	/* Make sure this is a valid attachment */

	const Attachment* attach;
	
	lockAST(dbb);

	for (attach = dbb->dbb_attachments; attach; attach = attach->att_next)
		if (attach == attachment)
			break;

	if (!attach)
		return handle_error(user_status, isc_bad_db_handle, 0);

	switch (option) 
		{
		case CANCEL_disable:
			attachment->att_flags |= ATT_cancel_disable;
			break;

		case CANCEL_enable:
			attachment->att_flags &= ~ATT_cancel_disable;
			break;

		case CANCEL_raise:
			attachment->att_flags |= ATT_cancel_raise;
			break;

		default:
			fb_assert(FALSE);
		}

	unlockAST(dbb);

	return FB_SUCCESS;
}
#endif


ISC_STATUS GDS_CLOSE_BLOB(ISC_STATUS * user_status, blb* * blob_handle)
{
/**************************************
 *
 *	g d s _ $ c l o s e _ b l o b
 *
 **************************************
 *
 * Functional description
 *	Abort a partially completed blob.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	blb* blob = check_blob(threadData, user_status, blob_handle);
	if (!blob)
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_close_blob, *blob_handle);
#endif

	try
	{
		////tdbb->tdbb_status_vector = user_status;

		BLB_close(threadData, blob);
		*blob_handle = NULL;
	}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}


	return return_success(threadData);
}


ISC_STATUS GDS_COMMIT(ISC_STATUS * user_status, Transaction* * tra_handle)
{
/**************************************
 *
 *	g d s _ $ c o m m i t
 *
 **************************************
 *
 * Functional description
 *	Commit a transaction.
 *
 **************************************/

	api_entry_point_init(user_status);
	Database *dbb = (*tra_handle)->tra_attachment->att_database;
	lockAST(dbb);

	if (commit(user_status, tra_handle, false))
		return user_status[1];

	unlockAST(dbb);
	*tra_handle = NULL;

	return FB_SUCCESS;
}


ISC_STATUS GDS_COMMIT_RETAINING(ISC_STATUS * user_status, Transaction* * tra_handle)
{
/**************************************
 *
 *	g d s _ $ c o m m i t _ r e t a i n i n g
 *
 **************************************
 *
 * Functional description
 *	Commit a transaction.
 *
 **************************************/

	api_entry_point_init(user_status);

	return commit(user_status, tra_handle, true);
}

ISC_STATUS jrd8_compile_request(ISC_STATUS*, Attachment**,
											  Request**,
											  SSHORT, const UCHAR*);

ISC_STATUS GDS_COMPILE(ISC_STATUS* user_status,
						Attachment **db_handle,
						Request** req_handle,
						SSHORT blr_length,
						const UCHAR* blr)
{
/**************************************
 *
 *	g d s _ $ c o m p i l e
 *
 **************************************
 *
 * Functional description
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	NULL_CHECK(req_handle, isc_bad_req_handle);
	Attachment* attachment = *db_handle;

	if (check_database(threadData, attachment))
		return user_status[1];

	lockAST(attachment->att_database);

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_compile, *db_handle, *req_handle, blr_length, blr);
#endif

	try
		{
#ifdef SHARED_CACHE
		Sync sync(&attachment->syncRequests, "jrd8_compile_request");
		sync.lock(Exclusive);
#endif
		Request* request = CMP_compile2(threadData, blr, FALSE);
		request->req_attachment = attachment;
		request->req_request = attachment->att_requests;
		attachment->att_requests = request;
	
		DEBUG;
		*req_handle = request;
#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
		LOG_call(log_handle_returned, *req_handle);
#endif
		}
	catch (OSRIException& exception)
		{
		unlockAST(attachment->att_database);
		return error(&exception, user_status);
		}

	unlockAST(attachment->att_database);
	return return_success(threadData);
}


ISC_STATUS GDS_CREATE_BLOB2(ISC_STATUS* user_status,
							Attachment** db_handle,
							Transaction** tra_handle,
							blb** blob_handle,
							bid* blob_id,
							USHORT bpb_length,
							const UCHAR* bpb)
{
/**************************************
 *
 *	g d s _ $ c r e a t e _ b l o b
 *
 **************************************
 *
 * Functional description
 *	Open an existing blob.
 *
 **************************************/
	Transaction* transaction;
	blb* blob;
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	NULL_CHECK(blob_handle, isc_bad_segstr_handle);

	if (check_database(threadData, *db_handle))
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_create_blob2, *db_handle, *tra_handle, *blob_handle, blob_id,
			 bpb_length, bpb);
#endif

	try
		{
		transaction = find_transaction(threadData, *tra_handle, isc_segstr_wrong_db);
#ifdef SHARED_CACHE
		Sync syncTransaction(&transaction->syncInUse, "jrd8_create_blob");
		syncTransaction.lock(Exclusive);
#endif
		blob = BLB_create2(threadData, transaction, blob_id, bpb_length, bpb);
		*blob_handle = blob;
	
#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
		LOG_call(log_handle_returned, *blob_handle);
		LOG_call(log_handle_returned, blob_id->bid_stuff.bid_blob);
#endif
	}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}


	return return_success(threadData);
}


ISC_STATUS GDS_CREATE_DATABASE(ISC_STATUS*	user_status,
								const TEXT *orgName,
								const TEXT *translatedName,
								Attachment**	handle,
								USHORT	dpb_length,
								const UCHAR*	dpb,
								USHORT	db_type,
								ConfObject* databaseConfiguration,
								ConfObject* providerConfiguration)
{
/**************************************
 *
 *	g d s _ $ c r e a t e _ d a t a b a s e
 *
 **************************************
 *
 * Functional description
 *	Create a nice, squeeky clean database, uncorrupted by user data.
 *
 **************************************/
	ISC_STATUS *status;

	api_entry_point_init(user_status);

	if (*handle)
		return handle_error(user_status, isc_bad_db_handle, 0);

	JString expandedName = PathName::expandFilename (translatedName, dpb_length, dpb);
	ThreadData threadData (user_status);

	if (databaseConfiguration)
		databaseConfiguration->setChain (providerConfiguration);
		
	DBB dbb = init(threadData, user_status, expandedName, false, databaseConfiguration);
	
	if (!dbb) 
		{
		JRD_restore_context();
		return user_status[1];
		}

	lockAST(dbb);

	/* Handoff database existence lock to local Sync object */
	
#ifdef SHARED_CACHE
	Sync sync(&dbb->syncExistence, "jrd8_create_database");
	sync.lock(Exclusive);
	dbb->syncExistence.unlock();
#endif
	
	threadData.setDatabase (dbb);
	dbb->dbb_flags |= DBB_being_opened;

	/* Initialize error handling */

	status = user_status;
	Attachment* attachment = NULL;

	/* Count active thread in database */

	dbb->incrementUseCount();
	
	try 
		{
		/* Process database parameter block */
		TEXT opt_buffer[DPB_EXPAND_BUFFER];
		TEXT* opt_ptr = opt_buffer;
		DPB options;
		get_options(threadData, dpb, dpb_length, &opt_ptr, DPB_EXPAND_BUFFER, &options);
		
		if (invalid_client_SQL_dialect == FALSE && options.dpb_sql_dialect == 99)
			options.dpb_sql_dialect = 0;

#ifndef NO_NFS
		/* Don't check nfs if single user */

		if (!options.dpb_single_user)
#endif
			if (ISC_check_if_remote(expandedName, true)) 
				ERR_post(isc_unavailable, 0);

		if (options.dpb_key) 
			dbb->dbb_encrypt_key = copy_string(options.dpb_key, strlen(options.dpb_key));

		attachment = dbb->createAttachment();
		threadData.setAttachment (attachment);
		
		dbb->dbb_flags &= ~DBB_being_opened;

		if (options.dpb_working_directory) 
			attachment->att_working_directory = options.dpb_working_directory;

		if (options.dpb_gbak_attach)
			attachment->att_flags |= ATT_gbak_attachment;

		switch (options.dpb_sql_dialect) 
			{
			case 0:
				/* This can be issued by QLI, GDEF and old BDE clients.  In this case
				* assume dialect 1 */
				options.dpb_sql_dialect = SQL_DIALECT_V5;
			case SQL_DIALECT_V5:
				break;
				
			case SQL_DIALECT_V6:
				dbb->dbb_flags |= DBB_DB_SQL_dialect_3;
				break;
				
			default:
				ERR_post(isc_database_create_failed, 
						isc_arg_string, (const char*) expandedName, 
						isc_arg_gds, isc_inv_dialect_specified,
						isc_arg_number, options.dpb_sql_dialect, 
						isc_arg_gds, isc_valid_db_dialects, 
						isc_arg_string, "1 and 3", 
						0);
				break;
			}

		attachment->att_charset = options.dpb_interp;

		const time_t clock = time(NULL);
		const tm times = *localtime(&clock);
		isc_encode_timestamp(&times, &attachment->att_timestamp);

		if (options.dpb_lc_messages)
			attachment->att_lc_messages =
				copy_string(options.dpb_lc_messages, strlen(options.dpb_lc_messages));

		if (!options.dpb_page_size) 
			options.dpb_page_size = DEFAULT_PAGE_SIZE;

		USHORT page_size;
		
		for (page_size = MIN_PAGE_SIZE; page_size < MAX_PAGE_SIZE; page_size <<= 1)
			if (options.dpb_page_size < page_size << 1)
				break;

		dbb->dbb_page_size =
			(page_size > MAX_PAGE_SIZE) ? MAX_PAGE_SIZE : page_size;

		LCK_init(threadData, LCK_OWNER_attachment);	/* For the attachment */
		attachment->att_flags |= ATT_lck_init_done;
		
		/* Extra LCK_init() done to keep the lock table until the
		   database is shutdown() after the last detach. */
		   
		LCK_init(threadData, LCK_OWNER_database);
		dbb->dbb_flags |= DBB_lck_init_done;
		INI_init(threadData);
		FUN_init();
		PAG_init(threadData);
		
		if (dbb->securityDatabase == translatedName)
			ERR_post(isc_login, 0);
			
		if (dbb->securityDatabase != "self")
			attachment->authenticateUser(threadData, dpb_length, dpb);
			
		SCL_init(true, attachment, dbb->securityDatabase, threadData);
		
		if (!verify_database_name(expandedName, user_status)) 
			{
			JRD_restore_context();
			return user_status[1];
			}
			
		dbb->dbb_file = PIO_create(threadData, expandedName, expandedName.length(), options.dpb_overwrite, dbb->fileShared);
		const File* first_dbb_file = dbb->dbb_file;
		
		if (options.dpb_set_page_buffers)
			dbb->dbb_page_buffers = options.dpb_page_buffers;
			
		dbb->pageCache->initialize(threadData, options.dpb_buffers);
		
		
		// Initialize backup difference subsystem. This must be done before WAL and shadowing
		// is enabled because nbackup it is a lower level subsystem
		
		dbb->backup_manager = FB_NEW(*dbb->dbb_permanent) BackupManager(threadData, dbb, nbak_state_normal); 
		
		PAG_format_header(threadData);
		INI_init2(threadData);
		PAG_format_log(threadData);
		PAG_format_pip(threadData);

		if (options.dpb_set_page_buffers)
			PAG_set_page_buffers(threadData, options.dpb_page_buffers);

		if (options.dpb_set_no_reserve)
			PAG_set_no_reserve(threadData, dbb, options.dpb_no_reserve);

		INI_format(threadData, attachment->userData.userName, options.dpb_set_db_charset);

		if (options.dpb_shutdown || options.dpb_online) 
			{
			/* By releasing the DBB_MUTX_init_fini mutex here, we would be allowing
			other threads to proceed with their detachments, so that shutdown does
			not timeout for exclusive access and other threads don't have to wait
			behind shutdown */

			if (!SHUT_database(dbb, options.dpb_shutdown, options.dpb_shutdown_delay)) 
				ERR_post(isc_no_priv,
						isc_arg_string, "shutdown or online",
						isc_arg_string, "database",
						isc_arg_string, (const char*) expandedName,
						0);
			}

		if (options.dpb_sweep_interval != -1) 
			{
			PAG_sweep_interval(threadData, options.dpb_sweep_interval);
			dbb->dbb_sweep_interval = options.dpb_sweep_interval;
			}

		if (options.dpb_set_force_write)
			PAG_set_force_write(threadData, dbb, options.dpb_force_write);

		/* initialize shadowing semaphore as soon as the database is ready for it
		   but before any real work is done */

		SDW_init(threadData, options.dpb_activate_shadow, options.dpb_delete_shadow, NULL);

#ifdef GARBAGE_THREAD
		VIO_init(threadData);
#endif

		if (options.dpb_set_db_readonly) 
			{
			if (!CCH_EXCLUSIVE (threadData.threadData, LCK_EX, WAIT_PERIOD))
				ERR_post (isc_lock_timeout, isc_arg_gds, isc_obj_in_use,
						isc_arg_string, (const char*) expandedName,
						0);
			PAG_set_db_readonly (threadData, dbb, options.dpb_db_readonly);
			}

		CCH_RELEASE_EXCLUSIVE(threadData.threadData);

		/* Figure out what character set & collation this attachment prefers */

		find_intl_charset(threadData, attachment, &options);

#ifdef WIN_NT
		dbb->dbb_filename = JString (first_dbb_file->fil_string, first_dbb_file->fil_length);
#else
		dbb->dbb_filename = expandedName;
#endif

#ifdef GOVERNOR
		if (!options.dpb_sec_attach) 
			{
			if (JRD_max_users) 
				{
				if (num_attached < (JRD_max_users * ATTACHMENTS_PER_USER))
					num_attached++;
				else
					ERR_post(isc_max_att_exceeded, 0);
				}
			}
		else 
			attachment->att_flags |= ATT_security_db;
#endif /* GOVERNOR */


		*handle = attachment;
		CCH_FLUSH(threadData.threadData, (USHORT) FLUSH_FINI, 0);
		dbb->makeReady();
		unlockAST(dbb);

		/***
		if (dbb->securityDatabase == "self")
			{
			PBGen apb(fb_apb_version1);
			apb.putParameter(fb_apb_operation, fb_apb_create_account);
			apb.putParameter(fb_apb_account, options.dpb_user_name);
			
			if (options.dpb_password[0])
				apb.putParameter(fb_apb_password, options.dpb_password);
			
			dbb->updateAccountInfo(threadData, apb.getLength(), apb.buffer);
			}
		***/
		
		return return_success(threadData);
		}
	catch (OSRIException& exception)
		{
		try
			{
#ifdef SHARED_CACHE
			sync.unlock();
#endif
			dbb->dbb_flags &= ~DBB_being_opened;
			release_attachment(threadData, attachment);

			/* At this point, mutex dbb->dbb_mutexes [DBB_MUTX_init_fini] has been
			   unlocked and mutex databases_mutex has been locked. */
			   
			unlockAST(dbb);
			if (!dbb->dbb_attachments)
				shutdown_database(threadData, true);
			else if (attachment)
				delete attachment;
			}
		catch (OSRIException& secondary)
			{ 
			secondary; 
			}
			
		threadData.setStatusVector (status);
		return error(&exception, user_status);
		}
}


ISC_STATUS GDS_DATABASE_INFO(ISC_STATUS* user_status,
							Attachment** handle,
							SSHORT item_length,
							const UCHAR* items,
							SSHORT buffer_length,
							UCHAR* buffer)
{
/**************************************
 *
 *	g d s _ $ d a t a b a s e _ i n f o
 *
 **************************************
 *
 * Functional description
 *	Provide information on database object.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	if (check_database(threadData, *handle))
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_database_info2, *handle, item_length, items, buffer_length);
#endif

	try
		{
		INF_database_info(threadData, items, item_length, buffer, buffer_length);
		}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}

	return return_success(threadData);
}


ISC_STATUS GDS_DDL(ISC_STATUS* user_status,
					Attachment** db_handle,
					Transaction** tra_handle,
					USHORT ddl_length,
					const UCHAR* ddl)
{
/**************************************
 *
 *	g d s _ $ d d l
 *
 **************************************
 *
 * Functional description
 *
 **************************************/
	api_entry_point_init(user_status);
	ThreadData threadData (user_status);
	Attachment* attachment = *db_handle;
	
	if (check_database(threadData, attachment))
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_ddl, *db_handle, *tra_handle, ddl_length, ddl);
#endif

	Transaction* transaction = 0;

	try 
		{
		lockAST(attachment->att_database);
		transaction = find_transaction(threadData, *tra_handle, isc_segstr_wrong_db);
#ifdef SHARED_CACHE
		Sync syncTransaction(&transaction->syncInUse, "jrd8_ddl");
		syncTransaction.lock(Exclusive);
#endif
		DYN_ddl(threadData, attachment, transaction, ddl_length,ddl);
		}
	catch (OSRIException& exception)
		{
		unlockAST(attachment->att_database);
		return error(&exception, user_status);
		}

	/*
	* Perform an auto commit for autocommit transactions.
	* This is slightly tricky.  If the commit retain works,
	* all is well.  If TRA_commit () fails, we perform
	* a rollback_retain ().  This will backout the
	* effects of the transaction, mark it dead and
	* start a new transaction.

	* For now only ExpressLink will use this feature.  Later
	* a new entry point may be added.
	*/

	if (transaction->tra_flags & TRA_perform_autocommit)
		{
		transaction->tra_flags &= ~TRA_perform_autocommit;

		try 
			{
			TRA_commit(threadData, transaction, true);
			}
		catch (OSRIException& exception) 
			{
			ISC_STATUS_ARRAY temp_status;
			//tdbb->tdbb_status_vector = temp_status;
			threadData.setStatusVector (temp_status);
			try 
				{
				TRA_rollback(threadData, transaction, true);
				}
			catch (OSRIException& exception) 
				{
				exception;
			    // CVC, TMN: Do nothing, see FB1 code, this will fall into
			    // the two lines below, achieving the same logic than going
			    // back to the SETJMP(env) in FB1.
				}
			//tdbb->tdbb_status_vector = user_status;
			threadData.setStatusVector (user_status);
			unlockAST(attachment->att_database);
			return error(&exception, user_status);
			}
		};

	unlockAST(attachment->att_database);
	
	return return_success(threadData);
}


ISC_STATUS GDS_DETACH(ISC_STATUS * user_status, Attachment* * handle)
{
/**************************************
 *
 *	g d s _ $ d e t a c h
 *
 **************************************
 *
 * Functional description
 *	Close down a database.
 *
 **************************************/
	//struct tdbb thd_context;
#ifdef GOVERNOR
	USHORT attachment_flags;
#endif

	api_entry_point_init(user_status);
	ThreadData threadData (user_status);
	Attachment* attachment = *handle;

	/* Check out the database handle.  This is mostly code from
	   the routine "check_database" */

	Database* dbb;
	
	if (!attachment || !(dbb = attachment->att_database) )
		return handle_error(user_status, isc_bad_db_handle, threadData);

	//printf ("%s: detaching\n\n", (const char*) dbb->dbb_filename);
	
	/* Make sure this is a valid attachment */

#ifdef SHARED_CACHE
	Sync sync (&dbb->syncAttachments, "jrd8_detach_database");
	sync.lock(Exclusive);
#endif

	Attachment* attach;
	
	for (attach = dbb->dbb_attachments; attach; attach = attach->att_next)
		if (attach == attachment)
			break;

	if (!attach)
		return handle_error(user_status, isc_bad_db_handle, threadData);

	/* if this is the last attachment, mark dbb as not in use */

	if (dbb->dbb_attachments == attachment && !attachment->att_next &&
		!(dbb->dbb_flags & DBB_being_opened))
		dbb->dbb_flags |= DBB_not_in_use;
		
	lockAST(dbb); /* this will be unlocked in shutdown_blocking_thread */

	threadData.setDatabase (dbb);
	threadData.setAttachment (attachment);

	/* Count active thread in database */

	dbb->incrementUseCount();

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_detach, *handle);
	LOG_call(log_statistics, dbb->dbb_reads, dbb->dbb_writes,
			 dbb->dbb_max_memory);
#endif

	/* purge_attachment below can do an ERR_post */

	//tdbb->tdbb_status_vector = user_status;

	try 
		{
		// Purge attachment, don't rollback open transactions 


#ifdef CANCEL_OPERATION
		attachment->att_flags |= ATT_cancel_disable;
#endif

#ifdef GOVERNOR
		attachment_flags = (USHORT) attachment->att_flags;
#endif

#ifdef SHARED_CACHE
		sync.unlock();
#endif
		purge_attachment(threadData, user_status, attachment, false);

#ifdef GOVERNOR
	if (JRD_max_users) 
		if (!(attachment_flags & ATT_security_db)) 
			{
			fb_assert(num_attached > 0);
			num_attached--;
			}
#endif /* GOVERNOR */

		*handle = NULL;

		return return_success(threadData);
		}
	catch (OSRIException& exception) 
		{
		dbb->dbb_flags &= ~DBB_not_in_use;
		return error(&exception, user_status);
		}
}


ISC_STATUS GDS_DROP_DATABASE(ISC_STATUS * user_status, Attachment* * handle)
{
/**************************************
 *
 *	i s c _ d r o p _ d a t a b a s e
 *
 **************************************
 *
 * Functional description
 *	Close down and purge a database.
 *
 **************************************/

	api_entry_point_init(user_status);
	ThreadData threadData (user_status);
	Attachment* attachment = *handle;

	/* Check out the database handle.  This is mostly code from
	   the routine "check_database" */

	DBB dbb;
	
	if (!attachment || !(dbb = attachment->att_database))
		return handle_error(user_status, isc_bad_db_handle, threadData);

	/* Make sure this is a valid attachment */

	Attachment* attach;
	
	for (attach = dbb->dbb_attachments; attach; attach = attach->att_next)
		if (attach == attachment)
			break;

	if (!attach)
		return handle_error(user_status, isc_bad_db_handle, threadData);

	threadData.setDatabase (dbb);
	threadData.setAttachment (attachment);
	threadData.setPool (dbb->dbb_permanent);

	/* Count active thread in database */

	dbb->incrementUseCount();

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_drop_database, *handle);
	LOG_call(log_statistics, dbb->dbb_reads, dbb->dbb_writes,
			 dbb->dbb_max_memory);
#endif

#ifdef SHARED_CACHE
	Sync sync(&dbb->syncExistence, "jrd8_drop_database");
#endif
	
	try
		{
		if (!(attachment->userFlags & (USR_locksmith | USR_owner)))
			ERR_post(isc_no_priv,
					isc_arg_string, "drop",
					isc_arg_string, "database",
					isc_arg_string,
					ERR_cstring(attachment->att_filename), 0);

		if (attachment->att_flags & ATT_shutdown)
			ERR_post(isc_shutdown, isc_arg_string,
					ERR_cstring(attachment->att_filename), 0);

#ifdef SHARED_CACHE
		sync.lock(Exclusive);
#endif
		
		if (!CCH_EXCLUSIVE(threadData.threadData, LCK_PW, WAIT_PERIOD))
			ERR_post(isc_lock_timeout, isc_arg_gds, isc_obj_in_use,
					isc_arg_string,
					ERR_cstring(attachment->att_filename), 0);
		}
	catch(OSRIException& exception)
		{
		return error(&exception, user_status);
		}
	
	try 
		{
		/* Check if same process has more attachments */
		
		for (Attachment *att = dbb->dbb_attachments; att; att = att->att_next)
			if (att != attach && !(att->att_flags & ATT_internal))
				ERR_post(isc_no_meta_update, isc_arg_gds, isc_obj_in_use,
						isc_arg_string, "DATABASE", 0);

#ifdef CANCEL_OPERATION
		attachment->att_flags |= ATT_cancel_disable;
#endif

		/* Here we have database locked in exclusive mode.
		   Just mark the header page with an 0 ods version so that no other
		   process can attach to this database once we release our exclusive
		   lock and start dropping files. */

   		WIN window(HEADER_PAGE);
		header_page* header = (header_page*) CCH_FETCH(threadData.threadData, &window, LCK_write, pag_header);
		CCH_MARK_MUST_WRITE(threadData.threadData, &window);
		header->hdr_ods_version = 0;
		CCH_RELEASE(threadData.threadData, &window);
		}
	catch (OSRIException& exception) 
		{
		return error(&exception, user_status);
		}

    bool err = false; // so much for uninitialized vars... if something
    // failed before the first call to drop_files, which was the value?
    
	try 
		{
		// This point on database is useless; mark the dbb unusable
		dbb->dbb_flags |= DBB_not_in_use;
		*handle = NULL;

		const File* file = dbb->dbb_file;
		const Shadow* shadow = dbb->dbb_shadow;

#ifdef GOVERNOR
		if (JRD_max_users) 
			{
			fb_assert(num_attached > 0);
			num_attached--;
			}
#endif /* GOVERNOR */

		/* Unlink attachment from database */

		release_attachment(threadData, attachment);
		delete attachment;
		shutdown_database(threadData, false);

		/* drop the files here. */

		err = drop_files(file);
		
		for (; shadow; shadow = shadow->sdw_next) 
			err = err || drop_files(shadow->sdw_file);

		}
	catch (OSRIException& exception) 
		{
		return error(&exception, user_status);
		}
	
#ifdef SHARED_CACHE
	sync.unlock();	
#endif
	//delete dbb;
	dbb->release();
		
	if (err) 
		{
		user_status[0] = isc_arg_gds;
		user_status[1] = isc_drdb_completed_with_errs;
		user_status[2] = isc_arg_end;
		return user_status[1];
		}

	//return return_success(threadData);
	return user_status[1];
}


ISC_STATUS GDS_ENGINE_INFO(ISC_STATUS* userStatus, const TEXT* engineName,
						   int itemsLength, const UCHAR* items, 
						   int bufferLength, UCHAR* buffer)
{
/**************************************
 *
 *	g d s _ $ e n g i n e _ i n f o
 *
 **************************************
 *
 * Functional description
 *	Retrieve engine information as requested
 *	by info items.
 *
 **************************************/
 
	EngineInfo engineInfo(&databaseManager);

	return engineInfo.getInfo(userStatus, itemsLength, items, bufferLength, buffer);
}


ISC_STATUS GDS_GET_SEGMENT(ISC_STATUS * user_status,
							blb* * blob_handle,
							int * length,
							USHORT buffer_length,
							UCHAR * buffer)
{
/**************************************
 *
 *	g d s _ $ g e t _ s e g m e n t
 *
 **************************************
 *
 * Functional description
 *	Abort a partially completed blob.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	blb* blob = check_blob(threadData, user_status, blob_handle);
	if (!blob)
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_get_segment2, *blob_handle, length, buffer_length);
#endif

	try
		{
		//tdbb->tdbb_status_vector = user_status;

		*length = BLB_get_segment(threadData, blob, buffer, buffer_length);
		//tdbb->tdbb_status_vector[0] = isc_arg_gds;
		//tdbb->tdbb_status_vector[2] = isc_arg_end;
		DBB dbb = threadData.threadData->tdbb_database;
	
		if (blob->blb_flags & BLB_eof) 
			{
			JRD_restore_context();
			dbb->decrementUseCount();
			return (user_status[1] = isc_segstr_eof);
			}
		else if (!(blob->blb_flags & BLB_stream) && blob->blb_fragment_size) 
			{
			JRD_restore_context();
			dbb->decrementUseCount();
			return (user_status[1] = isc_segment);
			}
		}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}

	return return_success(threadData);
}


ISC_STATUS GDS_GET_SLICE(ISC_STATUS* user_status,
						Attachment** db_handle,
						Transaction** tra_handle,
						SLONG* array_id,
						USHORT sdl_length,
						const UCHAR* sdl,
						USHORT param_length,
						const UCHAR* param,
						SLONG slice_length,
						UCHAR* slice,
						SLONG* return_length)
{
/**************************************
 *
 *	g d s _ $ g e t _ s l i c e
 *
 **************************************
 *
 * Functional description
 *	Snatch a slice of an array.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	if (check_database(threadData, *db_handle))
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_get_slice2, *db_handle, *tra_handle, *array_id, sdl_length,
			 sdl, param_length, param, slice_length);
#endif

	try
	{
		//tdbb->tdbb_status_vector = user_status;

		Transaction* transaction =
			find_transaction(threadData, *tra_handle, isc_segstr_wrong_db);
	
		if (!array_id[0] && !array_id[1]) {
			MOVE_CLEAR(slice, slice_length);
			*return_length = 0;
		}
		else
			*return_length = BLB_get_slice(threadData,
									   transaction,
									   reinterpret_cast<bid*>(array_id),
									   sdl,
									   param_length,
									   (const SLONG*) (param),
									   slice_length, slice);
	}
	catch (OSRIException& exception)
	{
		return error(&exception, user_status);
	}

	return return_success(threadData);
}


ISC_STATUS GDS_OPEN_BLOB2(ISC_STATUS* user_status,
						Attachment** db_handle,
						Transaction** tra_handle,
						blb** blob_handle,
						bid* blob_id,
						USHORT bpb_length,
						const UCHAR* bpb)
{
/**************************************
 *
 *	g d s _ $ o p e n _ b l o b 2
 *
 **************************************
 *
 * Functional description
 *	Open an existing blob.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	NULL_CHECK(blob_handle, isc_bad_segstr_handle);

	if (check_database(threadData, *db_handle))
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_open_blob2, *db_handle, *tra_handle, *blob_handle, blob_id,
			 bpb_length, bpb);
#endif

	try
		{
		Transaction* transaction = find_transaction(threadData, *tra_handle, isc_segstr_wrong_db);
#ifdef SHARED_CACHE
		Sync syncTransaction(&transaction->syncInUse, "jrd8_open_blob");
		syncTransaction.lock(Exclusive);
#endif
		blb* blob = BLB_open2(threadData, transaction, blob_id, bpb_length, bpb);
		*blob_handle = blob;
	
#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
		LOG_call(log_handle_returned, *blob_handle);
#endif
		}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}

	return return_success(threadData);
}


ISC_STATUS GDS_PREPARE(ISC_STATUS * user_status,
						Transaction** tra_handle,
						USHORT length,
						const UCHAR * msg)
{
/**************************************
 *
 *	g d s _ $ p r e p a r e
 *
 **************************************
 *
 * Functional description
 *	Prepare a transaction for commit.  First phase of a two
 *	phase commit.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	CHECK_HANDLE((*tra_handle), type_tra, isc_bad_trans_handle);
	Transaction* transaction = *tra_handle;

	if (check_database(threadData, transaction->tra_attachment))
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_prepare2, *tra_handle, length, msg);
#endif

	lockAST(transaction->tra_attachment->att_database);

	if (prepare(threadData, transaction, user_status, length, msg))
	{
		unlockAST(transaction->tra_attachment->att_database);
		return error(NULL, user_status);
	}

	unlockAST(transaction->tra_attachment->att_database);

	return return_success(threadData);
}


ISC_STATUS GDS_PUT_SEGMENT(ISC_STATUS* user_status,
							blb** blob_handle,
							USHORT buffer_length,
							const UCHAR* buffer)
{
/**************************************
 *
 *	g d s _ $ p u t _ s e g m e n t
 *
 **************************************
 *
 * Functional description
 *	Abort a partially completed blob.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	blb* blob = check_blob(threadData, user_status, blob_handle);
	if (!blob)
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_put_segment, *blob_handle, buffer_length, buffer);
#endif

	//tdbb->tdbb_status_vector = user_status;
	try
	{
		BLB_put_segment(threadData, blob, buffer, buffer_length);
	}
	catch (OSRIException& exception)
	{
		return error(&exception, user_status);
	}

	return return_success(threadData);
}


ISC_STATUS GDS_PUT_SLICE(ISC_STATUS* user_status,
						Attachment** db_handle,
						Transaction** tra_handle,
						SLONG* array_id,
						USHORT sdl_length,
						const UCHAR* sdl,
						USHORT param_length,
						const UCHAR* param,
						SLONG slice_length,
						const UCHAR* slice)
{
/**************************************
 *
 *	g d s _ $ p u t _ s l i c e
 *
 **************************************
 *
 * Functional description
 *	Snatch a slice of an array.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	if (check_database(threadData, *db_handle))
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_put_slice, *db_handle, *tra_handle, *array_id, sdl_length,
			 sdl, param_length, param, slice_length, slice);
#endif

	//tdbb->tdbb_status_vector = user_status;
	try
	{
		Transaction* transaction =
			find_transaction(threadData, *tra_handle, isc_segstr_wrong_db);
		BLB_put_slice(threadData,
				  transaction,
				  reinterpret_cast<bid*>(array_id),
				  sdl,
				  param_length,
				  (const SLONG*) (param), slice_length, slice);
	}
	catch (OSRIException& exception)
	{
		return error(&exception, user_status);
	}

	return return_success(threadData);
}


ISC_STATUS GDS_QUE_EVENTS(ISC_STATUS* user_status,
							Attachment** handle,
							SLONG* id,
							SSHORT length,
							const UCHAR* items,
							FPTR_EVENT_CALLBACK ast,
							void* arg)
{
/**************************************
 *
 *	g d s _ $ q u e _ e v e n t s
 *
 **************************************
 *
 * Functional description
 *	Que a request for event notification.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	if (check_database(threadData, *handle))
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_que_events, *handle, length, items);
#endif

	//tdbb->tdbb_status_vector = user_status;
	try
		{
		DBB dbb = threadData.getDatabase(); //tdbb->tdbb_database;
		Lock* lock = dbb->dbb_lock;
		Attachment* attachment = threadData.getAttachment(); //tdbb->tdbb_attachment;
	
		if (!attachment->att_event_session &&
			 !(attachment->att_event_session = EVENT_create_session(user_status, dbb)))
			return error(NULL, user_status);
	
		*id = EVENT_que(user_status,
						attachment->att_event_session,
						lock->lck_length,
						(const TEXT*) & lock->lck_key, length, items, ast, arg);
	
#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
		LOG_call(log_handle_returned, *id);
#endif
		}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}

	return return_success(threadData);
}


ISC_STATUS GDS_RECEIVE(ISC_STATUS * user_status,
						JRD_REQ * req_handle,
						USHORT msg_type,
						USHORT msg_length,
						UCHAR * msg,
						SSHORT level
#ifdef SCROLLABLE_CURSORS
						, USHORT direction,
						ULONG offset
#endif
	)
{
/**************************************
 *
 *	g d s _ $ r e c e i v e
 *
 **************************************
 *
 * Functional description
 *	Get a record from the host program.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	ThreadData threadData (user_status);
	CHECK_HANDLE((*req_handle), type_req, isc_bad_req_handle);
	Request* request = *req_handle;

	if (check_database(threadData, request->req_attachment))
		return user_status[1];

	lockAST(request->req_attachment->att_database);

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_receive2, *req_handle, msg_type, msg_length, level);
#endif

	try
		{
		request = request->getInstantiatedRequest(level);

		if (!request->req_transaction)
			ERR_post(isc_bad_trans_handle, 0);

#ifdef SHARED_CACHE
		Sync syncTransaction(&request->req_transaction->syncInUse, "jrd8_receive");
		syncTransaction.lock(Exclusive);
#endif
		
	#ifdef SCROLLABLE_CURSORS
		if (direction)
			EXE_seek(threadData, request, direction, offset);
	#endif
	
		EXE_receive(threadData, request, msg_type, msg_length, msg);
		check_autocommit(request, threadData);
	
		if (request->req_flags & req_warning) 
			{
			request->req_flags &= ~req_warning;
			unlockAST(request->req_attachment->att_database);
			return error(NULL, user_status);
			}
		}
	catch (OSRIException& exception)
		{
		unlockAST(request->req_attachment->att_database);
		return error(&exception, user_status);
		}

	unlockAST(request->req_attachment->att_database);
	return return_success(threadData);
}


ISC_STATUS GDS_RECONNECT(ISC_STATUS* user_status,
						Attachment** db_handle,
						Transaction** tra_handle,
						SSHORT length,
						const UCHAR* id)
{
/**************************************
 *
 *	g d s _ $ r e c o n n e c t
 *
 **************************************
 *
 * Functional description
 *	Connect to a transaction in limbo.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	NULL_CHECK(tra_handle, isc_bad_trans_handle);
	Attachment* attachment = *db_handle;

	if (check_database(threadData, attachment))
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_reconnect, *db_handle, *tra_handle, length, id);
#endif

	lockAST(attachment->att_database);

	//tdbb->tdbb_status_vector = user_status;
	try
		{
		Transaction* transaction = TRA_reconnect(threadData, attachment, id, length);
		*tra_handle = transaction;
	
#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
		LOG_call(log_handle_returned, *tra_handle);
#endif
		}
	catch (OSRIException& exception)
		{
		unlockAST(attachment->att_database);
		return error(&exception, user_status);
		}

	unlockAST(attachment->att_database);
	
	return return_success(threadData);
}


ISC_STATUS GDS_RELEASE_REQUEST(ISC_STATUS * user_status, JRD_REQ * req_handle)
{
/**************************************
 *
 *	g d s _ $ r e l e a s e _ r e q u e s t
 *
 **************************************
 *
 * Functional description
 *	Release a request.
 *
 **************************************/

	api_entry_point_init(user_status);
	ThreadData threadData (user_status);
	CHECK_HANDLE((*req_handle), type_req, isc_bad_req_handle);
	Request* request = *req_handle;
	Attachment* attachment = request->req_attachment;

	if (check_database(threadData, attachment))
		return user_status[1];

	lockAST(attachment->att_database);

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_release_request, *req_handle);
#endif

	try
		{
		CMP_release(threadData, request);
		*req_handle = NULL;
		}
	catch (OSRIException& exception)
		{
		unlockAST(attachment->att_database);
		return error(&exception, user_status);
		}

	unlockAST(attachment->att_database);
	return return_success(threadData);
}


ISC_STATUS GDS_REQUEST_INFO(ISC_STATUS* user_status,
							JRD_REQ* req_handle,
							SSHORT level,
							SSHORT item_length,
							const UCHAR* items,
							SSHORT buffer_length,
							UCHAR* buffer)
{
/**************************************
 *
 *	g d s _ $ r e q u e s t _ i n f o
 *
 **************************************
 *
 * Functional description
 *	Provide information on blob object.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	Request* request = *req_handle;
	CHECK_HANDLE(request, type_req, isc_bad_req_handle);

	if (check_database(threadData, request->req_attachment))
		return user_status[1];

	lockAST(request->req_attachment->att_database);

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_request_info2, *req_handle, level, item_length, items,
			 buffer_length);
#endif

	try
		{
		request = request->getInstantiatedRequest(level);
		INF_request_info(threadData, request, items, item_length, buffer, buffer_length);
		}
	catch (OSRIException& exception)
		{
		unlockAST(request->req_attachment->att_database);
		return error(&exception, user_status);
		}

	unlockAST(request->req_attachment->att_database);
	return return_success(threadData);
}


ISC_STATUS GDS_ROLLBACK_RETAINING(ISC_STATUS* user_status,
									Transaction** tra_handle)
{
/**************************************
 *
 *	i s c _ r o l l b a c k _ r e t a i n i n g
 *
 **************************************
 *
 * Functional description
 *	Abort a transaction but keep the environment valid
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	Transaction* transaction = *tra_handle;
	CHECK_HANDLE(transaction, type_tra, isc_bad_trans_handle);

	if (check_database(threadData, transaction->tra_attachment))
		return user_status[1];

	lockAST(transaction->tra_attachment->att_database);
#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_rollback, *tra_handle);
#endif

	if (rollback(threadData, transaction, user_status, true))
	{
		unlockAST(transaction->tra_attachment->att_database);
		return error(NULL, user_status);
	}

	unlockAST(transaction->tra_attachment->att_database);

	return return_success(threadData);
}


ISC_STATUS GDS_ROLLBACK(ISC_STATUS * user_status, Transaction* * tra_handle)
{
/**************************************
 *
 *	g d s _ $ r o l l b a c k
 *
 **************************************
 *
 * Functional description
 *	Abort a transaction.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	ThreadData threadData (user_status);
	Transaction* transaction = *tra_handle;
	CHECK_HANDLE(transaction, type_tra, isc_bad_trans_handle);

	if (check_database(threadData, transaction->tra_attachment))
		return user_status[1];

	DBB dbb = transaction->tra_attachment->att_database;
	lockAST(dbb);

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_rollback, *tra_handle);
#endif

	if (rollback(threadData, transaction, user_status, false))
		{
		unlockAST(transaction->tra_attachment->att_database);
		return error(NULL, user_status);
		}

	unlockAST(dbb);
	*tra_handle = NULL;
	
	return return_success(threadData);
}


ISC_STATUS GDS_SEEK_BLOB(ISC_STATUS * user_status,
						blb* * blob_handle,
						SSHORT mode,
						SLONG offset,
						SLONG * result)
{
/**************************************
 *
 *	g d s _ $ s e e k _ b l o b
 *
 **************************************
 *
 * Functional description
 *	Seek a stream blob.
 *
 **************************************/

	api_entry_point_init(user_status);

	ThreadData threadData (user_status);

	blb* blob = check_blob(threadData, user_status, blob_handle);
	if (!blob)
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_blob_seek, *blob_handle, mode, offset);
#endif

	try
		{
		*result = BLB_lseek(blob, mode, offset);
		}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}

	return return_success(threadData);
}


ISC_STATUS GDS_SEND(ISC_STATUS * user_status,
					JRD_REQ * req_handle,
					USHORT msg_type,
					USHORT msg_length,
					const UCHAR * msg,
					SSHORT level)
{
/**************************************
 *
 *	g d s _ $ s e n d
 *
 **************************************
 *
 * Functional description
 *	Get a record from the host program.
 *
 **************************************/

	api_entry_point_init(user_status);

	ThreadData threadData (user_status);

	CHECK_HANDLE((*req_handle), type_req, isc_bad_req_handle);
	Request* request = *req_handle;

	if (check_database(threadData, request->req_attachment))
		return user_status[1];

	lockAST(request->req_attachment->att_database);
#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_send, *req_handle, msg_type, msg_length, msg, level);
#endif

	try
		{
		request = request->getInstantiatedRequest(level);

		if (!request->req_transaction)
			ERR_post(isc_bad_trans_handle, 0);

#ifdef SHARED_CACHE
		Sync syncTransaction(&request->req_transaction->syncInUse, "jrd8_send");
		syncTransaction.lock(Exclusive);
#endif

		EXE_send(threadData, request, msg_type, msg_length, msg);
		check_autocommit(request, threadData);
	
		if (request->req_flags & req_warning) 
			{
			request->req_flags &= ~req_warning;
			unlockAST(request->req_attachment->att_database);
			return error(NULL, user_status);
			}
		}
	catch (OSRIException& exception)
		{
		unlockAST(request->req_attachment->att_database);
		return error(&exception, user_status);
		}

	unlockAST(request->req_attachment->att_database);
	return return_success(threadData);
}

#ifdef SERVICES
ISC_STATUS GDS_SERVICE_ATTACH(ISC_STATUS* user_status,
							USHORT service_length,
							const TEXT* service_name,
							SVC* svc_handle,
							USHORT spb_length,
							const UCHAR* spb)
{
/**************************************
 *
 *	g d s _ $ s e r v i c e _ a t t a c h
 *
 **************************************
 *
 * Functional description
 *	Connect to an Interbase service.
 *
 **************************************/

	api_entry_point_init(user_status);

	if (*svc_handle)
		return handle_error(user_status, isc_bad_svc_handle, 0);

	ThreadData threadData (user_status);

	try
		{
		*svc_handle = SVC_attach(service_name, spb_length, spb);
		}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}

	return return_success(threadData);
}


ISC_STATUS GDS_SERVICE_DETACH(ISC_STATUS* user_status, SVC* svc_handle)
{
/**************************************
 *
 *	g d s _ $ s e r v i c e _ d e t a c h
 *
 **************************************
 *
 * Functional description
 *	Close down a service.
 *
 **************************************/

	api_entry_point_init(user_status);

	ThreadData threadData (user_status);

	Service* service = *svc_handle;
	CHECK_HANDLE(service, type_svc, isc_bad_svc_handle);

	try
		{
		threadData.setDatabase (NULL); //tdbb->tdbb_database = NULL;
		SVC_detach(service);
		*svc_handle = NULL;
		}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}

	return return_success(threadData);
}


ISC_STATUS GDS_SERVICE_QUERY(ISC_STATUS*	user_status,
							SVC*	svc_handle,
							ULONG*	reserved,
							USHORT	send_item_length,
							const UCHAR*	send_items,
							USHORT	recv_item_length,
							const UCHAR*	recv_items,
							USHORT	buffer_length,
							UCHAR*	buffer)
{
/**************************************
 *
 *	g d s _ $ s e r v i c e _ q u e r y
 *
 **************************************
 *
 * Functional description
 *	Provide information on service object.
 *
 *	NOTE: The parameter RESERVED must not be used
 *	for any purpose as there are networking issues
 *	involved (as with any handle that goes over the
 *	network).  This parameter will be implemented at
 *	a later date.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	Service* service = *svc_handle;
	CHECK_HANDLE(service, type_svc, isc_bad_svc_handle);

	//tdbb->tdbb_status_vector = user_status;
	//tdbb->tdbb_database = NULL;
	threadData.setDatabase (NULL);

	try
	{
		if (service->svc_spb_version == isc_spb_version1)
			SVC_query(service, send_item_length, send_items, recv_item_length,
					recv_items, buffer_length, buffer);
		else {
			/* For SVC_query2, we are going to completly dismantle user_status (since at this point it is
			* meaningless anyway).  The status vector returned by this function can hold information about
			* the call to query the service manager and/or a service thread that may have been running.
			*/
	
			SVC_query2(service, threadData, send_item_length, send_items,
					recv_item_length, recv_items, buffer_length, buffer);
	
			/* if there is a status vector from a service thread, copy it into the thread status */
			int len, warning;
			PARSE_STATUS(service->svc_status, len, warning);
			if (len) {
				MOVE_FASTER(service->svc_status, threadData.threadData->tdbb_status_vector, //tdbb->tdbb_status_vector,
							sizeof(ISC_STATUS) * len);
	
				/* Empty out the service status vector */
				memset(service->svc_status, 0,
					ISC_STATUS_LENGTH * sizeof(ISC_STATUS));
			}
	
			if (user_status[1])
				return error(&exception, user_status);
		}
	}
	catch (OSRIException& exception)
	{
		return error(&exception, user_status);
	}
	return return_success(threadData);
}


ISC_STATUS GDS_SERVICE_START(ISC_STATUS*	user_status,
							SVC*	svc_handle,
							ULONG*	reserved,
							USHORT	spb_length,
							const UCHAR*	spb)
{
/**************************************
 *
 *	g d s _ s e r v i c e _ s t a r t
 *
 **************************************
 *
 * Functional description
 *	Start the specified service
 *
 *	NOTE: The parameter RESERVED must not be used
 *	for any purpose as there are networking issues
 *  	involved (as with any handle that goes over the
 *   	network).  This parameter will be implemented at
 * 	a later date.
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	Service* service = *svc_handle;
	CHECK_HANDLE(service, type_svc, isc_bad_svc_handle);

	//tdbb->tdbb_status_vector = user_status;
	//tdbb->tdbb_database = NULL;
	threadData.setDatabase (NULL);

	try
	{
		SVC_start(service, spb_length, spb);
	
		if (service->svc_status[1]) {
			ISC_STATUS* svc_status = service->svc_status;
			ISC_STATUS* tdbb_status = threadData.getStatusVector();//tdbb->tdbb_status_vector;
	
			while (*svc_status) {
				*tdbb_status++ = *svc_status++;
			}
			*tdbb_status = isc_arg_end;
		}
	
		if (user_status[1]) {
			return error(&exception, user_status);
		}
	}
	catch (OSRIException& exception)
	{
		return error(&exception, user_status);
	}

	return return_success(threadData);
}
#endif // SERVICES

ISC_STATUS GDS_START_AND_SEND(ISC_STATUS* user_status,
							JRD_REQ* req_handle,
							Transaction** tra_handle,
							USHORT msg_type,
							USHORT msg_length,
							const UCHAR* msg,
							SSHORT level)
{
/**************************************
 *
 *	g d s _ $ s t a r t _ a n d _ s e n d
 *
 **************************************
 *
 * Functional description
 *	Get a record from the host program.
 *
 **************************************/
	api_entry_point_init(user_status);
	ThreadData threadData (user_status);
	Request* request = *req_handle;
	CHECK_HANDLE(request, type_req, isc_bad_req_handle);

	if (check_database(threadData, request->req_attachment))
		return user_status[1];

	lockAST(request->req_attachment->att_database);

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_start_and_send, *req_handle, *tra_handle, msg_type,
			 msg_length, msg, level);
#endif

	try
		{
		Transaction *transaction = find_transaction(threadData, *tra_handle, isc_req_wrong_db);
#ifdef SHARED_CACHE
		Sync syncTransaction(&transaction->syncInUse, "jrd8_start_and_send");
		syncTransaction.lock(Exclusive);
#endif
	
		if (level)
			request = CMP_clone_request(threadData, request, level, false);
	
		//EXE_unwind(threadData, request);
		request->unwind();
		EXE_start(threadData, request, transaction);
		EXE_send(threadData, request, msg_type, msg_length, msg);
		check_autocommit(request, threadData);
	
		if (request->req_flags & req_warning) 
			{
			request->req_flags &= ~req_warning;
			unlockAST(request->req_attachment->att_database);
			return error(NULL, user_status);
			}
		}
	catch (OSRIException& exception)
		{
		unlockAST(request->req_attachment->att_database);
		return error(&exception, user_status);
		}

	unlockAST(request->req_attachment->att_database);
	return return_success(threadData);
}


ISC_STATUS GDS_START(ISC_STATUS * user_status,
					JRD_REQ * req_handle,
					Transaction** tra_handle,
					SSHORT level)
{
/**************************************
 *
 *	g d s _ $ s t a r t
 *
 **************************************
 *
 * Functional description
 *	Get a record from the host program.
 *
 **************************************/
	api_entry_point_init(user_status);
	ThreadData threadData (user_status);

	Request* request = *req_handle;
	CHECK_HANDLE(request, type_req, isc_bad_req_handle);

	if (check_database(threadData, request->req_attachment))
		return user_status[1];

	lockAST(request->req_attachment->att_database);
#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_start, *req_handle, *tra_handle, level);
#endif

	try
		{
		Transaction* transaction = find_transaction(threadData, *tra_handle, isc_req_wrong_db);
#ifdef SHARED_CACHE
		Sync syncTransaction(&transaction->syncInUse, "jrd8_start");
		syncTransaction.lock(Exclusive);
#endif
		
		if (level)
			request = CMP_clone_request(threadData, request, level, false);
	
		//EXE_unwind(threadData, request);
		request->unwind();
		EXE_start(threadData, request, transaction);
		check_autocommit(request, threadData);
	
		if (request->req_flags & req_warning) 
			{
			request->req_flags &= ~req_warning;
			unlockAST(request->req_attachment->att_database);
			return error(NULL, user_status);
			}
		}
	catch (OSRIException& exception)
		{
		unlockAST(request->req_attachment->att_database);
		return error(&exception, user_status);
		}

	unlockAST(request->req_attachment->att_database);
	return return_success(threadData);
}


ISC_STATUS GDS_START_MULTIPLE(ISC_STATUS * user_status,
							Transaction** tra_handle,
							USHORT count,
							const TransElement * vector)
{
/**************************************
 *
 *	g d s _ $ s t a r t _ m u l t i p l e
 *
 **************************************
 *
 * Functional description
 *	Start a transaction.
 *
 **************************************/
	DBB dbb;
	api_entry_point_init(user_status);
	ThreadData threadData (user_status);

	NULL_CHECK(tra_handle, isc_bad_trans_handle);
	const TransElement* const end = vector + count;
	const TransElement *v;

	for (v = vector; v < end; v++) 
		{
		if (check_database(threadData, *v->teb_database))
			return user_status[1];
			
		dbb = threadData.getDatabase(); //tdbb->tdbb_database;
		dbb->decrementUseCount();
		}

	if (check_database(threadData, *vector->teb_database))
		return user_status[1];

	Transaction* prior = NULL;
	Transaction* transaction = NULL;

	try 
		{
		for (v = vector; v < end; v++)
			{
			Attachment* attachment = *v->teb_database;
			
			if (check_database(threadData, attachment)) 
				return user_status[1];

			lockAST(attachment->att_database);
			
	#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
			LOG_call(log_start_multiple, *tra_handle, count, vector);
	#endif

			transaction = TRA_start(threadData, attachment, v->teb_tpb_length, v->teb_tpb);
			transaction->tra_sibling = prior;
			prior = transaction;
			dbb = threadData.getDatabase(); //tdbb->tdbb_database;
			dbb->decrementUseCount();
			unlockAST(dbb);
			}
		}
	catch (OSRIException& exception) 
		{
		dbb = threadData.getDatabase(); //tdbb->tdbb_database;
		dbb->decrementUseCount();
		
		if (prior) 
			{
			ISC_STATUS_ARRAY temp_status;
			rollback(threadData, prior, temp_status, false);
			}
			
		unlockAST(dbb);
		return error(&exception, user_status);
		}

	*tra_handle = transaction;
	
#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_handle_returned, *tra_handle);
#endif

	return return_success(threadData);
}


ISC_STATUS GDS_START_TRANSACTION(ISC_STATUS * user_status,
								Transaction** tra_handle,
								SSHORT count,
								...)
{
/**************************************
 *
 *	g d s _ $ s t a r t _ t r a n s a c t i o n
 *
 **************************************
 *
 * Functional description
 *	Start a transaction.
 *
 **************************************/
	TransElement tebs[16], *teb, *end;
	va_list ptr;
	api_entry_point_init(user_status);
	VA_START(ptr, count);

	for (teb = tebs, end = teb + count; teb < end; teb++) 
		{
		teb->teb_database = va_arg(ptr, Attachment* *);
		teb->teb_tpb_length = va_arg(ptr, int);
		teb->teb_tpb = va_arg(ptr, UCHAR *);
		}

	return GDS_START_MULTIPLE(user_status, tra_handle, count, tebs);
}


ISC_STATUS GDS_TRANSACT_REQUEST(ISC_STATUS*	user_status,
								Attachment** db_handle,
								Transaction** tra_handle,
								USHORT blr_length,
								const UCHAR* blr,
								USHORT in_msg_length,
								const UCHAR* in_msg,
								USHORT out_msg_length,
								UCHAR* out_msg)
{
/**************************************
 *
 *	i s c _ t r a n s a c t _ r e q u e s t
 *
 **************************************
 *
 * Functional description
 *	Execute a procedure.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	Attachment* attachment = *db_handle;
	if (check_database(threadData, attachment))
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_transact_request, *db_handle, *tra_handle,
			 blr_length, blr, in_msg_length, in_msg, out_msg_length);
#endif

	JrdMemoryPool *old_pool, *new_pool;
	new_pool = old_pool = NULL;
	Request* request = NULL;

	//tdbb->tdbb_status_vector = user_status;

	Transaction* transaction;
	try 
		{
		transaction = find_transaction(threadData, *tra_handle, isc_req_wrong_db);
#ifdef SHARED_CACHE
		Sync syncTransaction(&transaction->syncInUse, "jrd8_transact");
		syncTransaction.lock(Exclusive);
#endif

		old_pool = threadData.getPool();
		new_pool = JrdMemoryPool::createPool(transaction->tra_attachment->att_database);
		lockAST(transaction->tra_attachment->att_database);
		threadData.setPool (new_pool);
		CompilerScratch* csb = PAR_parse(threadData, reinterpret_cast<const UCHAR*>(blr), FALSE);
		request = CMP_make_request(threadData, csb);

		for (const AccessItem* access = request->req_access; access; access = access->acc_next)
			{
			SecurityClass* sec_class = SCL_get_class(threadData, access->acc_security_name);
			SCL_check_access(threadData, sec_class, access->acc_view_id, access->acc_trg_name,
							access->acc_prc_name, access->acc_mask,
							access->acc_type, access->acc_name);
			}

		jrd_nod* in_message = NULL;
		jrd_nod* out_message = NULL;
		jrd_nod* node;
		
		for (size_t i = 0; i < csb->csb_rpt.getCount(); i++)
			if ( (node = csb->csb_rpt[i].csb_message) )
				{
				if ((int) (IPTR) node->nod_arg[e_msg_number] == 0) 
					in_message = node;
				else if ((int) (IPTR) node->nod_arg[e_msg_number] == 1) 
					out_message = node;
				}

		//tdbb->tdbb_default = old_pool;
		threadData.setPool (old_pool);
		old_pool = NULL;

		request->req_attachment = attachment;

		USHORT len;
		
		if (in_msg_length)
			{
			if (in_message) 
				{
				const Format* format = (Format*) in_message->nod_arg[e_msg_format];
				len = format->fmt_length;
				}
			else 
				len = 0;
			if (in_msg_length != len)
				ERR_post(isc_port_len,
						isc_arg_number, in_msg_length,
						isc_arg_number, len, 0);
			if ((U_IPTR) in_msg & (ALIGNMENT - 1))
				MOVE_FAST(in_msg, IMPURE (request, in_message->nod_impure), in_msg_length);
			else 
				MOVE_FASTER(in_msg, IMPURE (request, in_message->nod_impure), in_msg_length);
			}

		EXE_start(threadData, request, transaction);

		if (out_message) 
			{
			const Format* format = (Format*) out_message->nod_arg[e_msg_format];
			len = format->fmt_length;
			}
		else 
			len = 0;

		if (out_msg_length != len) 
			ERR_post(isc_port_len,
					isc_arg_number, out_msg_length,
					isc_arg_number, len, 0);

		if (out_msg_length) 
			{
			if ((U_IPTR) out_msg & (ALIGNMENT - 1)) 
				MOVE_FAST(IMPURE (request, out_message->nod_impure), out_msg, out_msg_length);
			else
				MOVE_FASTER(IMPURE (request, out_message->nod_impure), out_msg, out_msg_length);
			}

		check_autocommit(request, threadData);
		CMP_release(threadData, request);

		unlockAST(transaction->tra_attachment->att_database);
		return return_success(threadData);
		}
	catch (OSRIException& exception)
		{
		/* Set up to trap error in case release pool goes wrong. */

		try {
			if (old_pool)
				threadData.setPool (old_pool);
			if (request)
				CMP_release(threadData, request);
			else if (new_pool)
				JrdMemoryPool::deletePool(new_pool);
			}	// try
		catch (OSRIException& exception) 
			{ 
			exception; 
			}

		unlockAST(transaction->tra_attachment->att_database);
		return error(&exception, user_status);
		}
}


ISC_STATUS GDS_TRANSACTION_INFO(ISC_STATUS* user_status,
								Transaction** tra_handle,
								SSHORT item_length,
								const UCHAR* items,
								SSHORT buffer_length,
								UCHAR* buffer)
{
/**************************************
 *
 *	g d s _ $ t r a n s a c t i o n _ i n f o
 *
 **************************************
 *
 * Functional description
 *	Provide information on blob object.
 *
 **************************************/
	//struct tdbb thd_context;

	api_entry_point_init(user_status);

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	Transaction* transaction = *tra_handle;
	CHECK_HANDLE(transaction, type_tra, isc_bad_trans_handle);

	if (check_database(threadData, transaction->tra_attachment))
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_transaction_info2, *tra_handle, item_length, items,
			 buffer_length);
#endif
	lockAST(transaction->tra_attachment->att_database);

	//tdbb->tdbb_status_vector = user_status;
	try
		{
		INF_transaction_info(transaction, items, item_length, buffer,
						 buffer_length);
		}
	catch (OSRIException& exception)
		{
		unlockAST(transaction->tra_attachment->att_database);
		return error(&exception, user_status);
		}

	unlockAST(transaction->tra_attachment->att_database);
	return return_success(threadData);
}


ISC_STATUS GDS_UNWIND(ISC_STATUS * user_status,
						JRD_REQ * req_handle,
						SSHORT level)
{
/**************************************
 *
 *	g d s _ $ u n w i n d
 *
 **************************************
 *
 * Functional description
 *	Unwind a running request.  This is potentially nasty since it can
 *	be called asynchronously.
 *
 **************************************/

	api_entry_point_init(user_status);

	ThreadData threadData (user_status);

	CHECK_HANDLE((*req_handle), type_req, isc_bad_req_handle);
	Request* request = *req_handle;

	/* Make sure blocks look and feel kosher */

	DBB dbb;
	Attachment* attachment = request->req_attachment;
	
	if (!attachment || !(dbb = attachment->att_database))
		return handle_error(user_status, isc_bad_db_handle, threadData);

	/* Make sure this is a valid attachment */

	Attachment* attach;
	
	for (attach = dbb->dbb_attachments; attach && attach != attachment; attach = attach->att_next)
		;

	if (!attach)
		return handle_error(user_status, isc_bad_db_handle, threadData);

	threadData.setDatabase (dbb);

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call(log_unwind, *req_handle, level);
#endif

	lockAST(dbb);

/* Set up error handling to restore old state */

	//tdbb->tdbb_status_vector = user_status;
	threadData.setAttachment (attachment);

	try 
		{

		/* Pick up and validate request level */

		request = request->getInstantiatedRequest(level);
		threadData.threadData->tdbb_request = NULL;
		threadData.threadData->tdbb_transaction = NULL;

		/* Unwind request.  This just tweaks some bits */

		//EXE_unwind(threadData, request);
		request->unwind();

		/* Restore old state and get out */

		JRD_restore_context();

		user_status[0] = isc_arg_gds;
		user_status[2] = isc_arg_end;

		unlockAST(dbb);
		return (user_status[1] = FB_SUCCESS);
		}
	catch (OSRIException& exception) 
		{
		exception;
		JRD_restore_context();
		unlockAST(dbb);
		return user_status[1];
		}
}

ISC_STATUS jrd8_update_account_info(ISC_STATUS* user_status, Attachment** handle, 
									int apbLength, const UCHAR *apb)
{
/**************************************
 *
 *	j r d 8 _ u p d a t e _ a c c o u n t _ i n f o
 *
 **************************************
 *
 * Functional description
 *	Update database accounts.
 *
 **************************************/

	api_entry_point_init(user_status);
	ThreadData threadData (user_status);

	if (check_database(threadData, *handle))
		return user_status[1];

	try
		{
		threadData.getAttachment()->updateAccountInfo(threadData, apbLength, apb);
		}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}

	return return_success(threadData);
}

ISC_STATUS jrd8_user_info(ISC_STATUS* user_status, Attachment** handle, 
						  int dpbLength, const UCHAR *dpb,
						  int itemsLength, const UCHAR *items,
						  int bufferLength, UCHAR *buffer)
{
/**************************************
 *
 *	j r d 8 _ u s e r _ i n f o
 *
 **************************************
 *
 * Functional description
 *	Update database accounts.
 *
 **************************************/

	api_entry_point_init(user_status);
	ThreadData threadData (user_status);

	if (check_database(threadData, *handle))
		return user_status[1];

	try
		{
		threadData.getDatabase()->authenticateUser(threadData, dpbLength, dpb,
										   itemsLength, items, bufferLength, buffer);
		}
	catch (OSRIException& exception)
		{
		return error(&exception, user_status);
		}

	return return_success(threadData);
}



#ifdef MULTI_THREAD
void JRD_blocked(Attachment* blocking, BlockingThread** que)
{
/**************************************
 *
 *	J R D _ b l o c k e d
 *
 **************************************
 *
 * Functional description
 *	We're blocked by another thread.  Unless it would cause
 *	a deadlock, wait for the other other thread (it will
 *	wake us up.
 *
 **************************************/
	thread_db* tdbb = get_thread_data();
	DBB  dbb  = tdbb->tdbb_database;

	/* Check for deadlock.  If there is one, complain */

	Attachment* attachment;
	
	for (attachment = blocking; attachment; attachment = attachment->att_blocking)
		if (attachment == tdbb->tdbb_attachment) 
			ERR_post(isc_deadlock, 0);

	BlockingThread* block = dbb->dbb_free_btbs;
	
	if (block) 
		dbb->dbb_free_btbs = block->btb_next;
	else 
		block = FB_NEW(*dbb->dbb_permanent) BlockingThread;

	block->btb_thread_id = (long) SCH_current_thread();
	block->btb_next = *que;
	*que = block;
	attachment = tdbb->tdbb_attachment;
	attachment->att_blocking = blocking;

	//SCH_hiber();

	attachment->att_blocking = NULL;
}
#endif


#ifdef SUPERSERVER
USHORT JRD_getdir(TEXT* buf, USHORT len)
{
/**************************************
 *
 *	J R D _ g e t d i r
 *
 **************************************
 *
 * Functional description
 *	Current working directory is cached in the attachment
 *	block.  get it out. This function could be called before
 *	an attachment is created. In such a case thread specific
 *	data (t_data) will hold the user name which will be used
 *	to get the users home directory.
 *
 **************************************/
	char* t_data = NULL;

	fb_assert(buf != NULL);
	fb_assert(len > 0);

	THD_getspecific_data((void**) &t_data);

	if (t_data) 
		{
#ifdef WIN_NT
		GetCurrentDirectory(len, buf);
#else
		const struct passwd* pwd;
		strcpy(buf, t_data);
		pwd = getpwnam(buf);
		if (pwd)
			strcpy(buf, pwd->pw_dir);
		else	/* No home dir for this users here. Default to server dir */
#ifdef HAVE_GETCWD
			getcwd(buf, len);
#else
			getwd(buf);
#endif
#endif
		}
	else
		{
		thread_db* tdbb = get_thread_data();
		Attachment* attachment;

		/** If the server has not done a SET_THREAD_DATA prior to this call
			(which will be the case when connecting via IPC), tdbb will
			be NULL so do not attempt to get the attachment handle from
			tdbb. Just return 0 as described below.  NOTE:  The only time
			this code is entered via IPC is if the database name = "".
		**/

		/** In case of backup/restore APIs, SET_THREAD_DATA has been done but
			the thread's context is a 'gbak' specific, so don't try extract
			attachment from there.
		**/

		if (tdbb && (tdbb->tdbb_thd_data.thdd_type == THDD_TYPE_TDBB))
			attachment = tdbb->tdbb_attachment;
		else
			return 0;

		/**
			An older version of client will not be sending isc_dpb_working directory
			so in all probabilities attachment->att_working_directory will be null.
			return 0 so that ISC_expand_filename will create the file in fbserver's dir
		**/
		
		if (!attachment || !attachment->att_working_directory ||
			len - 1 < attachment->att_working_directory.length())
			return 0;

		strcpy (buf,attachment->att_working_directory);
		}

	return strlen(buf);
}
#endif /* SUPERSERVER */



#ifdef SUPERSERVER
void JRD_print_all_counters(const char* fname)
{
/*****************************************************
 *
 *	J R D _ p r i n t _ a l l _ c o u n t e r s
 *
 *****************************************************
 *
 * Functional description
 *	print memory counters from DSQL, ENGINE & REMOTE
 *      component
 *
 ******************************************************/
	// Skidder: This may be ported to new memory pools, but
    // I think this debugging feature may be safely eradicated
	return;
}

#endif

#ifdef DEBUG_PROCS
void JRD_print_procedure_info(thread_db* tdbb, const char* mesg)
{
/*****************************************************
 *
 *	J R D _ p r i n t _ p r o c e d u r e _ i n f o
 *
 *****************************************************
 *
 * Functional description
 *	print name , use_count of all procedures in
 *      cache
 *
 ******************************************************/
	TEXT fname[MAXPATHLEN];

	gds__prefix(fname, "proc_info.log");
	IB_FILE* fptr = ib_fopen(fname, "a+");
	if (!fptr) {
		char buff[256];
		sprintf(buff, "Failed to open %s\n", fname);
		gds__log(buff, 0);
		return;
	}

	if (mesg)
		ib_fputs(mesg, fptr);
	ib_fprintf(fptr,
			   "Prc Name      , prc id , flags  ,  Use Count , Alter Count\n");


	vec* procedures = tdbb->tdbb_database->dbb_procedures;
	if (procedures) {
		vec::iterator ptr, end;
		for (ptr = procedures->begin(), end = procedures->end();
					ptr < end; ptr++)
		{
			const jrd_prc* procedure = (jrd_prc*) *ptr;
			if (procedure)
				ib_fprintf(fptr, "%s  ,  %d,  %X,  %d, %d\n",
							(procedure->prc_name) ?
								(char*) procedure->prc_name->str_data : "NULL",
							procedure->prc_id,
							procedure->prc_flags, procedure->prc_use_count,
							procedure->prc_alter_count);
		}
	}
	else
		ib_fprintf(fptr, "No Cached Procedures\n");

	ib_fclose(fptr);

}
#endif /* DEBUG_PROCS */

#ifdef MULTI_THREAD
BOOLEAN JRD_reschedule(thread_db* tdbb, SLONG quantum, bool punt)
{
/**************************************
 *
 *	J R D _ r e s c h e d u l e
 *
 **************************************
 *
 * Functional description
 *	Somebody has kindly offered to relinquish
 *	control so that somebody else may run.
 *
 **************************************/
 
 #ifdef OBSOLETE
	if (tdbb->tdbb_inhibit)
		return FALSE;

	/* Force garbage colletion activity to yield the
	   processor in case client threads haven't had
	   an opportunity to enter the scheduling queue. */

	if (!(tdbb->tdbb_flags & TDBB_sweeper))
		SCH_schedule();
	else 
		{
		THREAD_EXIT;
		THREAD_YIELD;
		THREAD_ENTER;
		}

	/* If database has been shutdown then get out */

	DBB dbb = tdbb->tdbb_database;
	Attachment* attachment = tdbb->tdbb_attachment;
	
	if (attachment) 
		{
		if (dbb->dbb_ast_flags & DBB_shutdown && attachment->att_flags & ATT_shutdown)
			{
			if (punt) 
				{
				CCH_UNWIND(tdbb, FALSE);
				ERR_post(isc_shutdown, 0);
				}
			else 
				{
				ISC_STATUS* status = tdbb->tdbb_status_vector;
				*status++ = isc_arg_gds;
				*status++ = isc_shutdown;
				*status++ = isc_arg_end;
				return TRUE;
				}
			}
#ifdef CANCEL_OPERATION

		/* If a cancel has been raised, defer its acknowledgement
		   when executing in the context of an internal request or
		   the system transaction. */

		if ((attachment->att_flags & ATT_cancel_raise) &&
			!(attachment->att_flags & ATT_cancel_disable))
			{
			const Transaction* transaction;
			Request* request = tdbb->tdbb_request;
			
			if ((!request || !(request->req_flags & (req_internal | req_sys_trigger))) &&
				(!(transaction = tdbb->tdbb_transaction) ||
				 !(transaction->tra_flags & TRA_system)))
				{
				attachment->att_flags &= ~ATT_cancel_raise;
				
				if (punt) 
					{
					CCH_UNWIND(tdbb, FALSE);
					ERR_post(isc_cancelled, 0);
					}
				else 
					{
					ISC_STATUS* status = tdbb->tdbb_status_vector;
					*status++ = isc_arg_gds;
					*status++ = isc_cancelled;
					*status++ = isc_arg_end;
					return TRUE;
					}
				}
			}
	}
#endif
#endif // OBSOLETE

	return FALSE;
}
#endif

void JRD_restore_context(void)
{
/**************************************
 *
 *	J R D _ r e s t o r e _ c o n t e x t
 *
 **************************************
 *
 * Functional description
 *	Restore the previous thread specific data
 *	and cleanup and objects that remain in use.
 *
 **************************************/
#ifdef OBSOLETE
	thread_db* tdbb) = get_thread_data();
	THD_restore_specific();

#ifdef DEV_BUILD
	if (tdbb->tdbb_status_vector &&
		!tdbb->tdbb_status_vector[1]
		&& cleaned_up)
	{
		gds__log("mutexes or pages in use on successful return");
	}
#endif
#endif /* obsolete code */
}


void JRD_set_context(thread_db* tdbb)
{
/**************************************
 *
 *	J R D _ s e t _ c o n t e x t
 *
 **************************************
 *
 * Functional description
 *	Set the thread specific data.  Initialize
 *	the in-use blocks so that we can unwind
 *	cleanly if an error occurs.
 *
 **************************************/

	//INUSE_clear(&tdbb->tdbb_mutexes);
	//INUSE_clear(&tdbb->tdbb_rw_locks);
	INUSE_clear(&tdbb->tdbb_pages);
	//tdbb->tdbb_status_vector = NULL;
	THD_put_specific((THDD) tdbb,THDD_TYPE_TDBB);
	tdbb->tdbb_thd_data.thdd_type = THDD_TYPE_TDBB;
}


#ifdef MULTI_THREAD
void JRD_unblock(BlockingThread** que)
{
/**************************************
 *
 *	J R D _ u n b l o c k
 *
 **************************************
 *
 * Functional description
 *	Unblock a linked list of blocked threads.  Rather
 *	than worrying about which, let 'em all loose.
 *
 **************************************/
	BlockingThread* block;

	DBB dbb = get_dbb();

	while (block = *que) 
		{
		*que = block->btb_next;

		if (block->btb_thread_id) 
			SCH_wake((struct thread *) (IPTR) block->btb_thread_id);

		block->btb_next = dbb->dbb_free_btbs;
		dbb->dbb_free_btbs = block;
		}
}
#endif


void jrd_vtof(const char* string, char* field, SSHORT length)
{
/**************************************
 *
 *	j r d _ v t o f
 *
 **************************************
 *
 * Functional description
 *	Move a null terminated string to a fixed length
 *	field.
 *	If the length of the string pointed to by 'field'
 *	is less than 'length', this function pads the
 *	destination string with space upto 'length' bytes.
 *
 *	The call is primarily generated  by the preprocessor.
 *
 *	This is the same code as gds__vtof but is used internally.
 *
 **************************************/

	while (*string) {
		*field++ = *string++;
		if (--length <= 0) {
			return;
		}
	}

	if (length) {
		do {
			*field++ = ' ';
		} while (--length);
	}
}

static blb* check_blob(thread_db* tdbb, ISC_STATUS* user_status, blb** blob_handle)
{
/**************************************
 *
 *	c h e c k _ b l o b
 *
 **************************************
 *
 * Functional description
 *	Check out a blob handle for goodness.  Return
 *	the address of the blob if ok, NULL otherwise.
 *
 **************************************/
	Transaction* transaction = 0;

	SET_TDBB(tdbb);
	 // this const made to check no accidental changes happen
	const blb* const blob = *blob_handle;

	if (!blob ||
		//(MemoryPool::blk_type(blob) != type_blb) ||
		check_database(tdbb, blob->blb_attachment) ||
		!(transaction = blob->blb_transaction))
		//MemoryPool::blk_type(transaction) != type_tra)
		{
		handle_error(user_status, isc_bad_segstr_handle, tdbb);
		return NULL;
		}

	tdbb->tdbb_transaction = transaction;

	return const_cast<blb*>(blob); // harmless, see comment above
}


static ISC_STATUS check_database(thread_db* tdbb, Attachment* attachment)
{
/**************************************
 *
 *	c h e c k _ d a t a b a s e
 *
 **************************************
 *
 * Functional description
 *	Check an attachment for validity, setting
 *	up environment.
 *
 **************************************/
	SET_TDBB(tdbb);

	/* Make sure blocks look and feel kosher */

	DBB dbb;
	
	if (!attachment || !(dbb = attachment->att_database))
		return handle_error(tdbb->tdbb_status_vector, isc_bad_db_handle, tdbb);

	/* Make sure this is a valid attachment */

#ifndef SUPERSERVER
	const Attachment* attach;
	
	for (attach = dbb->dbb_attachments; attach; attach = attach->att_next)
		if (attach == attachment)
			break;

	if (!attach)
		return handle_error(tdbb->tdbb_status_vector, isc_bad_db_handle, tdbb);
#endif

	tdbb->tdbb_database = dbb;
	tdbb->tdbb_attachment = attachment;
	tdbb->tdbb_request = NULL;
	tdbb->tdbb_transaction = NULL;
	tdbb->tdbb_default = NULL;
	tdbb->tdbb_inhibit = 0;
	tdbb->tdbb_flags = 0;

	/* Count active threads in database */

	dbb->incrementUseCount();
    
	if (dbb->dbb_flags & DBB_bugcheck) 
		{
		ISC_STATUS* ptr = tdbb->tdbb_status_vector;
		*ptr++ = isc_arg_gds;
		*ptr++ = isc_bug_check;
		*ptr++ = isc_arg_string;
		*ptr++ = (ISC_STATUS) "can't continue after bugcheck";
		*ptr = isc_arg_end;
		
		return error(NULL, tdbb->tdbb_status_vector);
		}

	if (attachment->att_flags & ATT_shutdown || 
		 ((dbb->dbb_ast_flags & DBB_shutdown) && 
		  !((attachment->userFlags & (USR_locksmith | USR_owner)) || attachment->userData.authenticating)))
		{
		ISC_STATUS* ptr = tdbb->tdbb_status_vector;
		*ptr++ = isc_arg_gds;
		*ptr++ = isc_shutdown;
		*ptr++ = isc_arg_string;
		*ptr++ = (ISC_STATUS) (const char*) attachment->att_filename;
		*ptr = isc_arg_end;
		
		return error(NULL, tdbb->tdbb_status_vector);
		}

#ifdef CANCEL_OPERATION
	if ((attachment->att_flags & ATT_cancel_raise) &&
		!(attachment->att_flags & ATT_cancel_disable))
		{
		attachment->att_flags &= ~ATT_cancel_raise;
		ISC_STATUS* ptr = tdbb->tdbb_status_vector;
		*ptr++ = isc_arg_gds;
		*ptr++ = isc_cancelled;
		*ptr++ = isc_arg_end;
		
		return error(NULL, tdbb->tdbb_status_vector);
		}
#endif

	return FB_SUCCESS;
}


static void cleanup(void* arg)
{
/**************************************
 *
 *	c l e a n u p
 *
 **************************************
 *
 * Functional description
 *	Exit handler for image exit.
 *
 **************************************/

	initialized = false;
}


static ISC_STATUS commit(ISC_STATUS* user_status, Transaction** tra_handle, const bool retaining_flag)
{
/**************************************
 *
 *	c o m m i t
 *
 **************************************
 *
 * Functional description
 *	Commit a transaction.
 *
 **************************************/
	ThreadData threadData (user_status);

	CHECK_HANDLE((*tra_handle), type_tra, isc_bad_trans_handle);
	Transaction *transaction = *tra_handle;
	Transaction *next = transaction;

	if (check_database(threadData, transaction->tra_attachment))
		return user_status[1];

#ifdef REPLAY_OSRI_API_CALLS_SUBSYSTEM
	LOG_call((retaining_flag) ? log_commit_retaining : log_commit,
			 *tra_handle);
#endif

	ISC_STATUS* ptr = user_status;

	if (transaction->tra_sibling &&
		 !(transaction->tra_flags & TRA_prepared) &&
		 prepare(threadData, transaction, ptr, 0, NULL))
		return error(NULL, user_status);
	
	try 
		{
		while (transaction = next) 
			{
			next = transaction->tra_sibling;
			check_database(threadData, transaction->tra_attachment);
			//tdbb->tdbb_status_vector = ptr;
			threadData.setTransaction(transaction);
			TRA_commit(threadData, transaction, retaining_flag);
			DBB dbb = threadData.getDatabase(); //tdbb->tdbb_database;
			dbb->decrementUseCount();
			}

		return return_success(threadData);
		}
	catch (OSRIException& exception) 
		{
		DBB dbb = threadData.getDatabase(); //tdbb->tdbb_database;
		dbb->decrementUseCount();
		return error(&exception, user_status);
		}
}


static STR copy_string(const TEXT* ptr, int length)
{
/**************************************
 *
 *	c o p y _ s t r i n g
 *
 **************************************
 *
 * Functional description
 *	Copy a string to a string block.
 *
 **************************************/
	DBB dbb = get_dbb();
	STR string = FB_NEW_RPT(*dbb->dbb_permanent, length) str();

	string->str_length = length;
	MOVE_FAST(ptr, string->str_data, length);

	return string;
}


static bool drop_files(const File* file)
{
/**************************************
 *
 *	d r o p _ f i l e s
 *
 **************************************
 *
 * Functional description
 *	drop a linked list of files
 *
 **************************************/
	ISC_STATUS_ARRAY status;

	status[1] = FB_SUCCESS;

	for (; file; file = file->fil_next)
		if (unlink(file->fil_string))
			{
			IBERR_build_status(status, isc_io_error,
							   isc_arg_string, "unlink",
							   isc_arg_string,
							   ERR_cstring(file->fil_string),
							   isc_arg_gds, isc_io_delete_err,
							   SYS_ERR, errno,
							   0);
			DBB dbb = get_dbb();
			gds__log_status(dbb->dbb_file->fil_string, status);
			}

	return status[1] ? true : false;
}


static Transaction* find_transaction(thread_db* tdbb, Transaction* transaction, ISC_STATUS error_code)
{
/**************************************
 *
 *	f i n d _ t r a n s a c t i o n
 *
 **************************************
 *
 * Functional description
 *	Find the element of a possible multiple database transaction
 *	that corresponds to the current database.
 *
 **************************************/

	///SET_TDBB(tdbb);

	if (!transaction) // || MemoryPool::blk_type(transaction) != type_tra)
		ERR_post(isc_bad_trans_handle, 0);

	for (; transaction; transaction = transaction->tra_sibling)
		if (transaction->tra_attachment == tdbb->tdbb_attachment) 
			{
			tdbb->tdbb_transaction = transaction;
			return transaction;
			}

	ERR_post(error_code, 0);
	
	return NULL;		/* Added to remove compiler warnings */
}


static ISC_STATUS error(OSRIException *exception, ISC_STATUS* user_status)
{
/**************************************
 *
 *	e r r o r
 *
 **************************************
 *
 * Functional description
 *	An error returned has been trapped.  Return a status code.
 *
 **************************************/
	thread_db* tdbb = get_thread_data();

	/* Decrement count of active threads in database */
	
	DBB dbb = tdbb->tdbb_database;
	
	if (dbb) 
		dbb->decrementUseCount();

	if (exception)
		exception->copy (user_status);
		
	JRD_restore_context();

	/* This is debugging code which is meant to verify that
	   the database use count is cleared on exit from the
	   engine. Database shutdown cannot succeed if the database
	   use count is erroneously left set. */

#if (defined DEV_BUILD && !defined MULTI_THREAD)
	if (dbb && dbb->dbb_use_count && !(dbb->dbb_flags & DBB_security_db)) 
		{
		ISC_STATUS *p;

		dbb->dbb_use_count = 0;
		p = user_status;
		*p++ = isc_arg_gds;
		*p++ = isc_random;
		*p++ = isc_arg_string;
		*p++ = (ISC_STATUS) "database use count set on error return";
		*p = isc_arg_end;
		}
#endif

	return user_status[1];
}


static void find_intl_charset(thread_db* tdbb, Attachment* attachment, const DPB* options)
{
/**************************************
 *
 *	f i n d _ i n t l _ c h a r s e t
 *
 **************************************
 *
 * Functional description
 *	Attachment has declared it's prefered character set
 *	as part of LC_CTYPE, passed over with the attachment
 *	block.  Now let's resolve that to an internal subtype id.
 *
 **************************************/
	SSHORT len;

	SET_TDBB(tdbb);

	if (!options->dpb_lc_ctype || (len = strlen(options->dpb_lc_ctype)) == 0) 
		{
		/* No declaration of character set, act like 3.x Interbase */
		attachment->att_charset = DEFAULT_ATTACHMENT_CHARSET;
		return;
		}

	SSHORT id;
	ISC_STATUS_ARRAY local_status;

	if (MET_get_char_subtype(tdbb, &id, options->dpb_lc_ctype, len) &&
		  INTL_defined_type(tdbb, local_status, id) && (id != CS_BINARY))
		attachment->att_charset = id;
	else
		{
		/* Report an error - we can't do what user has requested */
		ERR_post(isc_bad_dpb_content,
				isc_arg_gds, isc_charset_not_found,
				isc_arg_string, ERR_cstring(options->dpb_lc_ctype),
				0);
		}
}


static void get_options(thread_db* tdbb,
						const UCHAR*	dpb,
						USHORT	dpb_length,
						TEXT**	scratch,
					    ULONG	buf_size,
						DPB*	options)
{
/**************************************
 *
 *	g e t _ o p t i o n s
 *
 **************************************
 *
 * Functional description
 *	Parse database parameter block picking up options and things.
 *
 **************************************/
	USHORT l;
	SSHORT num_old_files = 0;

	DBB dbb = tdbb->tdbb_database;
	MOVE_CLEAR(options, (SLONG) sizeof(struct dpb));

	options->dpb_buffers = dbb->cacheDefault;
	options->dpb_sweep_interval = -1;
	options->dpb_wal_grp_cmt_wait = -1;
	options->dpb_overwrite = TRUE;
	options->dpb_sql_dialect = 99;
	invalid_client_SQL_dialect = FALSE;
	const UCHAR* p = dpb;
	const UCHAR* const end_dpb = p + dpb_length;

	if ((dpb == NULL) && (dpb_length > 0))
		ERR_post(isc_bad_dpb_form, 0);

	if (p < end_dpb && *p++ != isc_dpb_version1)
		ERR_post(isc_bad_dpb_form, isc_arg_gds, isc_wrodpbver, 0);

	while (p < end_dpb && buf_size)
		{
		switch (*p++)
			{
			case isc_dpb_working_directory:
				{
				// CLASSIC have no thread data. Init to zero.
				char* t_data = 0;
				options->dpb_working_directory =
					get_string_parameter(&p, scratch, &buf_size);

				THD_getspecific_data((void **) &t_data);

				/*
				Null value for working_directory implies remote database. So get
				the users HOME directory
				*/
#ifndef WIN_NT
				if (!options->dpb_working_directory[0]) 
					{
					struct passwd *passwd = NULL;

					if (t_data)
						passwd = getpwnam(t_data);
						
					if (passwd) 
						{
						l = strlen(passwd->pw_dir) + 1;
						if (l <= buf_size)
							strcpy(*scratch, passwd->pw_dir);
						else
							{
							**scratch = 0;
							l = buf_size;
							}
						}
					else 
						{		/*No home dir for this users here. Default to server dir */
						**scratch = 0;
#ifdef HAVE_GETCWD
						if (getcwd(*scratch, MIN(MAXPATHLEN, buf_size)))
#else
						if (getwd(*scratch))
#endif
							l = strlen(*scratch) + 1;
						else
							l = buf_size;	
						}
					options->dpb_working_directory = *scratch;
					*scratch += l;
					buf_size -= l;
					}
#endif
				if (t_data)
					{
					free(t_data);
					t_data = NULL;
					}
					
				/* Null out the thread local data so that further references will fail */
				
				//THD_putspecific_data(0);
				}
				break;

			case isc_dpb_set_page_buffers:
				options->dpb_page_buffers = get_parameter(&p);
				
				if (options->dpb_page_buffers &&
					(options->dpb_page_buffers < MIN_PAGE_BUFFERS ||
					options->dpb_page_buffers > MAX_PAGE_BUFFERS))
					ERR_post(isc_bad_dpb_content, 0);

				options->dpb_set_page_buffers = TRUE;
				break;

			case isc_dpb_num_buffers:
				options->dpb_buffers = get_parameter(&p);
				
				if (options->dpb_buffers < 10)
					ERR_post(isc_bad_dpb_content, 0);
				break;

			case isc_dpb_page_size:
				options->dpb_page_size = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_debug:
				options->dpb_debug = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_sweep:
				options->dpb_sweep = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_sweep_interval:
				options->dpb_sweep_interval = get_parameter(&p);
				break;

			case isc_dpb_verify:
				options->dpb_verify = (USHORT) get_parameter(&p);
				if (options->dpb_verify & isc_dpb_ignore)
					dbb->dbb_flags |= DBB_damaged;
				break;

			case isc_dpb_trace:
				options->dpb_trace = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_damaged:
				if (get_parameter(&p) & 1)
					dbb->dbb_flags |= DBB_damaged;
				break;

			case isc_dpb_enable_journal:
				options->dpb_journal = get_string_parameter(&p, scratch, &buf_size);
				break;

			case isc_dpb_wal_backup_dir:
				options->dpb_wal_backup_dir = get_string_parameter(&p, scratch, &buf_size);
				break;

			case isc_dpb_drop_walfile:
				options->dpb_wal_action = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_old_dump_id:
				options->dpb_old_dump_id = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_online_dump:
				options->dpb_online_dump = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_old_file_size:
				options->dpb_old_file_size = get_parameter(&p);
				break;

			case isc_dpb_old_num_files:
				options->dpb_old_num_files = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_old_start_page:
				options->dpb_old_start_page = get_parameter(&p);
				break;

			case isc_dpb_old_start_seqno:
				options->dpb_old_start_seqno = get_parameter(&p);
				break;

			case isc_dpb_old_start_file:
				options->dpb_old_start_file = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_old_file:
				if (num_old_files >= MAX_OLD_FILES)
					ERR_post(isc_num_old_files, 0);

				options->dpb_old_file [num_old_files] =
					get_string_parameter(&p, scratch, &buf_size);
				num_old_files++;
				break;

			case isc_dpb_wal_chkptlen:
				options->dpb_wal_chkpt_len = get_parameter(&p);
				break;

			case isc_dpb_wal_numbufs:
				options->dpb_wal_num_bufs = (SSHORT) get_parameter(&p);
				break;

			case isc_dpb_wal_bufsize:
				options->dpb_wal_bufsize = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_wal_grp_cmt_wait:
				options->dpb_wal_grp_cmt_wait = get_parameter(&p);
				break;

			case isc_dpb_dbkey_scope:
				options->dpb_dbkey_scope = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_sys_user_name:
				options->dpb_sys_user_name = get_string_parameter(&p, scratch, &buf_size);
				break;

			case isc_dpb_sql_role_name:
				options->dpb_role_name = get_string_parameter(&p, scratch, &buf_size);
				break;

			case isc_dpb_user_name:
				options->dpb_user_name = get_string_parameter(&p, scratch, &buf_size);
				break;

			case isc_dpb_password:
				options->dpb_password = get_string_parameter(&p, scratch, &buf_size);
				break;

			case isc_dpb_password_enc:
				options->dpb_password_enc = get_string_parameter(&p, scratch, &buf_size);
				break;

			case isc_dpb_encrypt_key:
#ifdef ISC_DATABASE_ENCRYPTION
				options->dpb_key = get_string_parameter(&p, scratch, &buf_size);
#else
				/* Just in case there WAS a customer using this unsupported
				* feature - post an error when they try to access it in 4.0
				*/
				ERR_post(isc_uns_ext, isc_arg_gds, isc_random,
						isc_arg_string, "Encryption not supported", 0);
#endif
				break;

			case isc_dpb_no_garbage_collect:
				options->dpb_no_garbage = TRUE;
				l = *p++;
				p += l;
				break;

			case isc_dpb_disable_journal:
				options->dpb_disable = TRUE;
				l = *p++;
				p += l;
				break;

			case isc_dpb_activate_shadow:
				options->dpb_activate_shadow = true;
				l = *p++;
				p += l;
				break;

			case isc_dpb_delete_shadow:
				options->dpb_delete_shadow = true;
				l = *p++;
				p += l;
				break;

			case isc_dpb_force_write:
				options->dpb_set_force_write = TRUE;
				options->dpb_force_write = (SSHORT) get_parameter(&p);
				break;

			case isc_dpb_begin_log:
				options->dpb_log = get_string_parameter(&p, scratch, &buf_size);
				break;

			case isc_dpb_quit_log:
				options->dpb_quit_log = TRUE;
				l = *p++;
				p += l;
				break;

			case isc_dpb_no_reserve:
				options->dpb_set_no_reserve = TRUE;
				options->dpb_no_reserve = (UCHAR) get_parameter(&p);
				break;

			case isc_dpb_interp:
				options->dpb_interp = (SSHORT) get_parameter(&p);
				break;

			case isc_dpb_lc_messages:
				options->dpb_lc_messages = get_string_parameter(&p, scratch, &buf_size);
				break;

			case isc_dpb_lc_ctype:
				options->dpb_lc_ctype = get_string_parameter(&p, scratch, &buf_size);
				break;

			case isc_dpb_shutdown:
				options->dpb_shutdown = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_shutdown_delay:
				options->dpb_shutdown_delay = (SSHORT) get_parameter(&p);
				break;

			case isc_dpb_online:
				options->dpb_online = TRUE;
				l = *p++;
				p += l;
				break;

			case isc_dpb_reserved:
				{
				const TEXT* single = get_string_parameter(&p, scratch, &buf_size);
				
		    	if (single && !strcmp(single, "YES"))
					options->dpb_single_user = TRUE;
				break;
				}

			case isc_dpb_overwrite:
				options->dpb_overwrite = (BOOLEAN) get_parameter(&p);
				break;

			case isc_dpb_sec_attach:
				options->dpb_sec_attach = (BOOLEAN) get_parameter(&p);
				options->dpb_buffers = 50;
				dbb->dbb_flags |= DBB_security_db;
				break;

			case isc_dpb_gbak_attach:
				options->dpb_gbak_attach = get_string_parameter(&p, scratch, &buf_size);
				break;

			case isc_dpb_gstat_attach:
				options->dpb_gstat_attach = TRUE;
				l = *p++;
				p += l;
				break;

			case isc_dpb_gfix_attach:
				options->dpb_gfix_attach = TRUE;
				l = *p++;
				p += l;
				break;

			case isc_dpb_disable_wal:
				options->dpb_disable_wal = TRUE;
				l = *p++;
				p += l;
				break;

			case isc_dpb_connect_timeout:
				options->dpb_connect_timeout = get_parameter(&p);
				break;

			case isc_dpb_dummy_packet_interval:
				options->dpb_dummy_packet_interval = get_parameter(&p);
				break;

			case isc_dpb_sql_dialect:
				options->dpb_sql_dialect = (USHORT) get_parameter(&p);
				if (options->dpb_sql_dialect > SQL_DIALECT_V6)
						invalid_client_SQL_dialect = TRUE;
				break;

			case isc_dpb_set_db_sql_dialect:
				options->dpb_set_db_sql_dialect = (USHORT) get_parameter(&p);
				break;

			case isc_dpb_set_db_readonly:
				options->dpb_set_db_readonly = TRUE;
				options->dpb_db_readonly = (SSHORT) get_parameter(&p);
				break;

			case isc_dpb_set_db_charset:
				options->dpb_set_db_charset = get_string_parameter (&p, scratch, &buf_size);
				break;

			default:
				l = *p++;
				p += l;
			}
		}

	if (p != end_dpb)
		ERR_post(isc_bad_dpb_form, 0);

}


static SLONG get_parameter(const UCHAR** ptr)
{
/**************************************
 *
 *	g e t _ p a r a m e t e r
 *
 **************************************
 *
 * Functional description
 *	Pick up a VAX format parameter from a parameter block, including the
 *	length byte.
 *
 **************************************/
	const SSHORT l = *(*ptr)++;
	const SLONG parameter = gds__vax_integer(*ptr, l);
	*ptr += l;

	return parameter;
}


static TEXT* get_string_parameter(const UCHAR** dpb_ptr, TEXT** opt_ptr, ULONG* buf_avail)
{
/**************************************
 *
 *	g e t _ s t r i n g _ p a r a m e t e r
 *
 **************************************
 *
 * Functional description
 *	Pick up a string valued parameter, copy it to a running temp,
 *	and return pointer to copied string.
 *
 **************************************/

	if (!*buf_avail)  /* Because "l" may be zero but the NULL term still is set. */
		return 0;

	TEXT* opt = *opt_ptr;
	TEXT* const rc = *opt_ptr; // On success, we return the incoming position.
	const UCHAR* dpb = *dpb_ptr;
	USHORT l = *(dpb++);
	
	if (l)
		{
		if (l >= *buf_avail) /* >= to count the NULL term. */
			{
			*buf_avail = 0;
			return 0;
			}
			
		*buf_avail -= l;
		
		do
			*opt++ = *dpb++;
		while (--l);
		}

	--*buf_avail;
	*opt++ = 0;
	*dpb_ptr = dpb;
	*opt_ptr = opt;

	return rc;
}


static ISC_STATUS handle_error(ISC_STATUS* user_status, ISC_STATUS code, thread_db* tdbb)
{
/**************************************
 *
 *	h a n d l e _ e r r o r
 *
 **************************************
 *
 * Functional description
 *	An invalid handle has been passed in.  If there is a user status
 *	vector, make it reflect the error.  If not, emulate the routine
 *	"error" and abort.
 *
 **************************************/
	if (tdbb)
		JRD_restore_context();

	ISC_STATUS* vector = user_status;
	*vector++ = isc_arg_gds;
	*vector++ = code;
	*vector = isc_arg_end;

	return code;
}


#if defined (WIN_NT) && !defined(SERVER_SHUTDOWN)
static BOOLEAN handler_NT(SSHORT controlAction)
{
/**************************************
 *
 *      h a n d l e r _ N T
 *
 **************************************
 *
 * Functional description
 *	For some actions, get NT to issue a popup asking
 *	the user to delay.
 *
 **************************************/

	switch (controlAction) {
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		return TRUE;			/* NT will issue popup */

	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
		return FALSE;			/* let it go */
	}
/* So, what are we to return here?! */
	return FALSE;				/* let it go */
}
#endif


static DBB init(thread_db* tdbb,
				ISC_STATUS*	user_status,
				const TEXT*	expanded_filename,
				bool attach_flag, 
				ConfObject *configObject)
{
/**************************************
 *
 *		i n i t
 *
 **************************************
 *
 * Functional description
 *	Initialize for database access.  First call from both CREATE and
 *	OPEN.
 *

	/* If this is the first time through, initialize local mutexes and set
	   up a cleanup handler.  Regardless, then lock the database mutex. */
	   
	Sync sync(&init_mutex, "init");
	sync.lock(Exclusive);
	
	if (!initialized) 
		{
		THD_INIT;

#ifdef PLUGIN_MANAGER
		PluginManager::load_engine_plugins();
#endif

		if (!initialized) 
			{
			gds__register_cleanup(cleanup, 0);
			initialized = true;
#if defined (WIN_NT) && !defined(SERVER_SHUTDOWN)
			setup_NT_handlers();
#endif
			}
		}
		
	sync.unlock();

	/* Check to see if the database is already actively attached */

	DBB dbb = databaseManager.getDatabase (expanded_filename, configObject);

	if (dbb->initialized)	
		{
		if (attach_flag)
			{
			dbb->isReady (true);
			return dbb;
			}

#ifdef SHARED_CACHE
		dbb->syncExistence.unlock();			
#endif
		dbb->release();
		
		return NULL;
		}

	/* Clean up temporary DBB */

	try 
		{
		dbb->initialized = true;
		tdbb->tdbb_database = dbb;
		ALL_init(tdbb);
		dbb->dbb_flags |= DBB_exclusive;
		dbb->dbb_sweep_interval = SWEEP_INTERVAL;

		/* Initialize a number of subsystems */

		TRA_init(tdbb);

		/* Lookup some external "hooks" */

#ifdef PLUGIN_MANAGER
		PluginManager::Plugin crypt_lib =
			PluginManager::getEnginePluginManager().findPlugin(CRYPT_IMAGE);

		if (crypt_lib) 
			{
			NAMESPACE::string encrypt_entrypoint(ENCRYPT);
			NAMESPACE::string decrypt_entrypoint(DECRYPT);
			dbb->dbb_encrypt = (dbb::crypt_routine) crypt_lib.lookupSymbol(encrypt_entrypoint);
			dbb->dbb_decrypt = (dbb::crypt_routine) crypt_lib.lookupSymbol(decrypt_entrypoint);
			}
#endif

		INTL_init(tdbb);
		
		if (attach_flag)
			dbb->makeReady();
		
		return dbb;
		}
	catch (OSRIException& exception) 
		{
		exception;		// for debugging purposes
		
		if (dbb)
			{
			databaseManager.shutdown(dbb);
#ifdef SHARED_CACHE
			dbb->syncExistence.unlock();
#endif
			dbb->release();
			}
			
		return NULL;
		}
}



static ISC_STATUS prepare(thread_db* tdbb,
					  Transaction* transaction,
					  ISC_STATUS* status_vector,
					  USHORT length,
					  const UCHAR*	msg)
{
/**************************************
 *
 *	p r e p a r e
 *
 **************************************
 *
 * Functional description
 *	Attempt to prepare a transaction.  If it fails at any point, return
 *	an error.
 *
 **************************************/
	SET_TDBB(tdbb);

	try 
		{
		for (; transaction; transaction = transaction->tra_sibling) 
			{
			check_database(tdbb, transaction->tra_attachment);
			//tdbb->tdbb_status_vector = status_vector;
			TRA_prepare(tdbb, transaction, length, msg);
			DBB dbb = tdbb->tdbb_database;
			dbb->decrementUseCount();
			}
		}
	catch (OSRIException& exception) 
		{
		exception;
		DBB dbb = tdbb->tdbb_database;
		dbb->decrementUseCount();
		
		return status_vector[1];
		}

	return FB_SUCCESS;
}


static void release_attachment(thread_db* tdbb, Attachment* attachment)
{
/**************************************
 *
 *	r e l e a s e _ a t t a c h m e n t
 *
 **************************************
 *
 * Functional description
 *	Disconnect attachment block from database block.
 *	NOTE:  This routine assumes that upon entry,
 *	mutex DBB_MUTX_init_fini will be locked.
 *	Before exiting, there is a handoff from this
 *	mutex to the databases_mutex mutex.  Upon exit,
 *	that mutex remains locked.  It is the
 *	responsibility of the caller to unlock it.
 *
 **************************************/

	if (!attachment)
		return;

	DBB  dbb  = tdbb->tdbb_database;
	attachment->shutdown(tdbb);
	dbb->deleteAttachment (attachment);
}


static ISC_STATUS return_success(thread_db* tdbb)
{
/**************************************
 *
 *	r e t u r n _ s u c c e s s
 *
 **************************************
 *
 * Functional description
 *	Set up status vector to reflect successful execution.
 *
 **************************************/
	SET_TDBB(tdbb);

	/* Decrement count of active threads in database */

	DBB dbb = tdbb->tdbb_database;
	
	if (dbb)
		dbb->decrementUseCount();

	ISC_STATUS* const user_status = tdbb->tdbb_status_vector;
	ISC_STATUS* p = user_status;

	/* If the status vector has not been initialized, then
	   initilalize the status vector to indicate success.
	   Else pass the status vector along at it stands.  */
   
	if (p[0] != isc_arg_gds ||
		p[1] != FB_SUCCESS ||
		(p[2] != isc_arg_end && p[2] != isc_arg_gds &&
		 p[2] != isc_arg_warning))
		{
		*p++ = isc_arg_gds;
		*p++ = FB_SUCCESS;
		*p = isc_arg_end;
		}

	JRD_restore_context();

	/* This is debugging code which is meant to verify that
	   the database use count is cleared on exit from the
	   engine. Database shutdown cannot succeed if the database
	   use count is erroneously left set. */

#if (defined DEV_BUILD && !defined MULTI_THREAD)
	if (dbb && dbb->dbb_use_count && !(dbb->dbb_flags & DBB_security_db)) 
		{
		dbb->dbb_use_count = 0;
		p = user_status;
		*p++ = isc_arg_gds;
		*p++ = isc_random;
		*p++ = isc_arg_string;
		*p++ = (ISC_STATUS) "database use count set on success return";
		*p = isc_arg_end;
		}
#endif

	return user_status[1];
}


static BOOLEAN rollback(thread_db* tdbb,
						Transaction* next,
						ISC_STATUS* status_vector,
						const bool retaining_flag)
{
/**************************************
 *
 *	r o l l b a c k
 *
 **************************************
 *
 * Functional description
 *	Abort a transaction.
 *
 **************************************/
	Transaction* transaction;
	ISC_STATUS_ARRAY local_status;

	SET_TDBB(tdbb);

	while (transaction = next)
		{
		next = transaction->tra_sibling;
		check_database(tdbb, transaction->tra_attachment);

		try 
			{
			//tdbb->tdbb_status_vector = status_vector;
			TRA_rollback(tdbb, transaction, retaining_flag);
			DBB dbb = tdbb->tdbb_database;
			dbb->decrementUseCount();
			}
		catch (OSRIException& exception) 
			{
			exception.copy (status_vector);
			status_vector = local_status;
			DBB dbb = tdbb->tdbb_database;
			dbb->decrementUseCount();
			continue;
			}
		}

	return (status_vector == local_status);
}


#if defined (WIN_NT) && !defined(SERVER_SHUTDOWN)
static void setup_NT_handlers()
{
/**************************************
 *
 *      s e t u p _ N T _ h a n d l e r s
 *
 **************************************
 *
 * Functional description
 *      Set up Windows NT console control handlers for
 *      things that can happen.   The handler used for
 *      all cases, handler_NT(), will flush and close
 *      any open database files.
 *
 **************************************/

	SetConsoleCtrlHandler((PHANDLER_ROUTINE) handler_NT, TRUE);
}
#endif


static void shutdown_database(thread_db* tdbb, const bool release_pools)
{
/**************************************
 *
 *	s h u t d o w n _ d a t a b a s e
 *
 **************************************
 *
 * Functional description
 *	Shutdown physical database environment.
 *	NOTE:  This routine assumes that upon entry,
 *	mutex databases_mutex will be locked.
 *
 **************************************/

	Database *dbb = tdbb->tdbb_database;
	
	if (databaseManager.shutdown(dbb))
		dbb->shutdown(tdbb);

	if (release_pools) 
		{
		dbb->release();
		tdbb->tdbb_database = NULL;
		}
}


static void strip_quotes(const TEXT* in, TEXT* out)
{
/**************************************
 *
 *	s t r i p _ q u o t e s
 *
 **************************************
 *
 * Functional description
 *	Get rid of quotes around strings
 *
 **************************************/
	if (!in || !*in) {
		*out = 0;
		return;
	}

	TEXT quote = 0;
/* Skip any initial quote */
	if ((*in == DBL_QUOTE) || (*in == SINGLE_QUOTE))
		quote = *in++;
	const TEXT* p = in;

/* Now copy characters until we see the same quote or EOS */
	while (*p && (*p != quote)) {
		*out++ = *p++;
	}
	*out = 0;
}

#ifdef DEADCODE
void JRD_set_cache_default(ULONG* num_ptr)
{
/**************************************
 *
 *	J R D _ s e t _ c a c h e _ d e f a u l t
 *
 **************************************
 *
 * Functional description
 *	Set the number of cache pages to use for each
 *	database, but don't go less than MIN_PAGE_BUFFERS and
 *      more than MAX_PAGE_BUFFERS.
 *	Currently MIN_PAGE_BUFFERS = 50L,
 *		  MAX_PAGE_BUFFERS = 65535L.
 *
 **************************************/

	if (*num_ptr < MIN_PAGE_BUFFERS)
		*num_ptr = MIN_PAGE_BUFFERS;
	if (*num_ptr > MAX_PAGE_BUFFERS)
		*num_ptr = MAX_PAGE_BUFFERS;
	JRD_cache_default = *num_ptr;
}
#endif // DEADCODE

#ifdef SERVER_SHUTDOWN
#ifdef OBSOLETE
ISC_STATUS number_of_attachments(TEXT* const buf, USHORT buf_len, USHORT flag,
						USHORT* atts, USHORT* dbs, TEXT** returned_buf)
{
/**************************************
 *
 *	J R D _ n u m _ a t t a c h m e n t s
 *
 **************************************
 *
 * Functional description
 *	Count the number of active databases and
 *	attachments.  If flag is set then put
 *	what it says into buf, if it fits. If it does not fit
 *	then allocate local buffer, put info into there, and
 *	return pointer to caller (in this case a caller must
 *	release memory allocated for local buffer).
 *
 **************************************/
	USHORT num_dbs = 0;
	USHORT num_att = 0;
	USHORT total = 0;
	ULONG drive_mask = 0L;
	DBF dbf = NULL, dbfp = NULL;

/* protect against NULL value for buf */

	TEXT* lbuf = buf;
	if (!lbuf)
		buf_len = 0;

#ifdef WIN_NT
/* Check that the buffer is big enough for the requested
 * information.  If not, unset the flag
 */

	if (flag == JRD_info_drivemask) {
		if (buf_len < sizeof(ULONG)) {
		    lbuf = (TEXT*) gds__alloc((SLONG) (sizeof(ULONG)));
			if (!lbuf)
				flag = 0;
		}
	}
#endif

	THREAD_ENTER;

	/* Zip through the list of databases and count the number of local
	 * connections.  If buf is not NULL then copy all the database names
	 * that will fit into it. */

	for (Database* dbb = databaseManager.databases; dbb; dbb = dbb->dbb_next) 
		{
#ifdef WIN_NT
		File* files;
		struct DirectoryList* dirs;
		struct SortContext* scb;
		struct SortWorkFile* sfb;

		/* Get drive letters for db files */

		if (flag == JRD_info_drivemask)
			for (files = dbb->dbb_file; files; files = files->fil_next)
				ExtractDriveLetter(files->fil_string, &drive_mask);
#endif

		if (!(dbb->dbb_flags & (DBB_bugcheck | DBB_not_in_use | DBB_security_db)) &&
			!(dbb->dbb_ast_flags & DBB_shutdown
			  && dbb->dbb_ast_flags & DBB_shutdown_locks)) {
			num_dbs++;
			if (flag == JRD_info_dbnames) 
				{
				if (dbfp == NULL) 
					{
					dbfp = (DBF) gds__alloc((sizeof(struct dbf) +  dbb->dbb_filename.length()));
					dbf = dbfp;
					}
				else 
					{
					dbfp->dbf_next = (DBF)
						gds__alloc((sizeof(struct dbf) + dbb->dbb_filename.length()));
					dbfp = dbfp->dbf_next;
					}
				if (dbfp) 
					{
					dbfp->dbf_length = dbb->dbb_filename.length();
					dbfp->dbf_next = NULL;
					MOVE_FAST(dbb->dbb_filename, dbfp->dbf_data,
							  dbfp->dbf_length);
					total += sizeof(USHORT) + dbfp->dbf_length;
					}
				else
					flag = 0;
				}

			for (const Attachment* attach = dbb->dbb_attachments; attach; attach = attach->att_next)
				{
				num_att++;

#ifdef WIN_NT
				/* Get drive letters for temp directories */

				if (flag == JRD_info_drivemask) 
					{
					MDLS* ptr = DLS_get_access(dbb->configuration);
					for (dirs = ptr->mdls_dls; dirs; dirs = dirs->dls_next) 
						ExtractDriveLetter(dirs->dls_directory, &drive_mask);
					}

				/* Get drive letters for sort files */

				if (flag == JRD_info_drivemask)
					for (scb = attach->att_active_sorts; scb; scb = scb->scb_next)
						for (sfb = scb->scb_sfb; sfb; sfb = sfb->sfb_next)
							ExtractDriveLetter(sfb->sfb_file_name, &drive_mask);
#endif
				}
		}
	}

	THREAD_EXIT;

	*atts = num_att;
	*dbs = num_dbs;

	if (dbf)
	{
		if (flag == JRD_info_dbnames)
		{
			if (buf_len < (sizeof(USHORT) + total))
			{
				lbuf = (TEXT *) gds__alloc((SLONG) (sizeof(USHORT) + total));
			}
			TEXT* lbufp = lbuf;
			if (lbufp)
			{
				/*  Put db info into buffer. Format is as follows:

				   number of dbases sizeof (USHORT)
				   1st db name length   sizeof (USHORT)
				   1st db name      sizeof (TEXT) * length
				   2nd db name length
				   2nd db name
				   ...
				   last db name length
				   last db name
				 */

				lbufp += sizeof(USHORT);
				total = 0;
				for (dbfp = dbf; dbfp; dbfp = dbfp->dbf_next) {
					*lbufp++ = (TEXT) dbfp->dbf_length;
					*lbufp++ = dbfp->dbf_length >> 8;
					MOVE_FAST(dbfp->dbf_data, lbufp, dbfp->dbf_length);
					lbufp += dbfp->dbf_length;
					total++;
				}
				fb_assert(total == num_dbs);
				lbufp = lbuf;
				*lbufp++ = (TEXT) total;
				*lbufp++ = total >> 8;
			}
		}

		for (dbfp = dbf; dbfp;) {
			DBF x = dbfp->dbf_next;
			gds__free(dbfp);
			dbfp = x;
		}
	}

#ifdef WIN_NT
	if (flag == JRD_info_drivemask)
		*(ULONG *) lbuf = drive_mask;
#endif

// CVC: Apparently, the original condition will leak memory, because flag
// may be JRD_info_drivemask and memory could be allocated for that purpose,
// as few as sizeof(ULONG), but a leak is a leak! I added the ifdef below.
	if (num_dbs == 0)
	{
#ifdef WIN_NT
		if (flag == JRD_info_drivemask && lbuf != buf)
		    gds__free(lbuf);
#endif
		lbuf = NULL;
	}

	*(returned_buf) = lbuf;

	return FB_SUCCESS;
}
#endif //OBSOLETE

#ifdef WIN_NT
static void ExtractDriveLetter(const TEXT* file_name, ULONG* drive_mask)
{
/**************************************
 *
 *	E x t r a c t D r i v e L e t t e r
 *
 **************************************
 *
 * Functional description
 *	Determine the drive letter of file_name
 *	and set the proper bit in the bit mask.
 *		bit 0 = drive A
 *		bit 1 = drive B and so on...
 *	This function is used to determine drive
 *	usage for use with Plug and Play for
 *	MS Windows 4.0.
 *
 **************************************/
	ULONG mask = 1;

	const SHORT shift = (*file_name - 'A');
	mask <<= shift;
	*drive_mask |= mask;
}
#endif

#ifdef OBSOLETE
ULONG JRD_shutdown_all()
{
/**************************************
 *
 *	J R D _ s h u t d o w n _ a l l
 *
 **************************************
 *
 * Functional description
 *	rollback every transaction,
 *	release every attachment,
 *	and shutdown every database.
 *
 **************************************/
	ISC_STATUS_ARRAY user_status;
	//struct tdbb thd_context;

	//struct tdbb* tdbb = set_thread_data(thd_context);
	ThreadData threadData (user_status);

	if (initialized) {
		JRD_SS_MUTEX_LOCK;
	}

	DBB dbb_next;
	for (DBB dbb = databases; dbb; dbb = dbb_next)
	{
		dbb_next = dbb->dbb_next;
		if (!(dbb->dbb_flags & (DBB_bugcheck | DBB_not_in_use | DBB_security_db)) &&
			!(dbb->dbb_ast_flags & DBB_shutdown &&
			  dbb->dbb_ast_flags & DBB_shutdown_locks))
		{
			Attachment* att_next;
			for (Attachment* attach = dbb->dbb_attachments; attach; attach = att_next)
			{
				att_next = attach->att_next;
				//threadData.setDatabase (dbb);
				threadData.setAttachment (attach);
				//tdbb->tdbb_attachment = attach;
				//tdbb->tdbb_request = NULL;
				//tdbb->tdbb_transaction = NULL;
				//tdbb->tdbb_default = NULL;

				dbb->incrementUseCount();

				/* purge_attachment below can do an ERR_post */

				//tdbb->tdbb_status_vector = user_status;

				try 
					{
					/* purge_attachment, rollback any open transactions */

					purge_attachment(threadData, user_status, attach, true);
					}	// try
				catch (OSRIException& exception) 
					{
					if (initialized) 
						JRD_SS_MUTEX_UNLOCK;
					return error(&exception, user_status);
					}
				}
			}
		}

	if (initialized) {
		JRD_SS_MUTEX_UNLOCK;
    }

	JRD_restore_context();

	return FB_SUCCESS;
}
#endif // OBSOLETE
#endif /* SERVER_SHUTDOWN */


static void purge_attachment(thread_db* tdbb,
							 ISC_STATUS* user_status,
							 Attachment* attachment,
							 const bool	force_flag)
{
/**************************************
 *
 *	p u r g e _ a t t a c h m e n t
 *
 **************************************
 *
 * Functional description
 *	Zap an attachment, shutting down the database
 *	if it is the last one.
 *	NOTE:  This routine assumes that upon entry,
 *	mutex databases_mutex will be locked.
 *
 **************************************/
 
	DBB dbb = attachment->att_database;

	if (!(dbb->dbb_flags & DBB_bugcheck)) 
		{
		/* Check for any pending transactions */

		int count = 0;
		Transaction* next;
		
		for (Transaction* transaction = attachment->att_transactions; transaction; transaction = next)
			{
			next = transaction->tra_next;
			
			if (transaction != attachment->att_dbkey_trans)
				{
				if (transaction->tra_flags & TRA_prepared ||
					dbb->dbb_ast_flags & DBB_shutdown ||
					attachment->att_flags & ATT_shutdown)
					TRA_release_transaction(tdbb, transaction);
				else if (force_flag)
					TRA_rollback(tdbb, transaction, false);
				else
					++count;
				}
			}

		if (count)
			ERR_post(isc_open_trans, isc_arg_number, count, 0);

		/* If there's a side transaction for db-key scope, get rid of it */

		Transaction* trans_dbk = attachment->att_dbkey_trans;
		
		if (trans_dbk)
			{
			attachment->att_dbkey_trans = NULL;
			
			if (dbb->dbb_ast_flags & DBB_shutdown || attachment->att_flags & ATT_shutdown)
				TRA_release_transaction(tdbb, trans_dbk);
			else
				TRA_commit(tdbb, trans_dbk, false);
			}

		SORT_shutdown(attachment);
		}

	/* Unlink attachment from database */

	release_attachment(tdbb, attachment);

	/* If there are still attachments, do a partial shutdown */

	if (dbb->dbb_attachments || (dbb->dbb_flags & DBB_being_opened))
		{
		// There are still attachments so do a partial shutdown

        // Both CMP_release() and SCL_release() do advance the pointer
		// before the deallocation.
		
		for (Request* request; request = attachment->att_requests;) 
			CMP_release(tdbb, request);
		
		for (SecurityClass* sec_class; sec_class = attachment->att_security_classes;)
			SCL_release(tdbb, sec_class);
		
		delete attachment;
		dbb->release();
		}
	else
		{
		delete attachment;
		shutdown_database(tdbb, true);
		}
	
}


#ifdef OBSOLETE
// verify_request_synchronization
//
// @brief Finds the sub-requests at the given level and replaces with it the
// original passed request (note the pointer by reference). If that specific
// sub-request is not found, throw the dreaded "request synchronization error".
// Notice that at this time, the calling function's "request" pointer has been
// set to null, so remember that if you write a debugging routine.
// This function replaced a chunk of code repeated four times.
//
// @param request The incoming, parent request to be replaced.
// @param level The level of the sub-request we need to find.

static void verify_request_synchronization(Request*& request, SSHORT level)
{
	if (level) 
		{
		const vec* vector = request->req_sub_requests;
		
		if (!vector || level >= vector->count() || !(request = (Request*)(*vector)[level]))
			ERR_post(isc_req_sync, 0);
		}
}
#endif

/**
  
 	verify_database_name
  
    @brief	Verify database name for open/create 
	against given in conf file list of available directories

    @param name
    @param status

 **/
static bool verify_database_name(const TEXT* name, ISC_STATUS* status)
{
	static TEXT securityNameBuffer[MAXPATHLEN];
	//static TEXT ExpandedSecurityNameBuffer[MAXPATHLEN];
	static JString expandedSecurityName;
	static SyncObject syncObject;
	Sync sync(&syncObject, "verify_database_name");

	sync.lock(Exclusive); // sync around possible static variable write
	
	if (!securityNameBuffer[0]) 
		{
		//SecurityDatabase::getPath(securityNameBuffer);
		//ISC_expand_filename(SecurityNameBuffer, 0, ExpandedSecurityNameBuffer);
		//expandedSecurityName = PathName::expandFilename (securityNameBuffer);
		}
	
    sync.unlock();
        
	//if (strcmp(securityNameBuffer, name) == 0)
	if (PathName::pathsEquivalent (securityNameBuffer, name) ||
		PathName::pathsEquivalent (expandedSecurityName, name))
		return true;
		
	if (strcmp(expandedSecurityName, name) == 0)
		return true;
	
	// Check for .conf
	
	if (!ISC_verify_database_access(name)) 
		/***
		ERR_post (isc_conf_access_denied,
				  isc_arg_string, "database",
				  isc_arg_string, name, 
				  isc_arg_end);
		***/
		{
		status[0] = isc_arg_gds;
		status[1] = isc_conf_access_denied;
		status[2] = isc_arg_string;
		// CVC: Using STATUS to hold pointer to literal string!
		status[3] = reinterpret_cast<ISC_STATUS>("database");
		status[4] = isc_arg_string;
		status[5] = reinterpret_cast<ISC_STATUS>(ERR_cstring(name));
		status[6] = isc_arg_end;
		return false;
		}
		
	return true;
}

