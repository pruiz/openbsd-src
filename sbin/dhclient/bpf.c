/*	$OpenBSD: bpf.c,v 1.18 2007/01/04 22:17:48 krw Exp $	*/

/* BPF socket interface code, originally contributed by Archie Cobbs. */

/*
 * Copyright (c) 1995, 1996, 1998, 1999
 * The Internet Software Consortium.    All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */

#include "dhcpd.h"
#include <sys/ioctl.h>
#include <sys/uio.h>

#include <net/bpf.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>

#define BPF_FORMAT "/dev/bpf%d"

/*
 * Called by get_interface_list for each interface that's discovered.
 * Opens a packet filter for each interface and adds it to the select
 * mask.
 */
int
if_register_bpf(void)
{
	char filename[50];
	int sock, b;

	/* Open a BPF device */
	for (b = 0; 1; b++) {
		snprintf(filename, sizeof(filename), BPF_FORMAT, b);
		sock = open(filename, O_RDWR, 0);
		if (sock < 0) {
			if (errno == EBUSY)
				continue;
			else
				error("Can't find free bpf: %m");
		} else
			break;
	}

	/* Set the BPF device to point at this interface. */
	if (ioctl(sock, BIOCSETIF, ifi->ifp) < 0)
		error("Can't attach interface %s to bpf device %s: %m",
		    ifi->name, filename);

	return (sock);
}

void
if_register_send(void)
{
	int sock, on = 1;

	/*
	 * If we're using the bpf API for sending and receiving, we
	 * don't need to register this interface twice.
	 */
	ifi->wfdesc = ifi->rfdesc;

	/*
	 * Use raw socket for unicast send.
	 */
	if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) == -1)
		error("socket(SOCK_RAW): %m");
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &on,
	    sizeof(on)) == -1)
		error("setsockopt(IP_HDRINCL): %m");
	ifi->ufdesc = sock;
}

/*
 * Packet filter program...
 *
 * XXX: Changes to the filter program may require changes to the
 * constant offsets used in if_register_send to patch the BPF program!
 */
struct bpf_insn dhcp_bpf_filter[] = {
	/* Make sure this is an IP packet... */
	BPF_STMT(BPF_LD + BPF_H + BPF_ABS, 12),
	BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, ETHERTYPE_IP, 0, 8),

	/* Make sure it's a UDP packet... */
	BPF_STMT(BPF_LD + BPF_B + BPF_ABS, 23),
	BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, IPPROTO_UDP, 0, 6),

	/* Make sure this isn't a fragment... */
	BPF_STMT(BPF_LD + BPF_H + BPF_ABS, 20),
	BPF_JUMP(BPF_JMP + BPF_JSET + BPF_K, 0x1fff, 4, 0),

	/* Get the IP header length... */
	BPF_STMT(BPF_LDX + BPF_B + BPF_MSH, 14),

	/* Make sure it's to the right port... */
	BPF_STMT(BPF_LD + BPF_H + BPF_IND, 16),
	BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 67, 0, 1),		/* patch */

	/* If we passed all the tests, ask for the whole packet. */
	BPF_STMT(BPF_RET+BPF_K, (u_int)-1),

	/* Otherwise, drop it. */
	BPF_STMT(BPF_RET+BPF_K, 0),
};

int dhcp_bpf_filter_len = sizeof(dhcp_bpf_filter) / sizeof(struct bpf_insn);

/*
 * Packet write filter program:
 * 'ip and udp and src port bootps and dst port (bootps or bootpc)'
 */
struct bpf_insn dhcp_bpf_wfilter[] = {
	BPF_STMT(BPF_LD + BPF_B + BPF_IND, 14),
	BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, (IPVERSION << 4) + 5, 0, 12),

	/* Make sure this is an IP packet... */
	BPF_STMT(BPF_LD + BPF_H + BPF_ABS, 12),
	BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, ETHERTYPE_IP, 0, 10),

	/* Make sure it's a UDP packet... */
	BPF_STMT(BPF_LD + BPF_B + BPF_ABS, 23),
	BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, IPPROTO_UDP, 0, 8),

	/* Make sure this isn't a fragment... */
	BPF_STMT(BPF_LD + BPF_H + BPF_ABS, 20),
	BPF_JUMP(BPF_JMP + BPF_JSET + BPF_K, 0x1fff, 6, 0),	/* patched */

	/* Get the IP header length... */
	BPF_STMT(BPF_LDX + BPF_B + BPF_MSH, 14),

	/* Make sure it's from the right port... */
	BPF_STMT(BPF_LD + BPF_H + BPF_IND, 14),
	BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 68, 0, 3),

	/* Make sure it is to the right ports ... */
	BPF_STMT(BPF_LD + BPF_H + BPF_IND, 16),
	BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 67, 0, 1),

	/* If we passed all the tests, ask for the whole packet. */
	BPF_STMT(BPF_RET+BPF_K, (u_int)-1),

	/* Otherwise, drop it. */
	BPF_STMT(BPF_RET+BPF_K, 0),
};

