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
 *		Created August 1, 2005 by James A. Starkey
 *
 *  All Rights Reserved.
 */

#include "fbdev.h"
#include "RsbCount.h"
#include "jrd.h"
#include "rse.h"
#include "Request.h"
#include "CompilerScratch.h"
#include "req.h"

RsbCount::RsbCount(CompilerScratch *csb, RecordSource *next) : RecordSource(csb, rsb_counter)
{
	rsb_next = next;
}

RsbCount::~RsbCount(void)
{
}

void RsbCount::open(Request* request)
{
	rsb_next->open(request);
}

bool RsbCount::get(Request* request, RSE_GET_MODE mode)
{
	const bool count = (request->req_flags & req_count_records) != 0;
	request->req_flags &= ~req_count_records;

	if (!rsb_next->get(request, mode))
		return false;

	if (count) {
		request->req_records_selected++;
		request->req_records_affected++;
		request->req_flags |= req_count_records;
	}
		
	return true;
}

bool RsbCount::getExecutionPathInfo(Request* request, ExecutionPathInfoGen* infoGen)
{
	return rsb_next->getExecutionPathInfo(request, infoGen);
}

void RsbCount::close(Request* request)
{
	rsb_next->close(request);
}
