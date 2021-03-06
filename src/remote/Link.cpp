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

#include "fbdev.h"
#include "common.h"
#include "Link.h"

void Link::putShort(short value)
{
}

void Link::putInt32(INT32 value)
{
}

void Link::putInt64(INT64 value)
{
}

void Link::putDouble(double value)
{
}

void Link::putCString(int length, const char* string)
{
}

short Link::getShort(void)
{
	return 0;
}

INT32 Link::getInt32(void)
{
	return INT32();
}

INT64 Link::getInt64(void)
{
	return INT64();
}

double Link::getDouble(void)
{
	return 0;
}

void Link::flushBlock()
{
}

void Link::flushMessage()
{
}

void Link::endBlock()
{
}

