/* $OpenBSD: dispatch.c,v 1.25 2015/01/30 01:13:33 djm Exp $ */
/*
 * Copyright (c) 2000 Markus Friedl.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>

#include <signal.h>
#include <stdarg.h>

#include "ssh1.h"
#include "ssh2.h"
#include "log.h"
#include "dispatch.h"
#include "packet.h"
#include "compat.h"
#include "ssherr.h"

int
dispatch_protocol_error(int type, u_int32_t seq, void *ctx)
{
	struct ssh *ssh = active_state; /* XXX */
	int r;

	logit("dispatch_protocol_error: type %d seq %u", type, seq);
	if (!compat20)
		fatal("protocol error");
	if ((r = sshpkt_start(ssh, SSH2_MSG_UNIMPLEMENTED)) != 0 ||
	    (r = sshpkt_put_u32(ssh, seq)) != 0 ||
	    (r = sshpkt_send(ssh)) != 0 ||
	    (r = ssh_packet_write_wait(ssh)) != 0)
		sshpkt_fatal(ssh, __func__, r);
	return 0;
}

int
dispatch_protocol_ignore(int type, u_int32_t seq, void *ssh)
{
	logit("dispatch_protocol_ignore: type %d seq %u", type, seq);
	return 0;
}

void
ssh_dispatch_init(struct ssh *ssh, dispatch_fn *dflt)
{
	u_int i;
	for (i = 0; i < DISPATCH_MAX; i++)
		ssh->dispatch[i] = dflt;
}

void
ssh_dispatch_range(struct ssh *ssh, u_int from, u_int to, dispatch_fn *fn)
{
	u_int i;

	for (i = from; i <= to; i++) {
		if (i >= DISPATCH_MAX)
			break;
		ssh->dispatch[i] = fn;
	}
}

void
ssh_dispatch_set(struct ssh *ssh, int type, dispatch_fn *fn)
{
	ssh->dispatch[type] = fn;
}

int
ssh_dispatch_run(struct ssh *ssh, int mode, volatile sig_atomic_t *done,
    void *ctxt)
{
	int r;
	u_char type;
	u_int32_t seqnr;

	for (;;) {
		if (mode == DISPATCH_BLOCK) {
			r = ssh_packet_read_seqnr(ssh, &type, &seqnr);
			if (r != 0)
				return r;
		} else {
			r = ssh_packet_read_poll_seqnr(ssh, &type, &seqnr);
			if (r != 0)
				return r;
			if (type == SSH_MSG_NONE)
				return 0;
		}
		if (type > 0 && type < DISPATCH_MAX &&
		    ssh->dispatch[type] != NULL) {
			if (ssh->dispatch_skip_packets) {
				debug2("skipped packet (type %u)", type);
				ssh->dispatch_skip_packets--;
				continue;
			}
			/* XXX 'ssh' will replace 'ctxt' later */
			r = (*ssh->dispatch[type])(type, seqnr, ctxt);
			if (r != 0)
				return r;
		} else {
			r = sshpkt_disconnect(ssh,
			    "protocol error: rcvd type %d", type);
			if (r != 0)
				return r;
			return SSH_ERR_DISCONNECTED;
		}
		if (done != NULL && *done)
			return 0;
	}
}

void
ssh_dispatch_run_fatal(struct ssh *ssh, int mode, volatile sig_atomic_t *done,
    void *ctxt)
{
	int r;

	if ((r = ssh_dispatch_run(ssh, mode, done, ctxt)) != 0) {
		switch (r) {
		case SSH_ERR_CONN_CLOSED:
			logit("Connection closed by %.200s",
			    ssh_remote_ipaddr(ssh));
			cleanup_exit(255);
		case SSH_ERR_CONN_TIMEOUT:
			logit("Connection to %.200s timed out while "
			    "waiting to read", ssh_remote_ipaddr(ssh));
			cleanup_exit(255);
		default:
			fatal("%s: %s", __func__, ssh_err(r));
		}
	}
}
