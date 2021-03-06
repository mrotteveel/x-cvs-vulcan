/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		isc_proto.h
 *	DESCRIPTION:	Prototype header file for isc.cpp
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

#ifndef JRD_ISC_PROTO_H
#define JRD_ISC_PROTO_H

void	ISC_ast_enter(void);
void	ISC_ast_exit(void);
int		ISC_check_process_existence(SLONG, SLONG, USHORT);
void	ISC_get_config(TEXT *, struct ipccfg *);
int		ISC_set_config(TEXT *, struct ipccfg *);
TEXT*	INTERNAL_API_ROUTINE ISC_get_host(TEXT *, USHORT);

bool	INTERNAL_API_ROUTINE ISC_get_user(TEXT*, int*, int*, TEXT*, TEXT*, int*, const TEXT*);

SLONG	ISC_get_user_group_id(const TEXT*);
void	ISC_set_user(const TEXT*);
SLONG	API_ROUTINE ISC_get_prefix(const TEXT*);
void	API_ROUTINE ISC_prefix(TEXT*, const TEXT*);
void	API_ROUTINE ISC_prefix_lock(TEXT*, const TEXT*);
void	API_ROUTINE ISC_prefix_msg(TEXT*, const TEXT*);

#ifdef VMS
int		ISC_expand_logical_once(const TEXT*, USHORT, TEXT*);
int		ISC_make_desc(const TEXT*, struct dsc$descriptor*, USHORT);
void	ISC_wait(SSHORT *, SLONG);
void	ISC_wake(SLONG);
void	ISC_wake_init(void);
#endif

#ifdef WIN_NT
BOOLEAN ISC_is_WinNT(void);
struct _SECURITY_ATTRIBUTES* ISC_get_security_desc(void);
/* Disabled. Not found anywhere.
TEXT*	ISC_prefix_interbase(TEXT*, TEXT*);
*/
#endif

#endif // JRD_ISC_PROTO_H

