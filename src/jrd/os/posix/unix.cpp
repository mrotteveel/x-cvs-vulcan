/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		unix.cpp
 *	DESCRIPTION:	UNIX (BSD) specific physical IO
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
 * 2001.07.06 Sean Leyne - Code Cleanup, removed "#ifdef READONLY_DATABASE"
 *                         conditionals, as the engine now fully supports
 *                         readonly databases.
 *
 * 2002.10.27 Sean Leyne - Completed removal of "DELTA" port
 *
 */

#include "fbdev.h"
#include "../jrd/common.h"
#include "../jrd/ib_stdio.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_AIO_H
#include <aio.h>
#endif

#include "../jrd/jrd.h"
#include "../jrd/os/pio.h"
#include "../jrd/ods.h"
#include "../jrd/lck.h"
#include "../jrd/cch.h"
//#include "../jrd/y_ref.h"
#include "../jrd/ibase.h"
#include "gen/iberror.h"
#include "../jrd/all_proto.h"
#include "../jrd/cch_proto.h"
#include "../jrd/err_proto.h"
#include "../jrd/gds_proto.h"
#include "../jrd/isc_proto.h"
#include "../jrd/isc_f_proto.h"
#include "../jrd/os/isc_i_proto.h"

#include "../jrd/lck_proto.h"
#include "../jrd/mov_proto.h"
#include "../jrd/os/pio_proto.h"
#include "../jrd/thd_proto.h"

/* SUPERSERVER uses a mutex to allow atomic seek/read(write) sequences.
   SUPERSERVER_V2 uses "positioned" read (write) calls to avoid a seek
   and allow multiple threads to overlap database I/O. */
#if defined SUPERSERVER
#if (defined PREAD && defined PWRITE) || defined HAVE_AIO_H
#define PREAD_PWRITE
#endif
#endif


#ifndef SUPERSERVER
#undef THD_MUTEX_INIT
#undef THD_MUTEX_LOCK
#undef THD_MUTEX_UNLOCK
#undef THD_MUTEX_DESTROY

#define THD_MUTEX_INIT(mutx)
#define THD_MUTEX_LOCK(mutx)
#define THD_MUTEX_UNLOCK(mutx)
#define THD_MUTEX_DESTROY(mutx)
#endif

#define IO_RETRY	20

#ifdef O_SYNC
#define SYNC		O_SYNC
#endif

    /* Changed to not redfine SYNC if O_SYNC already exists
       they seem to be the same values anyway. MOD 13-07-2001 */
#if (!(defined SYNC) && (defined O_FSYNC))
#define SYNC		O_FSYNC
#endif

#ifdef O_DSYNC
#undef SYNC
#define SYNC		O_DSYNC
#endif

#ifndef SYNC
#define SYNC		0
#endif

#ifndef O_BINARY
#define O_BINARY	0
#endif

#ifdef SUPERSERVER
#define MASK		0600
#else
#define MASK		0666
#endif

static void close_marker_file(TEXT *);
static File* seek_file(File*, Bdb*, UINT64 *, ISC_STATUS *);
static File* setup_file(DBB, const TEXT*, USHORT, int);
static BOOLEAN unix_error(TEXT *, File*, ISC_STATUS, ISC_STATUS *);
#if defined PREAD_PWRITE && !(defined HAVE_PREAD && defined HAVE_PWRITE)
static SLONG pread(int, SCHAR *, SLONG, SLONG);
static SLONG pwrite(int, SCHAR *, SLONG, SLONG);
#endif
#ifdef SUPPORT_RAW_DEVICES
static BOOLEAN  raw_devices_check_file (const TEXT*);
static BOOLEAN  raw_devices_validate_database (int, const TEXT*, USHORT);
static int  raw_devices_unlink_database (const TEXT*);
#endif


int PIO_add_file(DBB dbb, File* main_file, const TEXT* file_name, SLONG start)
{
/**************************************
 *
 *	P I O _ a d d _ f i l e
 *
 **************************************
 *
 * Functional description
 *	Add a file to an existing database.  Return the sequence
 *	number of the new file.  If anything goes wrong, return a
 *	sequence of 0.
 *	NOTE:  This routine does not lock any mutexes on
 *	its own behalf.  It is assumed that mutexes will
 *	have been locked before entry.
 *
 **************************************/
	USHORT sequence;
	File* file, *new_file;

	if (!(new_file = PIO_create(dbb, file_name, strlen(file_name), FALSE)))
		return 0;

	new_file->fil_min_page = start;
	sequence = 1;

	for (file = main_file; file->fil_next; file = file->fil_next)
		++sequence;

	file->fil_max_page = start - 1;
	file->fil_next = new_file;

	return sequence;
}


