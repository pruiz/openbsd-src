/*	$OpenBSD: in6_gif.c,v 1.27 2008/09/28 15:25:32 jsing Exp $	*/
/*	$KAME: in6_gif.c,v 1.43 2001/01/22 07:27:17 itojun Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
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

#include "pf.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/mbuf.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/protosw.h>

#include <net/if.h>
#include <net/route.h>

#if NPF > 0
#include <net/pfvar.h>
#endif

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip_ipsp.h>

#ifdef INET
#include <netinet/ip.h>
#endif

#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>
#include <netinet6/in6_gif.h>

#ifdef INET6
#include <netinet/ip6.h>
#endif

#include <netinet/ip_ecn.h>

#include <net/if_gif.h>

#include "bridge.h"

#ifndef offsetof
#define offsetof(s, e) ((int)&((s *)0)->e)
#endif

/*
 * family - family of the packet to be encapsulate.
 */
int
in6_gif_output(struct ifnet *ifp, int family, struct mbuf *m)
{
	struct gif_softc *sc = (struct gif_softc*)ifp;
	struct sockaddr_in6 *dst = (struct sockaddr_in6 *)&sc->gif_ro6.ro_dst;
	struct sockaddr_in6 *sin6_src = (struct sockaddr_in6 *)sc->gif_psrc;
	struct sockaddr_in6 *sin6_dst = (struct sockaddr_in6 *)sc->gif_pdst;
	struct tdb tdb;
	struct xformsw xfs;
	int error;
	struct mbuf *mp;

	if (sin6_src == NULL || sin6_dst == NULL ||
	    sin6_src->sin6_family != AF_INET6 ||
	    sin6_dst->sin6_family != AF_INET6) {
		m_freem(m);
		return EAFNOSUPPORT;
	}

	/* setup dummy tdb.  it highly depends on ipip_output() code. */
	bzero(&tdb, sizeof(tdb));
	bzero(&xfs, sizeof(xfs));
	tdb.tdb_src.sin6.sin6_family = AF_INET6;
	tdb.tdb_src.sin6.sin6_len = sizeof(struct sockaddr_in6);
	tdb.tdb_src.sin6.sin6_addr = sin6_src->sin6_addr;
	tdb.tdb_dst.sin6.sin6_family = AF_INET6;
	tdb.tdb_dst.sin6.sin6_len = sizeof(struct sockaddr_in6);
	tdb.tdb_dst.sin6.sin6_addr = sin6_dst->sin6_addr;
	tdb.tdb_xform = &xfs;
	xfs.xf_type = -1;	/* not XF_IP4 */

	switch (family) {
#ifdef INET
	case AF_INET:
		break;
#endif
#ifdef INET6
	case AF_INET6:
		break;
#endif
#if NBRIDGE > 0
	case AF_LINK:
		break;
#endif /* NBRIDGE */
	default:
#ifdef DEBUG
		printf("in6_gif_output: warning: unknown family %d passed\n",
			family);
#endif
		m_freem(m);
		return EAFNOSUPPORT;
	}
	
	/* encapsulate into IPv6 packet */
	mp = NULL;
#if NBRIDGE > 0
	if (family == AF_LINK)
		error = etherip_output(m, &tdb, &mp, 0, 0);
	else
#endif /* NBRIDGE */
	error = ipip_output(m, &tdb, &mp, 0, 0);
	if (error)
	        return error;
	else if (mp == NULL)
	        return EFAULT;

	m = mp;

	/* See if out cached route remains the same */
	if (dst->sin6_family != sin6_dst->sin6_family ||
	     !IN6_ARE_ADDR_EQUAL(&dst->sin6_addr, &sin6_dst->sin6_addr)) {
		/* cache route doesn't match */
		bzero(dst, sizeof(*dst));
		dst->sin6_family = sin6_dst->sin6_family;
		dst->sin6_len = sizeof(struct sockaddr_in6);
		dst->sin6_addr = sin6_dst->sin6_addr;
		if (sc->gif_ro6.ro_rt) {
			RTFREE(sc->gif_ro6.ro_rt);
			sc->gif_ro6.ro_rt = NULL;
		}
	}

	if (sc->gif_ro6.ro_rt == NULL) {
		rtalloc((struct route *)&sc->gif_ro6);
		if (sc->gif_ro6.ro_rt == NULL) {
			m_freem(m);
			return ENETUNREACH;
		}
	}
	
	/*
	 * force fragmentation to minimum MTU, to avoid path MTU discovery.
	 * it is too painful to ask for resend of inner packet, to achieve
	 * path MTU discovery for encapsulated packets.
	 */
#if NPF > 0
	pf_pkt_addr_changed(m);
#endif
	error = ip6_output(m, 0, &sc->gif_ro6, IPV6_MINMTU, 0, NULL, NULL);

	return error;
}

int in6_gif_input(struct mbuf **mp, int *offp, int proto)
{
	struct mbuf *m = *mp;
	struct gif_softc *sc;
	struct ifnet *gifp = NULL;
	struct ip6_hdr *ip6;

	/* XXX What if we run transport-mode IPsec to protect gif tunnel ? */
	if (m->m_flags & (M_AUTH | M_CONF))
	        goto inject;

	ip6 = mtod(m, struct ip6_hdr *);

#define satoin6(sa)	(((struct sockaddr_in6 *)(sa))->sin6_addr)
	LIST_FOREACH(sc, &gif_softc_list, gif_list) {
		if (sc->gif_psrc == NULL || sc->gif_pdst == NULL ||
		    sc->gif_psrc->sa_family != AF_INET6 ||
		    sc->gif_pdst->sa_family != AF_INET6) {
			continue;
		}

		if ((sc->gif_if.if_flags & IFF_UP) == 0)
			continue;

		if (IN6_ARE_ADDR_EQUAL(&satoin6(sc->gif_psrc), &ip6->ip6_dst) &&
		    IN6_ARE_ADDR_EQUAL(&satoin6(sc->gif_pdst), &ip6->ip6_src)) {
			gifp = &sc->gif_if;
			break;
		}
	}

	if (gifp) {
	        m->m_pkthdr.rcvif = gifp;
		gifp->if_ipackets++;
		gifp->if_ibytes += m->m_pkthdr.len;
		ipip_input(m, *offp, gifp);
		return IPPROTO_DONE;
	}

inject:
	/* No GIF tunnel configured */
	ip4_input6(&m, offp, 0); /* XXX last argument ignored */
	return IPPROTO_DONE;
}
