/*
 *	PROGRAM:	Security data base manager
 *	MODULE:		call_service.cpp
 *	DESCRIPTION:	Invokes remote service manager to work with security DB.
 *
 *  The contents of this file are subject to the Initial
 *  Developer's Public License Version 1.0 (the "License");
 *  you may not use this file except in compliance with the
 *  License. You may obtain a copy of the License at
 *  http://www.ibphoenix.com/main.nfs?a=ibphoenix&page=ibp_idpl.
 *
 *  Software distributed under the License is distributed AS IS,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *  See the License for the specific language governing rights
 *  and limitations under the License.
 *
 *  The Original Code was created by Alexander Peshkoff (peshkoff@mail.ru)
 *  for the Firebird Open Source RDBMS project.
 *
 * All Rights Reserved.
 * Contributor(s): ______________________________________.
 */

#include <string.h>
#include <stdio.h>

#include "fbdev.h"
#include "../jrd/common.h"
#include "../gsec/call_service.h"
#include "gdsassert.h"

const size_t SERVICE_SIZE = 256;
const size_t SERVER_PART = 200;

/**
  
 	isValidServer
  
    @brief	Validates server name for non-local protocol.
	Replaces the original ugly macro. 
	Now the function that calls isValidServer is responsible 
	for returning NULL to its invoker in turn. It simply makes 
	sure there's something in the string containing the	server's name; 
	otherwise it fills the status vector with an error.
 

    @param status
    @param server

 **/
static bool isValidServer(ISC_STATUS* status, const TEXT* server)
{
	if (!server || !*server)
	{
	    status[0] = isc_arg_gds;
	    status[1] = isc_bad_protocol;
	    status[2] = isc_arg_end;
	    return false;
	}
	return true;
}


/**
  
 	serverSizeValidate
  
    @brief	Validates server name in order to avoid 
	buffer overflow later. Server name may be NULL 
	in case of local access, we take it into account.
 

    @param status
    @param server

 **/
static bool serverSizeValidate(ISC_STATUS* status, const TEXT* server)
{
	if (! server)
	{
		return true;
	}
	if (strlen(server) < SERVER_PART)
	{
		return true;
	}
    status[0] = isc_arg_gds;
    status[1] = isc_gsec_inv_param;
    status[2] = isc_arg_end;
    return false;
}


static int typeBuffer(tsec* tdsec, ISC_STATUS*, char*, int, internal_user_data&,
					   FPTR_SECURITY_CALLBACK, void*);


// all this spb-writing functions should be gone 
// as soon as we create SvcClumpletWriter

inline void stuffSpbByte(char*& spb, char data)
{
	*spb++ = data;
}

inline void stuffSpbShort(char*& spb, short data)
{
	ADD_SPB_LENGTH(spb, data);
}

inline void stuffSpbLong(char*& spb, SLONG data)
{
	ADD_SPB_NUMERIC(spb, data);
}

static void stuffSpb(char*& spb, char param, const TEXT* value)
{
	stuffSpbByte(spb, param);
	int l = (int) strlen(value);
	fb_assert(l < 256);
	stuffSpbByte(spb, char(l));
	memcpy(spb, value, l);
	spb += l;
}

static void stuffSpb2(char*& spb, char param, const TEXT* value)
{
	stuffSpbByte(spb, param);
	int l = (int) strlen(value);
	stuffSpbShort(spb, short(l));
	memcpy(spb, value, l);
	spb += l;
}


/**
  
	attachRemoteServiceManager
  
	@brief	Opens connection with service manager 
	on server using protocol, login username/password.
 

	@param status
	@param username
	@param password
	@param protocol
	@param server

 **/
isc_svc_handle attachRemoteServiceManager(ISC_STATUS* status,
							  const TEXT* username,
							  const TEXT* password, 
							  int protocol, 
							  const TEXT* server)
{
	char service[SERVICE_SIZE];

	if (! serverSizeValidate(status, server))
	{
		return 0;
	}

	switch (protocol) {
	case sec_protocol_tcpip:
		if (! isValidServer(status, server))
		{
			return 0;
		}
		strncpy(service, server, SERVER_PART);
		strcat(service, ":");
		break;

	case sec_protocol_netbeui:
		if (! isValidServer(status, server))
		{
			return 0;
		}
		strcpy(service, "\\\\");
		strncat(service, server, SERVER_PART);
		strcat(service, "\\");
		break;

	case sec_protocol_local:
		service[0] = 0;
		break;

	default:
	    isValidServer(status, 0); // let it to set the error status
		return 0;
	}

	return attachRemoteServiceManager(status, username, password, service);
}


/**
  
	attachRemoteServiceManager
  
	@brief	Opens connection with service manager on server 
	with protocol in it's name, login username/password.
 

	@param status
	@param username
	@param password
	@param server

 **/
