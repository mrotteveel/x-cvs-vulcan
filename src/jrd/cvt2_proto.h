/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		cvt2_proto.h
 *	DESCRIPTION:	Prototype header file for cvt2.cpp
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
 */

#ifndef JRD_CVT2_PROTO_H
#define JRD_CVT2_PROTO_H

struct thread_db;

SSHORT	CVT2_compare(thread_db* tdbb, const dsc*, const dsc*, FPTR_ERROR);
SSHORT	CVT2_blob_compare(const dsc*, const dsc*, FPTR_ERROR);
void	CVT2_get_name(const dsc*, TEXT*, FPTR_ERROR);
USHORT	CVT2_make_string2(const dsc*, USHORT, UCHAR**, vary*,
								USHORT, str**, FPTR_ERROR);

#endif // JRD_CVT2_PROTO_H

