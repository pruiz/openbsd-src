/*	$OpenBSD: mroute6.c,v 1.15 2015/01/16 06:40:10 deraadt Exp $	*/

/*
 * Copyright (C) 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1989 Stephen Deering
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Stephen Deering of Stanford University.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)mroute.c	8.2 (Berkeley) 4/28/95
 */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/sysctl.h>

#include <net/if.h>
#include <net/if_var.h>

#include <netinet/in.h>

#define _KERNEL 1
#include <netinet6/ip6_mroute.h>
#undef _KERNEL

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include "netstat.h"

#define	WID_ORG	(lflag ? 39 : (nflag ? 29 : 18)) /* width of origin column */
#define	WID_GRP	(lflag ? 18 : (nflag ? 16 : 18)) /* width of group column */

void
mroute6pr(u_long mfcaddr, u_long mifaddr)
{
	int banner_printed, saved_nflag, waitings, i;
	struct mf6c *mf6ctable[MF6CTBLSIZ], *mfcp;
	struct mif6 mif6table[MAXMIFS], *mifp;
	struct rtdetq rte, *rtep;
	mifi_t maxmif = 0, mifi;
	struct mf6c mfc;
	u_int mrtproto;
	int mib[] = { CTL_NET, PF_INET6, IPPROTO_IPV6, IPV6CTL_MRTPROTO };
	size_t len = sizeof(int);

	if (sysctl(mib, sizeof(mib) / sizeof(mib[0]),
	    &mrtproto, &len, NULL, 0) == -1) {
		if (errno != ENOPROTOOPT)
			warn("mroute");
		return;
	}
	switch (mrtproto) {
	case 0:
		printf("no IPv6 multicast routing compiled into this system\n");
		return;
	case IPPROTO_PIM:
		break;
	default:
		printf("IPv6 multicast routing protocol %u, unknown\n",
		    mrtproto);
		return;
	}

	if (mfcaddr == 0) {
		printf("mf6ctable: symbol not in namelist\n");
		return;
	}
	if (mifaddr == 0) {
		printf("miftable: symbol not in namelist\n");
		return;
	}

	saved_nflag = nflag;
	nflag = 1;

	kread(mifaddr, &mif6table, sizeof(mif6table));
	banner_printed = 0;
	for (mifi = 0, mifp = mif6table; mifi < MAXMIFS; ++mifi, ++mifp) {
		struct ifnet ifnet;
		char ifname[IFNAMSIZ];

		if (mifp->m6_ifp == NULL)
			continue;

		kread((u_long)mifp->m6_ifp, &ifnet, sizeof(ifnet));
		maxmif = mifi;
		if (!banner_printed) {
			printf("\nIPv6 Multicast Interface Table\n"
			    " Mif   Rate   PhyIF   "
			    "Pkts-In   Pkts-Out\n");
			banner_printed = 1;
		}

		printf("  %2u   %4d",
		    mifi, mifp->m6_rate_limit);
		printf("   %5s", (mifp->m6_flags & MIFF_REGISTER) ?
		    "reg0" : if_indextoname(ifnet.if_index, ifname));

		printf(" %9llu  %9llu\n", mifp->m6_pkt_in, mifp->m6_pkt_out);
	}
	if (!banner_printed)
		printf("IPv6 Multicast Interface Table is empty\n");

	kread(mfcaddr, &mf6ctable, sizeof(mf6ctable));
	banner_printed = 0;
	for (i = 0; i < MF6CTBLSIZ; ++i) {
		mfcp = mf6ctable[i];
		while (mfcp) {
			kread((u_long)mfcp, &mfc, sizeof(mfc));
			if (!banner_printed) {
				printf("\nIPv6 Multicast Forwarding Cache\n");
				printf(" %-*.*s %-*.*s %s",
				    WID_ORG, WID_ORG, "Origin",
				    WID_GRP, WID_GRP, "Group",
				    "  Packets Waits In-Mif  Out-Mifs\n");
				banner_printed = 1;
			}

			printf(" %-*.*s", WID_ORG, WID_ORG,
			    routename6(&mfc.mf6c_origin));
			printf(" %-*.*s", WID_GRP, WID_GRP,
			    routename6(&mfc.mf6c_mcastgrp));
			printf(" %9llu", mfc.mf6c_pkt_cnt);

			for (waitings = 0, rtep = mfc.mf6c_stall; rtep; ) {
				waitings++;
				kread((u_long)rtep, &rte, sizeof(rte));
				rtep = rte.next;
			}
			printf("   %3d", waitings);

			if (mfc.mf6c_parent == MF6C_INCOMPLETE_PARENT)
				printf("  ---   ");
			else
				printf("  %3d   ", mfc.mf6c_parent);
			for (mifi = 0; mifi <= MAXMIFS; mifi++) {
				if (IF_ISSET(mifi, &mfc.mf6c_ifset))
					printf(" %u", mifi);
			}
			printf("\n");

			mfcp = mfc.mf6c_next;
		}
	}
	if (!banner_printed)
		printf("IPv6 Multicast Routing Table is empty");

	printf("\n");
	nflag = saved_nflag;
}

