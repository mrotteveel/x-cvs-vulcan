/*
 *	PROGRAM:	JRD Access method
 *	MODULE:		filte_proto.h
 *	DESCRIPTION:	Prototype Header file for filters.cpp
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

#ifndef JRD_FILTE_PROTO_H
#define JRD_FILTE_PROTO_H

#ifdef __cplusplus
extern "C" {
#endif

ISC_STATUS filter_acl(USHORT, ctl*);
ISC_STATUS filter_blr(USHORT, ctl*);
ISC_STATUS filter_format(USHORT, ctl*);
ISC_STATUS filter_runtime(USHORT, ctl*);
ISC_STATUS filter_text(USHORT, ctl*);
ISC_STATUS filter_transliterate_text(USHORT, ctl*);
ISC_STATUS filter_trans(USHORT, ctl*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // JRD_FILTE_PROTO_H

