/* $Id$ */
/*
 *  
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
 *
 *  The Original Code was created by James A. Starkey for IBPhoenix.
 *
 *  Copyright (c) 1999, 2000, 2001 James A. Starkey
 *  All Rights Reserved.
 */

// BinaryBlob.h: interface for the BinaryBlob class.
//
//////////////////////////////////////////////////////////////////////

/*
 * copyright (c) 1999 - 2000 by James A. Starkey for IBPhoenix.
 */


#ifndef _LOCKACQUIRE_H_
#define _LOCKACQUIRE_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class LockMgr;

class LockAcquire
{
public:
#ifdef SHARED_CACHE
	LockAcquire(int offset);
#else
	LockAcquire(int offset, LockMgr *lm);
#endif
	~LockAcquire(void);
	void released(void);

	int		ptr;
	bool	wasReleased;
#ifndef SHARED_CACHE
	LockMgr *lockMgr;
#endif
};

#endif