void
mrt6_stats(void)
{
	struct mrt6stat mrt6stat;
	u_int mrt6proto;
	int mib[] = { CTL_NET, PF_INET6, IPPROTO_IPV6, IPV6CTL_MRTPROTO };
	int mib2[] = { CTL_NET, PF_INET6, IPPROTO_IPV6, IPV6CTL_MRTSTATS };
	size_t len = sizeof(int);

	if (sysctl(mib, sizeof(mib) / sizeof(mib[0]),
	    &mrt6proto, &len, NULL, 0) == -1) {
		if (errno != ENOPROTOOPT)
			warn("mroute");
		return;
	}
	switch (mrt6proto) {
	case 0:
		printf("no IPv6 multicast routing compiled into this system\n");
		return;
	case IPPROTO_PIM:
		break;
	default:
		printf("IPv6 multicast routing protocol %u, unknown\n",
		    mrt6proto);
		return;
	}

	len = sizeof(mrt6stat);
	if (sysctl(mib2, sizeof(mib2) / sizeof(mib2[0]),
	    &mrt6stat, &len, NULL, 0) == -1) {
		if (errno != ENOPROTOOPT)
			warn("mroute");
		return;
	}

	printf("multicast forwarding:\n");
	printf("\t%llu multicast forwarding cache lookup%s\n",
	    mrt6stat.mrt6s_mfc_lookups, plural(mrt6stat.mrt6s_mfc_lookups));
	printf("\t%llu multicast forwarding cache miss%s\n",
	    mrt6stat.mrt6s_mfc_misses, plurales(mrt6stat.mrt6s_mfc_misses));
	printf("\t%llu upcall%s to mrouted\n",
	    mrt6stat.mrt6s_upcalls, plural(mrt6stat.mrt6s_upcalls));
	printf("\t%llu upcall queue overflow%s\n",
	    mrt6stat.mrt6s_upq_ovflw, plural(mrt6stat.mrt6s_upq_ovflw));
	printf("\t%llu upcall%s dropped due to full socket buffer\n",
	    mrt6stat.mrt6s_upq_sockfull, plural(mrt6stat.mrt6s_upq_sockfull));
	printf("\t%llu cache cleanup%s\n",
	    mrt6stat.mrt6s_cache_cleanups, plural(mrt6stat.mrt6s_cache_cleanups));
	printf("\t%llu datagram%s with no route for origin\n",
	    mrt6stat.mrt6s_no_route, plural(mrt6stat.mrt6s_no_route));
	printf("\t%llu datagram%s arrived with bad tunneling\n",
	    mrt6stat.mrt6s_bad_tunnel, plural(mrt6stat.mrt6s_bad_tunnel));
	printf("\t%llu datagram%s could not be tunneled\n",
	    mrt6stat.mrt6s_cant_tunnel, plural(mrt6stat.mrt6s_cant_tunnel));
	printf("\t%llu datagram%s arrived on wrong interface\n",
	    mrt6stat.mrt6s_wrong_if, plural(mrt6stat.mrt6s_wrong_if));
	printf("\t%llu datagram%s selectively dropped\n",
	    mrt6stat.mrt6s_drop_sel, plural(mrt6stat.mrt6s_drop_sel));
	printf("\t%llu datagram%s dropped due to queue overflow\n",
	    mrt6stat.mrt6s_q_overflow, plural(mrt6stat.mrt6s_q_overflow));
	printf("\t%llu datagram%s dropped for being too large\n",
	    mrt6stat.mrt6s_pkt2large, plural(mrt6stat.mrt6s_pkt2large));
}