void PIO_close(File* main_file)
{
/**************************************
 *
 *	P I O _ c l o s e
 *
 **************************************
 *
 * Functional description
 *	NOTE:  This routine does not lock any mutexes on
 *	its own behalf.  It is assumed that mutexes will
 *	have been locked before entry.
 *
 **************************************/
	File* file;

	if (main_file) {
		TEXT marker_filename[MAXPATHLEN];

		/* If a marker file is found for single user access, close it. */

		strcpy(marker_filename, main_file->fil_string);
		strcat(marker_filename, "_m");
		if (!access(marker_filename, F_OK))	/* Marker file exists. */
			close_marker_file(marker_filename);
	}

	for (file = main_file; file; file = file->fil_next) {
		if (file->fil_desc == -1)
			continue;			/* This really should be an error */
		if (file->fil_desc) {
			close(file->fil_desc);
			file->fil_desc = -1;
#ifndef PREAD_PWRITE
			THD_MUTEX_DESTROY(file->fil_mutex);
#endif
		}
	}
}


int PIO_connection(const TEXT* file_name, USHORT* file_length)
{
/**************************************
 *
 *	P I O _ c o n n e c t i o n
 *
 **************************************
 *
 * Functional description
 *	Analyze a file specification and determine whether a page/lock
 *	server is required and available.  If so, return a "connection"
 *	block.  If not, return NULL.
 *
 *	Note: The file name must have been expanded prior to this call.
 *
 **************************************/

	return 0;
}


File* PIO_create(DBB dbb, const TEXT* string, SSHORT length, BOOLEAN overwrite)
{
/**************************************
 *
 *	P I O _ c r e a t e
 *
 **************************************
 *
 * Functional description
 *	Create a new database file.
 *	NOTE:  This routine does not lock any mutexes on
 *	its own behalf.  It is assumed that mutexes will
 *	have been locked before entry.
 *
 **************************************/
	int desc, flag;
	File* file;
	TEXT expanded_name[256], temp[256];

	const TEXT* file_name = string;

	if (length) {
		MOVE_FAST(file_name, temp, length);
		temp[length] = 0;
		file_name = temp;
	}

#ifdef SUPERSERVER_V2
	flag =
		SYNC | O_RDWR | O_CREAT | (overwrite ? O_TRUNC : O_EXCL) | O_BINARY;
#else
#ifdef SUPPORT_RAW_DEVICES
	flag = O_RDWR |
			(raw_devices_check_file(file_name) ? 0 : O_CREAT) |
			(overwrite ? O_TRUNC : O_EXCL) |
			O_BINARY;
#else
	flag = O_RDWR | O_CREAT | (overwrite ? O_TRUNC : O_EXCL) | O_BINARY;
#endif
#endif

	if ((desc = open(file_name, flag, MASK)) == -1)
		ERR_post(isc_io_error,
				 isc_arg_string, "open O_CREAT",
				 isc_arg_cstring, length, ERR_string(string, length),
				 isc_arg_gds, isc_io_create_err, isc_arg_unix, errno, 0);

/* File open succeeded.  Now expand the file name. */

	length = PIO_expand(string, length, expanded_name);
	file = setup_file(dbb, expanded_name, length, desc);
	return file;
}


int PIO_expand(const TEXT* file_name, USHORT file_length, TEXT* expanded_name)
{
/**************************************
 *
 *	P I O _ e x p a n d
 *
 **************************************
 *
 * Functional description
 *	Fully expand a file name.  If the file doesn't exist, do something
 *	intelligent.
 *
 **************************************/

	return ISC_expand_filename(file_name, file_length, expanded_name);
}


void PIO_flush(File* main_file)
{
/**************************************
 *
 *	P I O _ f l u s h
 *
 **************************************
 *
 * Functional description
 *	Flush the operating system cache back to good, solid oxide.
 *
 **************************************/
	File* file;

/* Since all SUPERSERVER_V2 database and shadow I/O is synchronous, this
   is a no-op. */

#ifndef SUPERSERVER_V2
	for (file = main_file; file; file = file->fil_next) {
		if (file->fil_desc != -1) {	/* This really should be an error */
			THD_MUTEX_LOCK(file->fil_mutex);
			fsync(file->fil_desc);
			THD_MUTEX_UNLOCK(file->fil_mutex);
		}
	}
#endif
}


void PIO_force_write(File* file, USHORT flag)
{
/**************************************
 *
 *	P I O _ f o r c e _ w r i t e	( G E N E R I C )
 *
 **************************************
 *
 * Functional description
 *	Set (or clear) force write, if possible, for the database.
 *
 **************************************/
	int control;

/* Since all SUPERSERVER_V2 database and shadow I/O is synchronous, this
   is a no-op. */

#ifndef SUPERSERVER_V2
	control = (flag) ? SYNC : 0;

	if (fcntl(file->fil_desc, F_SETFL, control) == -1)
	{
		ERR_post(isc_io_error,
				 isc_arg_string, "fcntl SYNC",
				 isc_arg_cstring, file->fil_length,
				 ERR_string(file->fil_string, file->fil_length), isc_arg_gds,
				 isc_io_access_err, isc_arg_unix, errno, 0);
	}
	else 
	{
		if (flag) {
			file->fil_flags |= (FIL_force_write | FIL_force_write_init);
		} else {
			file->fil_flags &= ~FIL_force_write;
		}
	}
#endif
}


