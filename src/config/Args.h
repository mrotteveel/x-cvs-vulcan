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
 *  Copyright (c) 1997 - 2000, 2001, 2003 Netfrastructure, Inc.
 *  All Rights Reserved.
 */
// Args.h: interface for the Args class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARGS_H__7C584339_5A11_4040_889B_417B607C858B__INCLUDED_)
#define AFX_ARGS_H__7C584339_5A11_4040_889B_417B607C858B__INCLUDED_

//#include "JString.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct Switches 
	{
    const char	*string;
	bool		*boolean;
	const char	**argument;
	const char	*argName;
	const char	*description;
	};

#define WORD_ARG(value,name)	"", NULL, &value, name, NULL,
#define ARG_ARG(sw,value,help)	sw, NULL, &value, NULL, help,
#define SW_ARG(sw,value,help)	sw, &value,NULL,  NULL, help,

class Args  
{
public:
	static bool readPassword (const char *msg, char *pw1, int length);
	static bool readPasswords(const char *msg, char *pw1, int length);
	static void printHelp (const char *helpText, const Switches *switches);
	static void init (const Switches *switches);
	static void parse (const Switches *switches, int argc, const char **argv, Args *args=NULL);
	Args();
	virtual ~Args();

	const char	**parameters;
	int			*lengths;
	int			parametersAllocated;
	int			parametersUsed;
	void extendParameters(void);
	void addParameter(const char* parameter, int length);
	const char* lookupParameter(const char* parameter);
	void parseParameters(const Switches* switches, int argc, const char** argv);
};

#endif // !defined(AFX_ARGS_H__7C584339_5A11_4040_889B_417B607C858B__INCLUDED_)