int dhcp_bpf_wfilter_len = sizeof(dhcp_bpf_wfilter) / sizeof(struct bpf_insn);

void
if_register_receive(void)
{
	struct bpf_version v;
	struct bpf_program p;
	int flag = 1, sz;

	/* Open a BPF device and hang it on this interface... */
	ifi->rfdesc = if_register_bpf();

	/* Make sure the BPF version is in range... */
	if (ioctl(ifi->rfdesc, BIOCVERSION, &v) < 0)
		error("Can't get BPF version: %m");

	if (v.bv_major != BPF_MAJOR_VERSION ||
	    v.bv_minor < BPF_MINOR_VERSION)
		error("Kernel BPF version out of range - recompile dhcpd!");

	/*
	 * Set immediate mode so that reads return as soon as a packet
	 * comes in, rather than waiting for the input buffer to fill
	 * with packets.
	 */
	if (ioctl(ifi->rfdesc, BIOCIMMEDIATE, &flag) < 0)
		error("Can't set immediate mode on bpf device: %m");

	if (ioctl(ifi->rfdesc, BIOCSFILDROP, &flag) < 0)
		error("Can't set filter-drop mode on bpf device: %m");

	/* Get the required BPF buffer length from the kernel. */
	if (ioctl(ifi->rfdesc, BIOCGBLEN, &sz) < 0)
		error("Can't get bpf buffer length: %m");
	ifi->rbuf_max = sz;
	ifi->rbuf = malloc(ifi->rbuf_max);
	if (!ifi->rbuf)
		error("Can't allocate %lu bytes for bpf input buffer.",
		    (unsigned long)ifi->rbuf_max);
	ifi->rbuf_offset = 0;
	ifi->rbuf_len = 0;

	/* Set up the bpf filter program structure. */
	p.bf_len = dhcp_bpf_filter_len;
	p.bf_insns = dhcp_bpf_filter;

	/* Patch the server port into the BPF program...
	 *
	 * XXX: changes to filter program may require changes to the
	 * insn number(s) used below!
	 */
	dhcp_bpf_filter[8].k = LOCAL_PORT;

	if (ioctl(ifi->rfdesc, BIOCSETF, &p) < 0)
		error("Can't install packet filter program: %m");

	/* Set up the bpf write filter program structure. */
	p.bf_len = dhcp_bpf_wfilter_len;
	p.bf_insns = dhcp_bpf_wfilter;

	if (dhcp_bpf_wfilter[7].k == 0x1fff)
		dhcp_bpf_wfilter[7].k = htons(IP_MF|IP_OFFMASK);

	if (ioctl(ifi->rfdesc, BIOCSETWF, &p) < 0)
		error("Can't install write filter program: %m");

	if (ioctl(ifi->rfdesc, BIOCLOCK, NULL) < 0)
		error("Cannot lock bpf");
}

ssize_t
send_packet(size_t len, struct in_addr from, struct sockaddr_in *to,
    struct hardware *hto)
{
#define IOVCNT		2
	unsigned char buf[256];
	struct iovec iov[IOVCNT];
	struct msghdr msg;
	int result, bufp = 0;

	if (to->sin_addr.s_addr == INADDR_BROADCAST) {
		assemble_hw_header(buf, &bufp, hto);
	}

	assemble_udp_ip_header(buf, &bufp, from.s_addr,
	    to->sin_addr.s_addr, to->sin_port,
	    (unsigned char *)&client->packet,
	    len);

	iov[0].iov_base = (char *)buf;
	iov[0].iov_len = bufp;
	iov[1].iov_base = (char *)&client->packet;
	iov[1].iov_len = len;

