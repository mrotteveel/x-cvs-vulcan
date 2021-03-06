/*
 *	PROGRAM:	JRD Remote Interface/Server
 *	MODULE:		remot_proto.h
 *	DESCRIPTION:	Prototpe header file for remote.cpp
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

#ifndef REMOTE_REMOT_PROTO_H
#define REMOTE_REMOT_PROTO_H

class Port;
class RRequest;
class RTransaction;
class RStatement;
class RMessage;

struct Packet;

//void		REMOTE_cleanup_transaction (RTransaction *);
ULONG		REMOTE_compute_batch_size (Port *, USHORT, P_OP, FMT);
void		REMOTE_get_timeout_params (Port*, const UCHAR*, USHORT);
RRequest*	REMOTE_find_request (RRequest *, USHORT);
void		REMOTE_free_packet (Port *, Packet *);
//struct str*	REMOTE_make_string (const SCHAR*);
void		REMOTE_release_messages (RMessage *);
//void		REMOTE_release_request (RRequest *);
void		REMOTE_reset_request (RRequest *, RMessage *);
void		REMOTE_reset_statement (RStatement *);
void		REMOTE_save_status_strings (ISC_STATUS *);
//OBJCT		REMOTE_set_object (Port *, struct blk *, OBJCT);

#endif // REMOTE_REMOT_PROTO_H