isc_svc_handle attachRemoteServiceManager(ISC_STATUS* status,
							  const TEXT* username,
							  const TEXT* password, 
							  const TEXT* server)
{
	char service[SERVICE_SIZE];

	if (! serverSizeValidate(status, server))
	{
		return 0;
	}
	strncpy(service, server, SERVER_PART);
	strcat(service, "service_mgr");

	char spb_buffer[1024];
	char* spb = spb_buffer;
	stuffSpbByte(spb, isc_spb_version);
	stuffSpbByte(spb, isc_spb_current_version);
	if (username && password)
	{
		stuffSpb(spb, isc_spb_user_name, username);
		stuffSpb(spb, isc_spb_password, password);
	}

	fb_assert((size_t)(spb - spb_buffer) <= sizeof(spb_buffer));
	isc_svc_handle svc_handle = 0;
	isc_service_attach(status, 
		static_cast<USHORT>(strlen(service)),
		service, &svc_handle, 
		static_cast<USHORT>(spb - spb_buffer), 
		spb_buffer);
	return status[1] ? 0 : svc_handle;
}


/**
  
	userInfoToSpb
  
	@brief	Writes data from awful borland's struct internal_user_data
	to not less awful borland's format of spb.
 

	@param spb
	@param userInfo

 **/
static void userInfoToSpb(char*& spb, 
						  const internal_user_data& userInfo)
{
	stuffSpb2(spb, isc_spb_sec_username, userInfo.user_name);
	if (userInfo.uid_entered) {
		stuffSpbByte(spb, isc_spb_sec_userid);
		stuffSpbLong(spb, userInfo.uid);
	}
	if (userInfo.gid_entered) {
		stuffSpbByte(spb, isc_spb_sec_groupid);
		stuffSpbLong(spb, userInfo.gid);
	}
	if (userInfo.group_name_entered) {
		stuffSpb2(spb, isc_spb_sec_groupname, userInfo.group_name);
	}
	if (userInfo.password_entered) {
		stuffSpb2(spb, isc_spb_sec_password, userInfo.password);
	}
	if (userInfo.first_name_entered) {
		stuffSpb2(spb, isc_spb_sec_firstname, userInfo.first_name);
	}
	if (userInfo.middle_name_entered) {
		stuffSpb2(spb, isc_spb_sec_middlename, userInfo.middle_name);
	}
	if (userInfo.last_name_entered) {
		stuffSpb2(spb, isc_spb_sec_lastname, userInfo.last_name);
	}
}


/**
  
	callRemoteServiceManager
  
	@brief	Calls service manager to execute command, 
	specified in userInfo
 

	@param status
	@param handle
	@param userInfo
	@param outputFunction
	@param functionArg

 **/
void callRemoteServiceManager(tsec* tdsec, 
							  ISC_STATUS* status, 
							  isc_svc_handle handle, 
							  const internal_user_data& userInfo,
							  FPTR_SECURITY_CALLBACK outputFunction,
							  void* functionArg)
{
	char spb_buffer[1024];
	char* spb = spb_buffer;
	
	if (userInfo.operation != DIS_OPER && !userInfo.user_name_entered)
		{
	    status[0] = isc_arg_gds;
	    status[1] = isc_gsec_switches_error;
	    status[2] = isc_arg_end;
		return;
		}

	switch (userInfo.operation) 
		{
		case ADD_OPER:
			stuffSpbByte(spb, isc_action_svc_add_user);
			userInfoToSpb(spb, userInfo);
			break;

		case MOD_OPER:
			stuffSpbByte(spb, isc_action_svc_modify_user);
			userInfoToSpb(spb, userInfo);
			break;

		case DEL_OPER:
			stuffSpbByte(spb, isc_action_svc_delete_user);
			stuffSpb2(spb, isc_spb_sec_username, userInfo.user_name);
			break;

		case DIS_OPER:
			stuffSpbByte(spb, isc_action_svc_display_user);
			
			if (userInfo.user_name_entered)
				stuffSpb2(spb, isc_spb_sec_username, userInfo.user_name);
			break;

		default:
			status[0] = isc_arg_gds;
			status[1] = isc_gsec_switches_error;
			status[2] = isc_arg_end;
			return;
		}

	fb_assert((size_t)(spb - spb_buffer) <= sizeof(spb_buffer));
	isc_service_start(status, &handle, 0, spb - spb_buffer, spb_buffer);
	
	if (status[1] || userInfo.operation != DIS_OPER || !outputFunction)
		return;

	spb = spb_buffer;
	stuffSpbByte(spb, isc_info_svc_timeout);
	stuffSpbLong(spb, 10);
	const char request = isc_info_svc_get_users;
	const int resultBufSize = 512;
	char resultBuffer[resultBufSize];
	int startQuery = 0;
	internal_user_data uData;
	memset(&uData, 0, sizeof uData);
	uData.user_name_entered = false;

	for (;;)
		{
		isc_resv_handle reserved = 0;
		isc_service_query(status, &handle, &reserved, spb - spb_buffer, 
							spb, 1, &request, resultBufSize - startQuery, 
							&resultBuffer[startQuery]);
							
		if (status[1])
			return;

		startQuery = typeBuffer(tdsec, status, resultBuffer, startQuery, uData,  outputFunction, functionArg);
		
		if (startQuery < 0)
			break;
		}

	if (uData.user_name[0])
		outputFunction(tdsec, functionArg, &uData, !uData.user_name_entered);
}