	if (to->sin_addr.s_addr == INADDR_BROADCAST) {
		result = writev(ifi->wfdesc, iov, IOVCNT);
	} else {
		memset(&msg, 0, sizeof(msg));
		msg.msg_name = (struct sockaddr *)to;
		msg.msg_namelen = sizeof(*to);
		msg.msg_iov = iov;
		msg.msg_iovlen = IOVCNT;
		result = sendmsg(ifi->ufdesc, &msg, 0);
	}

	if (result == -1)
		warning("send_packet: %m");
	return (result);
}

ssize_t
receive_packet(struct sockaddr_in *from, struct hardware *hfrom)
{
	int length = 0, offset = 0;
	struct bpf_hdr hdr;

	/*
	 * All this complexity is because BPF doesn't guarantee that
	 * only one packet will be returned at a time.  We're getting
	 * what we deserve, though - this is a terrible abuse of the BPF
	 * interface.  Sigh.
	 */

	/* Process packets until we get one we can return or until we've
	 * done a read and gotten nothing we can return...
	 */
	do {
		/* If the buffer is empty, fill it. */
		if (ifi->rbuf_offset == ifi->rbuf_len) {
			length = read(ifi->rfdesc, ifi->rbuf, ifi->rbuf_max);
			if (length <= 0)
				return (length);
			ifi->rbuf_offset = 0;
			ifi->rbuf_len = BPF_WORDALIGN(length);
		}

		/*
		 * If there isn't room for a whole bpf header, something
		 * went wrong, but we'll ignore it and hope it goes
		 * away... XXX
		 */
		if (ifi->rbuf_len - ifi->rbuf_offset < sizeof(hdr)) {
			ifi->rbuf_offset = ifi->rbuf_len;
			continue;
		}

		/* Copy out a bpf header... */
		memcpy(&hdr, &ifi->rbuf[ifi->rbuf_offset], sizeof(hdr));

		/*
		 * If the bpf header plus data doesn't fit in what's
		 * left of the buffer, stick head in sand yet again...
		 */
		if (ifi->rbuf_offset + hdr.bh_hdrlen + hdr.bh_caplen >
		    ifi->rbuf_len) {
			ifi->rbuf_offset = ifi->rbuf_len;
			continue;
		}

		/*
		 * If the captured data wasn't the whole packet, or if
		 * the packet won't fit in the input buffer, all we can
		 * do is drop it.
		 */
		if (hdr.bh_caplen != hdr.bh_datalen) {
			ifi->rbuf_offset = BPF_WORDALIGN(
			    ifi->rbuf_offset + hdr.bh_hdrlen +
			    hdr.bh_caplen);
			continue;
		}

		/* Skip over the BPF header... */
		ifi->rbuf_offset += hdr.bh_hdrlen;

		/* Decode the physical header... */
		offset = decode_hw_header(ifi->rbuf, ifi->rbuf_offset, hfrom);

		/*
		 * If a physical layer checksum failed (dunno of any
		 * physical layer that supports this, but WTH), skip
		 * this packet.
		 */
		if (offset < 0) {
			ifi->rbuf_offset = BPF_WORDALIGN(
			    ifi->rbuf_offset + hdr.bh_caplen);
			continue;
		}
		ifi->rbuf_offset += offset;
		hdr.bh_caplen -= offset;

		/* Decode the IP and UDP headers... */
		offset = decode_udp_ip_header(ifi->rbuf,
		    ifi->rbuf_offset, from, NULL, hdr.bh_caplen);

		/* If the IP or UDP checksum was bad, skip the packet... */
		if (offset < 0) {
			ifi->rbuf_offset = BPF_WORDALIGN(
			    ifi->rbuf_offset + hdr.bh_caplen);
			continue;
		}
		ifi->rbuf_offset += offset;
		hdr.bh_caplen -= offset;

		/*
		 * If there's not enough room to stash the packet data,
		 * we have to skip it (this shouldn't happen in real
		 * life, though).
		 */
		if (hdr.bh_caplen > sizeof(client->packet)) {
			ifi->rbuf_offset = BPF_WORDALIGN(
			    ifi->rbuf_offset + hdr.bh_caplen);
			continue;
		}

		/* Copy out the data in the packet... */
		bzero(&client->packet, sizeof(client->packet));
		memcpy(&client->packet, ifi->rbuf + ifi->rbuf_offset,
		    hdr.bh_caplen);
		ifi->rbuf_offset = BPF_WORDALIGN(ifi->rbuf_offset +
		    hdr.bh_caplen);
		return (hdr.bh_caplen);
	} while (!length);
	return (0);
}
