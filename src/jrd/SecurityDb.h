/*
 *     The contents of this file are subject to the Initial 
 *     Developer's Public License Version 1.0 (the "License"); 
 *     you may not use this file except in compliance with the 
 *     License. You may obtain a copy of the License at 
 *     http://www.ibphoenix.com/idpl.html. 
 *
 *     Software distributed under the License is distributed on 
 *     an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either 
 *     express or implied.  See the License for the specific 
 *     language governing rights and limitations under the License.
 *
 *     The contents of this file or any work derived from this file
 *     may not be distributed under any other license whatsoever 
 *     without the express prior written permission of the original 
 *     author.
 *
 *
 *  The Original Code was created by James A. Starkey for IBPhoenix.
 *
 *  Copyright (c) 1997 - 2000, 2001, 2003 James A. Starkey
 *  Copyright (c) 1997 - 2000, 2001, 2003 Netfrastructure, Inc.
 *  All Rights Reserved.
 */

//////////////////////////////////////////////////////////////////////

#ifndef _SECURITY_DB_H
#define _SECURITY_DB_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SecurityPlugin.h"
#include "JString.h"
#include "ibase.h"

class PreparedStatement;
class Connection;

class SecurityDb : public SecurityPlugin
{
public:
	SecurityDb(SecurityPlugin *chain);
	virtual ~SecurityDb(void);
	virtual void updateAccountInfo(SecurityContext *context, int apbLength, const UCHAR* apb);
	virtual void authenticateUser(SecurityContext *context, int dpbLength, const UCHAR* dpb, int itemsLength, const UCHAR* items, int bufferLength, UCHAR* buffer);

	JString			databaseName;
	bool			self;
	bool			none;
	bool			haveTable;
	isc_db_handle	dbHandle;
	Connection		*connection;
	PreparedStatement	*authenticate;
	
	void attachDatabase(void);
	bool checkUsersTable(Connection* connection);
	void close(void);
};

#endif
