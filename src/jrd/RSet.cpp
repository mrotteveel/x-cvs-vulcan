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
 *     The contents of this file or any work derived from this file
 *     may not be distributed under any other license whatsoever 
 *     without the express prior written permission of the original 
 *     author.
 *
 *
 *  The Original Code was created by James A. Starkey for IBPhoenix.
 *
 *  Copyright (c) 2004 James A. Starkey
 *  All Rights Reserved.
 */

// RSet.cpp: implementation of the RSet class.
//
//////////////////////////////////////////////////////////////////////

#include "firebird.h"
#include "RSet.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RSet::RSet()
{
	resultSet = NULL;
}

RSet::~RSet()
{
	if (resultSet)
		resultSet->close();
}

RSet::RSet(ResultSet *results)
{
	resultSet = results;
}

void RSet::operator =(ResultSet *results)
{
	if (resultSet)
		resultSet->close();

	resultSet = results;
}
