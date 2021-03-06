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
 *  Copyright (c) 1997 - 2000, 2001, 2003 James A. Starkey
 *  All Rights Reserved.
 */

// Generate.cpp: implementation of the Generate class.
//
//////////////////////////////////////////////////////////////////////

#include <string.h>
#include "fbdev.h"
#include "PBGen.h"

PBGen::PBGen(void)
{
}


PBGen::PBGen(UCHAR version)
{
	appendUCHAR(version);
}

PBGen::~PBGen(void)
{
}

void PBGen::putParameter(UCHAR parameter, int value)
{
	appendUCHAR(parameter);
	appendUCHAR(4);
	appendInt(value);
}

void PBGen::putParameter(UCHAR parameter, const TEXT* string)
{
	appendUCHAR(parameter);
	int length = (int) strlen (string);
	appendUCHAR(length);
	appendCharacters(length, string);
}

void PBGen::putParameter(UCHAR parameter, int length, const TEXT* string)
{
	appendUCHAR(parameter);
	appendUCHAR(length);
	appendCharacters(length, string);
}

void PBGen::putParameter(UCHAR parameter, int length, const UCHAR* string)
{
	appendUCHAR(parameter);
	appendUCHAR(length);
	appendData(length, string);
}

void PBGen::putParameter(UCHAR parameter)
{
	appendUCHAR(parameter);
	appendUCHAR(0);
}
