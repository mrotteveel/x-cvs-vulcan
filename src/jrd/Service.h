// Service.h: interface for the Service class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVICE_H__64F0DF89_DA26_48DD_951A_5BCDAF8CCDA4__INCLUDED_)
#define AFX_SERVICE_H__64F0DF89_DA26_48DD_951A_5BCDAF8CCDA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../jrd/jrd_blks.h"
#include "../include/fb_blk.h"
#include "AsyncEvent.h"
//#include "../include/fb_vector.h"

#ifndef MAX_PASSWORD_ENC_LENGTH
#define MAX_PASSWORD_ENC_LENGTH 12
#endif

/* Bitmask values for the svc_flags variable */

#define SVC_eof			 1
#define SVC_timeout		 2
#define SVC_forked		 4
#define SVC_detached	 8
#define SVC_finished	16
#define SVC_thd_running	32
#define SVC_evnt_fired	64

class Service;

typedef int (*pfn_svc_main) (Service*);
typedef int (*pfn_svc_output)(Service*, const UCHAR*);

struct serv
{
	USHORT		serv_action;
	const TEXT*	serv_name;
	const TEXT*	serv_std_switches;
	const TEXT*	serv_executable;
	pfn_svc_main	serv_thd;
	BOOLEAN*	in_use;
};

typedef serv *SERV;

class Service// : public pool_alloc<type_svc>
{
public:
	Service();
	virtual		~Service();
	SLONG			svc_handle;			/* "handle" of process/thread running service */
	//ISC_STATUS*	svc_status;			/* status vector for svc_handle */
	ISC_STATUS		svc_status[ISC_STATUS_LENGTH];
	void*			svc_input;			/* input to service */
	void*			svc_output;			/* output from service */
	ULONG			svc_stdout_head;
	ULONG			svc_stdout_tail;
	UCHAR*			svc_stdout;
	TEXT**			svc_argv;
	ULONG			svc_argc;
	AsyncEvent		svc_start_event[1];	/* fired once service has started successfully */
	const serv*		svc_service;
	UCHAR*			svc_resp_buf;
	UCHAR*			svc_resp_ptr;
	USHORT			svc_resp_buf_len;
	USHORT			svc_resp_len;
	USHORT			svc_flags;
	USHORT			svc_user_flag;
	USHORT			svc_spb_version;
	BOOLEAN			svc_do_shutdown;
	TEXT			svc_username[33];
	TEXT			svc_enc_password[MAX_PASSWORD_ENC_LENGTH];
	TEXT			svc_reserved[1];
	TEXT*			svc_switches;
};


#endif // !defined(AFX_SERVICE_H__64F0DF89_DA26_48DD_951A_5BCDAF8CCDA4__INCLUDED_)
