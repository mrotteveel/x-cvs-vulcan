/*
 *	PROGRAM:		JRD Access Method
 *	MODULE:			RsbBoolean.h
 *	DESCRIPTION:	Record source block definitions
 *
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
 * Refactored July 11, 2005 by James A. Starkey
 */

#ifndef JRD_RSB_BOOLEAN_H
#define JRD_RSB_BOOLEAN_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "RecordSource.h"

class RsbBoolean : public RecordSource
{
public:
	RsbBoolean(CompilerScratch *csb, RecordSource* prior_rsb, jrd_nod* node);
	virtual ~RsbBoolean(void);
	virtual void open(Request* request);
	virtual bool get(Request* request, RSE_GET_MODE mode);
	virtual bool getExecutionPathInfo(Request* request, ExecutionPathInfoGen* infoGen);
	virtual void close(Request* request);
	
	jrd_nod		*boolean;
	jrd_nod		*rsb_any_boolean;	// any/all boolean
	
	bool getAny(Request* request, RSE_GET_MODE mode);
};

#endif

