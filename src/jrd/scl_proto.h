/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		scl_proto.h
 *	DESCRIPTION:	Prototype header file for scl.epp
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

#ifndef JRD_SCL_PROTO_H
#define JRD_SCL_PROTO_H

class Generate;

void SCL_check_access(TDBB tdbb, struct scl*, SLONG, const TEXT*,
					  const TEXT*, USHORT, const TEXT*, const TEXT*);
void SCL_check_procedure(TDBB tdbb, struct dsc*, USHORT);
void SCL_check_relation(TDBB tdbb, struct dsc*, USHORT);
struct scl* SCL_get_class(TDBB tdbb, const TEXT*);
int SCL_get_mask(TDBB tdbb, const TEXT*, const TEXT*);
void SCL_init(bool, const TEXT*, TEXT*, TEXT*, TEXT*, TEXT*, const TEXT* securityDatabase, TDBB, bool);
void SCL_move_priv(Generate *acl, USHORT);
struct scl* SCL_recompute_class(TDBB, TEXT*);
void SCL_release(TDBB tdbb, struct scl*);
void SCL_check_index(TDBB, const TEXT*, UCHAR, USHORT);

#endif // JRD_SCL_PROTO_H