void PIO_header(DBB dbb, SCHAR * address, int length)
{
/**************************************
 *
 *	P I O _ h e a d e r
 *
 **************************************
 *
 * Functional description
 *	Read the page header.  This assumes that the file has not been
 *	repositioned since the file was originally mapped.
 *
 **************************************/
	File* file;
	SSHORT i;
	UINT64 bytes;

	file = dbb->dbb_file;

	ISC_inhibit();

	if (file->fil_desc == -1)
		unix_error("PIO_header", file, isc_io_read_err, 0);

	for (i = 0; i < IO_RETRY; i++) {
#ifndef PREAD_PWRITE
		THD_MUTEX_LOCK(file->fil_mutex);

		if ((lseek(file->fil_desc, LSEEK_OFFSET_CAST 0, 0)) == -1) {
			THD_MUTEX_UNLOCK(file->fil_mutex);
			unix_error("lseek", file, isc_io_read_err, 0);
		}
#endif
#ifdef ISC_DATABASE_ENCRYPTION
		if (dbb->dbb_encrypt_key) {
			SLONG spare_buffer[MAX_PAGE_SIZE / sizeof(SLONG)];

#ifdef PREAD_PWRITE
			if ((bytes = pread(file->fil_desc, spare_buffer, length, 0)) ==
				(UINT64) -1) {
#else
			if ((bytes = read(file->fil_desc, spare_buffer, length)) == 
				(UINT64) -1) {
				THD_MUTEX_UNLOCK(file->fil_mutex);
#endif
				if (SYSCALL_INTERRUPTED(errno))
					continue;
				unix_error("read", file, isc_io_read_err, 0);
			}

			(*dbb->dbb_decrypt) (dbb->dbb_encrypt_key->str_data,
								 spare_buffer, length, address);
		}
		else
#endif /* ISC_DATABASE_ENCRYPTION */
#ifdef PREAD_PWRITE
		if ((bytes = pread(file->fil_desc, address, length, 0)) == (UINT64) -1) {
#else
		if ((bytes = read(file->fil_desc, address, length)) == (UINT64) -1) {
			THD_MUTEX_UNLOCK(file->fil_mutex);
#endif
			if (SYSCALL_INTERRUPTED(errno))
				continue;
			unix_error("read", file, isc_io_read_err, 0);
		}
		else
			break;
	}

	if (i == IO_RETRY) {
		if (bytes == 0) {
#ifdef DEV_BUILD
			ib_fprintf(ib_stderr, "PIO_header: an empty page read!\n");
			ib_fflush(ib_stderr);
#endif
		}
		else {
#ifdef DEV_BUILD
			ib_fprintf(ib_stderr, "PIO_header: retry count exceeded\n");
			ib_fflush(ib_stderr);
#endif
			unix_error("read_retry", file, isc_io_read_err, 0);
		}
	}
#ifndef PREAD_PWRITE
	THD_MUTEX_UNLOCK(file->fil_mutex);
#endif
	ISC_enable();
}


SLONG PIO_max_alloc(DBB dbb)
{
/**************************************
 *
 *	P I O _ m a x _ a l l o c
 *
 **************************************
 *
 * Functional description
 *	Compute last physically allocated page of database.
 *
 **************************************/
	struct stat statistics;
	UINT64 length;
	File* file;

	for (file = dbb->dbb_file; file->fil_next; file = file->fil_next);

	if (file->fil_desc == -1) {
		ISC_inhibit();
		unix_error("fstat", file, isc_io_access_err, 0);
		return (0);
	}

	if (fstat(file->fil_desc, &statistics)) {
		ISC_inhibit();
		unix_error("fstat", file, isc_io_access_err, 0);
	}

	length = statistics.st_size;

/****
#ifndef sun
length = statistics.st_size;
#else
length = statistics.st_blocks * statistics.st_blksize;
#endif
****/

	return file->fil_min_page - file->fil_fudge +
		(length + dbb->dbb_page_size - 1) / dbb->dbb_page_size;
}


SLONG PIO_act_alloc(DBB dbb)
{
/**************************************
 *
 *	P I O _ a c t  _ a l l o c
 *
 **************************************
 *
 * Functional description
 *  Compute actual number of physically allocated pages of database.
 *
 **************************************/
	struct stat statistics;
	UINT64 length;
	File* file;
	ULONG tot_pages = 0;

/**
 **  Traverse the linked list of files and add up the number of pages
 **  in each file
 **/
	for (file = dbb->dbb_file; file != NULL; file = file->fil_next) {
		if (file->fil_desc == -1) {
			ISC_inhibit();
			unix_error("fstat", file, isc_io_access_err, 0);
			return (0);
		}

		if (fstat(file->fil_desc, &statistics)) {
			ISC_inhibit();
			unix_error("fstat", file, isc_io_access_err, 0);
		}

		length = statistics.st_size;

		tot_pages += (length + dbb->dbb_page_size - 1) / dbb->dbb_page_size;
	}

	return tot_pages;
}


File* PIO_open(DBB dbb,
			 const TEXT* string,
			 SSHORT trace_flag,
			 BLK connection, const TEXT* file_name)
{
/**************************************
 *
 *	P I O _ o p e n
 *
 **************************************
 *
 * Functional description
 *	Open a database file.  If a "connection" block is provided, use
 *	the connection to communication with a page/lock server.
 *
 **************************************/
	int desc, i, flag;

	const TEXT* ptr = (string) ? string : file_name;

#ifdef SUPERSERVER_V2
	flag = SYNC | O_RDWR | O_BINARY;
#else
	flag = O_RDWR | O_BINARY;
#endif

	for (i = 0; i < IO_RETRY; i++) 
		{
		if ((desc = open(ptr, flag)) != -1)
			break;
		if (!SYSCALL_INTERRUPTED(errno))
			break;
		}

	if (desc == -1)
		{
		/* Try opening the database file in ReadOnly mode. The database file could
		 * be on a RO medium (CD-ROM etc.). If this fileopen fails, return error.
		 */
		flag &= ~O_RDWR;
		flag |= O_RDONLY;
		
		if ((desc = open(ptr, flag)) == -1) 
			ERR_post(isc_io_error,
					 isc_arg_string, "open",
					 isc_arg_cstring, file_name,
					 isc_arg_gds, isc_io_open_err, 
					 isc_arg_unix, errno, 
					 0);
					 
		/* If this is the primary file, set DBB flag to indicate that it is
			* being opened ReadOnly. This flag will be used later to compare with
			* the Header Page flag setting to make sure that the database is set
			* ReadOnly.
			*/
			
		if (!dbb->dbb_file)
			dbb->dbb_flags |= DBB_being_opened_read_only;
		}

#ifdef SUPPORT_RAW_DEVICES
	/* At this point the file has successfully been opened in either RW or RO
	 * mode. Check if it is a special file (i.e. raw block device) and if a
	 * valid database is on it. If not, return an error.
	 */
	if (raw_devices_check_file(file_name)
		&& !raw_devices_validate_database(desc, file_name, file_length))
	{
		ERR_post (isc_io_error,
					isc_arg_string, "open",
					isc_arg_cstring, file_length,
						ERR_string (file_name, file_length),
					isc_arg_gds, isc_io_open_err,
					isc_arg_unix, ENOENT, 0);
	}
#endif /* SUPPORT_RAW_DEVICES */

	return setup_file(dbb, string, length, desc);
}


int PIO_read(File* file, Bdb* bdb, PAG page, ISC_STATUS * status_vector)
{
/**************************************
 *
 *	P I O _ r e a d
 *
 **************************************
 *
 * Functional description
 *	Read a data page.  Oh wow.
 *
 **************************************/
	DBB dbb;
	SSHORT i;
	UINT64 bytes, size, offset;

	ISC_inhibit();

	if (file->fil_desc == -1)
		return unix_error("read", file, isc_io_read_err, status_vector);

	dbb = bdb->bdb_dbb;
	size = dbb->dbb_page_size;

#ifdef ISC_DATABASE_ENCRYPTION
	if (dbb->dbb_encrypt_key) {
		SLONG spare_buffer[MAX_PAGE_SIZE / sizeof(SLONG)];

		for (i = 0; i < IO_RETRY; i++) {
			if (!(file = seek_file(file, bdb, &offset, status_vector)))
				return FALSE;
#ifdef PREAD_PWRITE
            if ((bytes = pread (file->fil_desc, spare_buffer, size, LSEEK_OFFSET_CAST offset)) == size) 
#else
			if ((bytes = read(file->fil_desc, spare_buffer, size)) == size)
#endif
			{
				(*dbb->dbb_decrypt) (dbb->dbb_encrypt_key->str_data,
									 spare_buffer, size, page);
				break;
			}
#ifndef PREAD_PWRITE
			THD_MUTEX_UNLOCK(file->fil_mutex);
#endif
			if (bytes == -1U && !SYSCALL_INTERRUPTED(errno))
				return unix_error("read", file, isc_io_read_err,
								  status_vector);
		}
	}
	else
#endif /* ISC_DATABASE_ENCRYPTION */
	{
		for (i = 0; i < IO_RETRY; i++) {
			if (!(file = seek_file(file, bdb, &offset, status_vector)))
				return FALSE;
#ifdef PREAD_PWRITE
			if ((bytes = pread(file->fil_desc, page, size, LSEEK_OFFSET_CAST offset)) == size)
				break;
#else
			if ((bytes = read(file->fil_desc, page, size)) == size)
				break;
			THD_MUTEX_UNLOCK(file->fil_mutex);
#endif
			if (bytes == -1U && !SYSCALL_INTERRUPTED(errno))
				return unix_error("read", file, isc_io_read_err,
								  status_vector);
		}
	}

#ifndef PREAD_PWRITE
	THD_MUTEX_UNLOCK(file->fil_mutex);
#endif

	if (i == IO_RETRY) {
		if (bytes == 0) {
#ifdef DEV_BUILD
			ib_fprintf(ib_stderr, "PIO_read: an empty page read!\n");
			ib_fflush(ib_stderr);
#endif
		}
		else {
#ifdef DEV_BUILD
			ib_fprintf(ib_stderr, "PIO_read: retry count exceeded\n");
			ib_fflush(ib_stderr);
#endif
			unix_error("read_retry", file, isc_io_read_err, 0);
		}
	}

	ISC_enable();
	return TRUE;
}


int PIO_write(File* file, Bdb* bdb, PAG page, ISC_STATUS * status_vector)
{
/**************************************
 *
 *	P I O _ w r i t e
 *
 **************************************
 *
 * Functional description
 *	Write a data page.  Oh wow.
 *
 **************************************/
	DBB dbb;
	SSHORT i;
	SLONG bytes, size;
    UINT64 offset;

	ISC_inhibit();

	if (file->fil_desc == -1)
		return unix_error("write", file, isc_io_write_err, status_vector);

	dbb = bdb->bdb_dbb;
	size = dbb->dbb_page_size;

#ifdef ISC_DATABASE_ENCRYPTION
	if (dbb->dbb_encrypt_key) {
		SLONG spare_buffer[MAX_PAGE_SIZE / sizeof(SLONG)];

		(*dbb->dbb_encrypt) (dbb->dbb_encrypt_key->str_data,
							 page, size, spare_buffer);

		for (i = 0; i < IO_RETRY; i++) {
			if (!(file = seek_file(file, bdb, &offset, status_vector)))
				return FALSE;
#ifdef PREAD_PWRITE
			if ((bytes = pwrite(file->fil_desc, spare_buffer, size, LSEEK_OFFSET_CAST offset)) == size)
				break;
#else
			if ((bytes = write(file->fil_desc, spare_buffer, size)) == size)
				break;
			THD_MUTEX_UNLOCK(file->fil_mutex);
#endif
			if (bytes == -1U && !SYSCALL_INTERRUPTED(errno))
				return unix_error("write", file, isc_io_write_err,
								  status_vector);
		}
	}
	else
#endif /* ISC_DATABASE_ENCRYPTION */
	{
		for (i = 0; i < IO_RETRY; i++) {
			if (!(file = seek_file(file, bdb, &offset, status_vector)))
				return FALSE;
#ifdef PREAD_PWRITE
			if ((bytes = pwrite(file->fil_desc, page, size, LSEEK_OFFSET_CAST offset)) == size)
				break;
#else
			if ((bytes = write(file->fil_desc, page, size)) == size)
				break;
			THD_MUTEX_UNLOCK(file->fil_mutex);
#endif
			if (bytes == (SLONG) -1 && !SYSCALL_INTERRUPTED(errno))
				return unix_error("write", file, isc_io_write_err,
								  status_vector);
		}
	}

#ifndef PREAD_PWRITE
	THD_MUTEX_UNLOCK(file->fil_mutex);
#endif

	ISC_enable();
	return TRUE;
}


#ifdef UNIX
static void close_marker_file(TEXT * marker_filename)
{
/***************************************
 *
 *	c l o s e _ m a r k e r _ f i l e
 *
 ***************************************
 *
 * Functional description
 *	The caller has already found a marker
 *	file exists, so open the file, read
 *	the absolute path to the database,
 *	then start reading fildes and closing
 *	the open marker files.
 *
 ***************************************/
	TEXT fildes_str[5], marker_file_path[MAXPATHLEN];
	IB_FILE *fp;
	int fd;

/* Open the marker file and save the marker_file_path for later. */

	if ((fp = ib_fopen(marker_filename, "r")) == NULL)
		return;
	if (ib_fgets(marker_file_path, sizeof(marker_file_path), fp) == NULL)
		return;

/* Read the fildes string, convert it into a file descriptor and close. */

	while (ib_fgets(fildes_str, sizeof(fildes_str), fp) != NULL) {
		sscanf(fildes_str, "%d", &fd);
		close(fd);
	}

/* Remove all fildes from marker file by writing just the path to the file. */

	if ((fp = ib_fopen(marker_filename, "w")) == NULL)
		return;

	ib_fputs(marker_file_path, fp);
	ib_fclose(fp);
}
#endif


static File* seek_file(File* file, Bdb* bdb, UINT64 * offset, ISC_STATUS * status_vector)
{
/**************************************
 *
 *	s e e k _ f i l e
 *
 **************************************
 *
 * Functional description
 *	Given a buffer descriptor block, find the appropropriate
 *	file block and seek to the proper page in that file.
 *
 **************************************/
	ULONG page;
	DBB dbb;
    UINT64 lseek_offset;

	dbb = bdb->bdb_dbb;
	page = bdb->bdb_page;

	for (;; file = file->fil_next)
		if (!file) {
			ISC_enable();
			CORRUPT(158);		/* msg 158 database file not available */
		}
		else if (page >= file->fil_min_page && page <= file->fil_max_page)
			break;

	if (file->fil_desc == -1)
		return (File*)(ULONG) unix_error("lseek", file, isc_io_access_err,
								status_vector);

	page -= file->fil_min_page - file->fil_fudge;

    lseek_offset = page;
    lseek_offset *= dbb->dbb_page_size;

#if _FILE_OFFSET_BITS != 64
    if (lseek_offset > MAX_SLONG) {
        return (File*)(ULONG) unix_error ("lseek", file, isc_io_32bit_exceeded_err, status_vector);
    }
#endif

#ifdef PREAD_PWRITE
	*offset = lseek_offset;
#else
	THD_MUTEX_LOCK(file->fil_mutex);

	if ((lseek(file->fil_desc, LSEEK_OFFSET_CAST lseek_offset, 0)) == (off_t)-1) {
		THD_MUTEX_UNLOCK(file->fil_mutex);
		return (File*)(ULONG) unix_error("lseek", file, isc_io_access_err,
								status_vector);
	}
#endif

	return file;
}


static File* setup_file(DBB dbb, const TEXT* file_name, USHORT file_length, int desc)
{
/**************************************
 *
 *	s e t u p _ f i l e
 *
 **************************************
 *
 * Functional description
 *	Set up file and lock blocks for a file.
 *
 **************************************/
	File* file;
	Lock* lock;
	UCHAR *p, *q, lock_string[32];
	USHORT l;
	struct stat statistics;

/* Allocate file block and copy file name string */

	file = FB_NEW_RPT(*dbb->dbb_permanent, file_length + 1) File();
	file->fil_desc = desc;
	file->fil_length = file_length;
	file->fil_max_page = -1UL;
	MOVE_FAST(file_name, file->fil_string, file_length);
	file->fil_string[file_length] = '\0';
#ifndef PREAD_PWRITE
	THD_MUTEX_INIT(file->fil_mutex);
#endif

/* If this isn't the primary file, we're done */

	if (dbb->dbb_file)
		return file;

/* Build unique lock string for file and construct lock block */

	fstat(desc, &statistics);
	p = lock_string;

	q = (UCHAR *) & statistics.st_dev;
	l = sizeof(statistics.st_dev);
	do
		*p++ = *q++;
	while (--l);

	q = (UCHAR *) & statistics.st_ino;
	l = sizeof(statistics.st_ino);
	do
		*p++ = *q++;
	while (--l);

	l = p - lock_string;

	dbb->dbb_lock = lock = FB_NEW_RPT(*dbb->dbb_permanent, l) Lock();
	lock->lck_type = LCK_database;
	lock->lck_owner_handle = LCK_get_owner_handle(NULL, lock->lck_type);
	lock->lck_object = reinterpret_cast<blk*>(dbb);
	lock->lck_length = l;
	lock->lck_dbb = dbb;
	lock->lck_ast = CCH_DOWN_GRADE_DBB;
	MOVE_FAST(lock_string, lock->lck_key.lck_string, l);

/* Try to get an exclusive lock on database.  If this fails, insist
   on at least a shared lock */

	dbb->dbb_flags |= DBB_exclusive;
	if (!LCK_lock(NULL, lock, LCK_EX, LCK_NO_WAIT)) {
		dbb->dbb_flags &= ~DBB_exclusive;
		LCK_lock(NULL, lock, LCK_SW, LCK_WAIT);
	}

	return file;
}


static BOOLEAN unix_error(
						  TEXT * string,
						  File* file, ISC_STATUS operation, ISC_STATUS * status_vector)
{
/**************************************
 *
 *	u n i x _ e r r o r
 *
 **************************************
 *
 * Functional description
 *	Somebody has noticed a file system error and expects error
 *	to do something about it.  Harumph!
 *
 **************************************/
	ISC_STATUS *status;

	ISC_enable();

	if (status = status_vector) {
		*status++ = isc_arg_gds;
		*status++ = isc_io_error;
		*status++ = isc_arg_string;
		*status++ = (ISC_STATUS) string;
		*status++ = isc_arg_string;
		*status++ = (ISC_STATUS) ERR_string(file->fil_string, file->fil_length);
		*status++ = isc_arg_gds;
		*status++ = operation;
		*status++ = isc_arg_unix;
		*status++ = errno;
		*status++ = isc_arg_end;
		gds__log_status(file->fil_string, status_vector);
		return FALSE;
	}
	else
		ERR_post(isc_io_error,
				 isc_arg_string, string,
				 isc_arg_string, ERR_string(file->fil_string,
											file->fil_length), isc_arg_gds,
				 operation, isc_arg_unix, errno, 0);


    // Added a FALSE for final return - which seems to be the answer, 
    // but is better than what it was which was nothing ie random 
    // Most usages within here want it to return a failure.
    // MOD 01-July-2002

    return FALSE;
}

#if defined PREAD_PWRITE && !(defined HAVE_PREAD && defined HAVE_PWRITE)

/* pread() and pwrite() behave like read() and write() except that they
   take an additional 'offset' argument. The I/O takes place at the specified
   'offset' from the beginning of the file and does not affect the offset
   associated with the file descriptor.
   This is done in order to allow more than one thread to operate on
   individual records within the same file simultaneously. This is
   called Positioned I/O. Since positioned I/O is not currently directly
   available through the POSIX interfaces, it has been implemented
   using the POSIX asynchronous I/O calls.

   NOTE: pread() and pwrite() are defined in UNIX International system
         interface and are a part of POSIX systems.
*/

static SLONG pread(int fd, SCHAR * buf, SLONG nbytes, SLONG offset)
/**************************************
 *
 *	p r e a d
 *
 **************************************
 *
 * Functional description
 *
 *   This function uses Asynchronous I/O calls to implement
 *   positioned read from a given offset
 **************************************/
{
	struct aiocb io;
	struct aiocb *list[1];
	int err;

	io.aio_fildes = fd;
	io.aio_offset = offset;
	io.aio_buf = buf;
	io.aio_nbytes = nbytes;
	io.aio_reqprio = 0;
	io.aio_sigevent.sigev_notify = SIGEV_NONE;
	err = aio_read(&io);		/* atomically reads at offset */
	if (err != 0)
		return (err);			/* errno is set */
	list[0] = &io;
	err = aio_suspend(list, 1, NULL);	/* wait for I/O to complete */
	if (err != 0)
		return (err);			/* errno is set */
	return (aio_return(&io));	/* return I/O status */
}

static SLONG pwrite(int fd, SCHAR * buf, SLONG nbytes, SLONG offset)
/**************************************
 *
 *	p w r i t e
 *
 **************************************
 *
 * Functional description
 *
 *   This function uses Asynchronous I/O calls to implement
 *   positioned write from a given offset
 **************************************/
{
	struct aiocb io;
	struct aiocb *list[1];
	int err;

	io.aio_fildes = fd;
	io.aio_offset = offset;
	io.aio_buf = buf;
	io.aio_nbytes = nbytes;
	io.aio_reqprio = 0;
	io.aio_sigevent.sigev_notify = SIGEV_NONE;
	err = aio_write(&io);		/* atomically reads at offset */
	if (err != 0)
		return (err);			/* errno is set */
	list[0] = &io;
	err = aio_suspend(list, 1, NULL);	/* wait for I/O to complete */
	if (err != 0)
		return (err);			/* errno is set */
	return (aio_return(&io));	/* return I/O status */
}

#endif /* PREAD_PWRITE && !(HAVE_PREAD && HAVE_PWRITE)*/


int PIO_unlink (const TEXT* file_name)
{
/**************************************
 *
 *	P I O _ u n l i n k
 *
 **************************************
 *
 * Functional description
 *	Delete a database file.
 *
 **************************************/

#ifdef SUPPORT_RAW_DEVICES
	if (raw_devices_check_file(file_name))
		return raw_devices_unlink_database(file_name);
	else
#endif
		return unlink(file_name);
}

#ifdef SUPPORT_RAW_DEVICES
static BOOLEAN
raw_devices_check_file (
	const TEXT* file_name)
{
/**************************************
 *
 *	raw_devices_check_file
 *
 **************************************
 *
 * Functional description
 *	Checks if the supplied file name is a special file
 *
 **************************************/
	struct stat s;

	return (stat(file_name, &s) == 0
			&& (S_ISCHR(s.st_mode) || S_ISBLK(s.st_mode)));
}

static BOOLEAN
raw_devices_validate_database (
	int desc,
	const TEXT* file_name,
	USHORT file_length)
{
/**************************************
 *
 *	raw_devices_validate_database
 *
 **************************************
 *
 * Functional description
 *	Checks if the special file contains a valid database
 *
 **************************************/
	char header[MIN_PAGE_SIZE];
	HDR hp = (HDR)header;
	ssize_t bytes;
	BOOLEAN retval = FALSE;
	int i;

	/* Read in database header. Code lifted from PIO_header. */
	if (desc == -1)
		ERR_post (isc_io_error,
					isc_arg_string, "raw_devices_validate_database",
					isc_arg_string, ERR_string (file_name, file_length),
					isc_arg_gds, isc_io_read_err,
					isc_arg_unix, errno, 0);

	for (i = 0; i < IO_RETRY; i++)
	{
		if (lseek (desc, LSEEK_OFFSET_CAST 0, 0) == (off_t) -1)
			ERR_post (isc_io_error,
						isc_arg_string, "lseek",
						isc_arg_string, ERR_string (file_name, file_length),
						isc_arg_gds, isc_io_read_err,
						isc_arg_unix, errno, 0);
		if ((bytes = read (desc, header, sizeof(header))) == sizeof(header))
			goto read_finished;
		if (bytes == -1 && !SYSCALL_INTERRUPTED(errno))
			ERR_post (isc_io_error,
						isc_arg_string, "read",
						isc_arg_string, ERR_string (file_name, file_length),
						isc_arg_gds, isc_io_read_err,
						isc_arg_unix, errno, 0);
	}

	ERR_post (isc_io_error,
				isc_arg_string, "read_retry",
				isc_arg_string, ERR_string (file_name, file_length),
				isc_arg_gds, isc_io_read_err,
				isc_arg_unix, errno, 0);

  read_finished:
	/* Rewind file pointer */
	if (lseek (desc, LSEEK_OFFSET_CAST 0, 0) == (off_t)-1)
		ERR_post (isc_io_error,
					isc_arg_string, "lseek",
					isc_arg_string, ERR_string (file_name, file_length),
					isc_arg_gds, isc_io_read_err,
					isc_arg_unix, errno, 0);

	/* Validate database header. Code lifted from PAG_header. */
	if (hp->hdr_header.pag_type != pag_header /*|| hp->hdr_sequence*/)
		goto quit;

#ifdef ODS_8_TO_CURRENT
	/* This Server understands ODS greater than 8 *ONLY* upto current major
	 * ODS_VERSION defined in ods.h, Refuse connections to older or newer ODS's.
	 */
	if ((hp->hdr_ods_version < ODS_VERSION8)
		|| (hp->hdr_ods_version > ODS_VERSION))
#else
	if (hp->hdr_ods_version != ODS_VERSION)
#endif
		goto quit;

	if (hp->hdr_page_size < MIN_PAGE_SIZE || hp->hdr_page_size > MAX_PAGE_SIZE)
		goto quit;

	/* At this point we think we have identified a database on the device.
 	 * PAG_header will validate the entire structure later.
 	 */
	retval = TRUE;

  quit:
#ifdef DEV_BUILD
	gds__log ("raw_devices_validate_database: %s -> %s%s\n",
		 file_name,
		 retval ? "true" : "false",
		 retval && hp->hdr_sequence != 0 ? " (continuation file)" : "");
#endif
	return retval;
}

static int
raw_devices_unlink_database (
	const TEXT* file_name)
{
	char header[MIN_PAGE_SIZE];
	size_t file_length = strlen(file_name);
	ssize_t bytes;
	int desc, i;

	for (i = 0; i < IO_RETRY; i++)
	{
		if ((desc = open (file_name, O_RDWR | O_BINARY)) != -1)
			break;
		if (!SYSCALL_INTERRUPTED(errno))
			ERR_post (isc_io_error,
						isc_arg_string, "open",
						isc_arg_string, ERR_string (file_name, file_length),
						isc_arg_gds, isc_io_open_err,
						isc_arg_unix, errno, 0);
	}

	memset(header, 0xa5, sizeof(header));

	for (i = 0; i < IO_RETRY; i++)
	{
		if ((bytes = write (desc, header, sizeof(header))) == sizeof(header))
			break;
		if (bytes == -1 && SYSCALL_INTERRUPTED(errno))
			continue;
		ERR_post (isc_io_error,
			isc_arg_string, "write",
			isc_arg_string, ERR_string (file_name, file_length),
			isc_arg_gds, isc_io_write_err,
			isc_arg_unix, errno, 0);
	}

	(void)close(desc);

#if DEV_BUILD
	gds__log ("raw_devices_unlink_database: %s -> %s\n",
				file_name, i < IO_RETRY ? "true" : "false");
#endif

	return 0;
}
#endif /* SUPPORT_RAW_DEVICES */

