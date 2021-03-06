/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		ini_proto.h
 *	DESCRIPTION:	Prototype header file for ini.cpp
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

#ifndef JRD_INI_PROTO_H
#define JRD_INI_PROTO_H

void	INI_format(thread_db* tdbb, const TEXT*, const TEXT*);
USHORT	INI_get_trig_flags(const TEXT*);
void	INI_init(thread_db* tdbb);
void	INI_init2(thread_db* tdbb);
const struct jrd_trg*	INI_lookup_sys_trigger(Relation*, const jrd_trg*,
										  const UCHAR**, UCHAR*, const SCHAR**,
										  USHORT*);
void INI_update_database(thread_db* tdbb);

#endif // JRD_INI_PROTO_H