/**
  
	detachRemoteServiceManager
  
	@brief	Close service manager
 

	@param status
	@param handle

 **/
void detachRemoteServiceManager(ISC_STATUS* status, 
							    isc_svc_handle handle)
{
	isc_service_detach(status, &handle);
}


// all this spb-parsing functions should be gone 
// as soon as we create SvcClumpletReader

static void parseString2(const char*& p, char* buffer, size_t bufSize, size_t& loop)
{
	const size_t len = static_cast<size_t>(isc_vax_integer(p, sizeof(USHORT)));

	size_t len2 = len + sizeof(ISC_USHORT) + 1;
	if (len2 > loop)
	{
		throw loop;
	}
	loop -= len2;

	bufSize--;
	len2 = len < bufSize ? len : bufSize;
	p += sizeof(USHORT);
	memcpy (buffer, p, len2);
	buffer[len2] = 0;
	p += len;
}

static void parseLong(const char*& p, int& ul, size_t& loop)
{
	ul = isc_vax_integer(p, sizeof(ULONG));
	
	const size_t len2 = sizeof(ULONG) + 1;
	if (len2 > loop)
	{
		throw loop;
	}
	loop -= len2;

	p += sizeof(ULONG);
}


/**
  
	typeBuffer
  
	@brief	Prints data, returned by service, using outputFunction
 

	@param status
	@param buf
	@param offset
	@param uData
	@param outputFunction
	@param functionArg

 **/
static int typeBuffer(tsec* tdsec, ISC_STATUS* status, char* buf, int offset,
					   internal_user_data& uData,
					   FPTR_SECURITY_CALLBACK outputFunction,
					   void* functionArg)
{
	const char* p = &buf[offset];

	// Sanity checks
	if (*p++ != isc_info_svc_get_users)
		{
	    status[0] = isc_arg_gds;
	    status[1] = isc_gsec_params_not_allowed;
		status[2] = isc_arg_end;
		return -1;
		}
		
	size_t loop = isc_vax_integer (p, sizeof (USHORT));
	p += sizeof (USHORT);
	
    if (p[loop] != isc_info_end)
		{
	    status[0] = isc_arg_gds;
	    status[1] = isc_gsec_params_not_allowed;
		status[2] = isc_arg_end;
		return -1;
		}

	// No data - stop processing
	
	if (! loop)
		return -1;

	// Old data left - use them
	
	if (offset)
		{
		memmove(&buf[offset], p, loop);
		p = buf;
		offset = 0;
		}

	while (*p != isc_info_end)
		{
		try 
			{
			switch (*p++)
				{
				case isc_spb_sec_username:
					if (uData.user_name[0]) 
						{
						outputFunction(tdsec, functionArg, &uData, !uData.user_name_entered);
						memset(&uData, 0, sizeof uData);
						uData.user_name_entered = true;
						}
						
					parseString2(p, uData.user_name, sizeof(uData.user_name), loop);
					break;
					
				case isc_spb_sec_firstname:
					parseString2(p, uData.first_name, sizeof(uData.first_name), loop);
					break;
					
				case isc_spb_sec_middlename:
					parseString2(p, uData.middle_name, sizeof(uData.middle_name), loop);
					break;
					
				case isc_spb_sec_lastname:
					parseString2(p, uData.last_name, sizeof(uData.last_name), loop);
					break;
					
				case isc_spb_sec_groupid:
					parseLong(p, uData.gid, loop);
					break;
					
				case isc_spb_sec_userid:
					parseLong(p, uData.uid, loop);
					break;
					
				default:
					status[0] = isc_arg_gds;
					status[1] = isc_gsec_params_not_allowed;
					status[2] = isc_arg_end;
					return true;
				}
			}
		catch(size_t newOffset)
			{
			memmove(buf, --p, newOffset);

			return (int) newOffset;
			}
		}
		
	fb_assert(loop == 0);
	
	return 0;
}
