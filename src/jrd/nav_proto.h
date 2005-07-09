/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		nav_proto.h
 *	DESCRIPTION:	Prototype header file for nav.cpp
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

#ifndef JRD_NAV_PROTO_H
#define JRD_NAV_PROTO_H

#include "../jrd/rse.h"

#ifdef SCROLLABLE_CURSORS
struct jrd_exp* NAV_expand_index(struct win*, struct irsb_nav*);
#endif
BOOLEAN NAV_get_record(thread_db*, class RecordSource*, struct irsb_nav*, struct record_param*,RSE_GET_MODE);

#ifdef PC_ENGINE
BOOLEAN NAV_find_record(thread_db*, class RecordSource*, USHORT, USHORT, struct jrd_nod*);
void NAV_get_bookmark(class RecordSource*, struct irsb_nav*, struct Bookmark*);
BOOLEAN NAV_reset_position(class RecordSource*, struct record_param*);
BOOLEAN NAV_set_bookmark(thread_db*, class RecordSource*, struct irsb_nav*, struct record_param*,
								struct Bookmark*);
#endif

#endif // JRD_NAV_PROTO_H
