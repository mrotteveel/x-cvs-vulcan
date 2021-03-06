/*
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
 * December 27, 2003	Created by James A. Starkey
 *
 */

#ifndef _Trigger_H_
#define _Trigger_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "JString.h"

class str;
class Relation;
class Request;

struct thread_db;

class Trigger
{
public:
	Trigger(void);
	~Trigger(void);

    str*		blr; // BLR code
	Request*	request; // Compiled request. Gets filled on first invocation
	BOOLEAN		compile_in_progress;
	BOOLEAN		sys_trigger;
	USHORT		flags; // Flags as they are in RDB$TRIGGERS table
	Relation*	relation; // Trigger parent relation
	JString		name; // Trigger name
	void		compile(thread_db* _tdbb); // Ensure that trigger is compiled
	BOOLEAN		release(thread_db* _tdbb); // Try to free trigger request
};

/***
class Triggers : public SIVector<Trigger*>
{
};
***/

//typedef firebird::vector<Trigger*> trig_vec;
//typedef trig_vec* TRIG_VEC;

#endif
