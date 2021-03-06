/*
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
 *  The Original Code was created by James A. Starkey
 *  for the Firebird Open Source RDBMS project.
 *
 *  Copyright (c)  2004 James A. Starkey
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): James A. Starkey
 */

#ifndef _MODULEMANAGER_H_
#define _MODULEMANAGER_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "SyncObject.h"
#include "LinkedList.h"

CLASS(Module);

START_NAMESPACE

class ModuleManager
{
public:
	ModuleManager(void);
	virtual ~ModuleManager(void);
	Module* findModule(const char* name);
	void addSearchPath(const char* path);
	
	Module		*modules;
	SyncObject	syncObject;
	LinkedList	searchList;
};

END_NAMESPACE

#endif
