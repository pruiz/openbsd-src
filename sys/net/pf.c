/*	$OpenBSD: pf.c,v 1.293 2003/01/01 16:09:29 henning Exp $ */

/*
 * Copyright (c) 2001 Daniel Hartmeier
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Effort sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F30602-01-2-0537.
 *
 */

#include "bpfilter.h"
#include "pflog.h"
#include "pfsync.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/filio.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/kernel.h>
#include <sys/time.h>
#include <sys/pool.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/bpf.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/udp_var.h>

#include <dev/rndvar.h>
#include <net/pfvar.h>
#include <net/if_pflog.h>
#include <net/if_pfsync.h>

#ifdef INET6
#include <netinet/ip6.h>
#include <netinet/in_pcb.h>
#include <netinet/icmp6.h>
#endif /* INET6 */

#ifdef ALTQ
#include <altq/if_altq.h>
#endif


#define DPFPRINTF(n, x)	if (pf_status.debug >= (n)) printf x
struct pf_state_tree;

/*
 * Global variables
 */

struct pf_anchorqueue	 pf_anchors;
struct pf_ruleset	 pf_main_ruleset;
struct pf_altqqueue	 pf_altqs[2];
struct pf_palist	 pf_pabuf;
struct pf_altqqueue	*pf_altqs_active;
struct pf_altqqueue	*pf_altqs_inactive;
struct pf_status	 pf_status;
struct ifnet		*status_ifp;

u_int32_t		 ticket_altqs_active;
u_int32_t		 ticket_altqs_inactive;
u_int32_t		 ticket_pabuf;

/* Timeouts */
int			 pftm_tcp_first_packet = 120;	/* First TCP packet */
int			 pftm_tcp_opening = 30;		/* No response yet */
int			 pftm_tcp_established = 24*60*60;  /* established */
int			 pftm_tcp_closing = 15 * 60;	/* Half closed */
int			 pftm_tcp_fin_wait = 45;	/* Got both FINs */
int			 pftm_tcp_closed = 90;		/* Got a RST */

int			 pftm_udp_first_packet = 60;	/* First UDP packet */
int			 pftm_udp_single = 30;		/* Unidirectional */
int			 pftm_udp_multiple = 60;	/* Bidirectional */

int			 pftm_icmp_first_packet = 20;	/* First ICMP packet */
int			 pftm_icmp_error_reply = 10;	/* Got error response */

int			 pftm_other_first_packet = 60;	/* First packet */
int			 pftm_other_single = 30;	/* Unidirectional */
int			 pftm_other_multiple = 60;	/* Bidirectional */

int			 pftm_frag = 30;		/* Fragment expire */

int			 pftm_interval = 10;		/* expire interval */
struct timeout		 pf_expire_to;			/* expire timeout */

int			*pftm_timeouts[PFTM_MAX] = { &pftm_tcp_first_packet,
				&pftm_tcp_opening, &pftm_tcp_established,
				&pftm_tcp_closing, &pftm_tcp_fin_wait,
				&pftm_tcp_closed, &pftm_udp_first_packet,
				&pftm_udp_single, &pftm_udp_multiple,
				&pftm_icmp_first_packet, &pftm_icmp_error_reply,
				&pftm_other_first_packet, &pftm_other_single,
				&pftm_other_multiple, &pftm_frag,
				&pftm_interval };


struct pool		 pf_tree_pl, pf_rule_pl, pf_addr_pl;
struct pool		 pf_state_pl, pf_altq_pl, pf_pooladdr_pl;
struct pool		 pfr_ktable_pl, pfr_kentry_pl;

void			 pf_addrcpy(struct pf_addr *, struct pf_addr *,
			    sa_family_t);
int			 pf_insert_state(struct pf_state *);
struct pf_state		*pf_find_state(struct pf_state_tree *,
			    struct pf_tree_node *);
void			 pf_purge_expired_states(void);
void			 pf_purge_timeout(void *);
void			 pf_dynaddr_update(void *);
void			 pf_print_host(struct pf_addr *, u_int16_t, u_int8_t);
void			 pf_print_state(struct pf_state *);
void			 pf_print_flags(u_int8_t);

u_int16_t		 pf_cksum_fixup(u_int16_t, u_int16_t, u_int16_t,
			    u_int8_t);
void			 pf_change_ap(struct pf_addr *, u_int16_t *,
			    u_int16_t *, u_int16_t *, struct pf_addr *,
			    u_int16_t, u_int8_t, sa_family_t);
void			 pf_change_a(u_int32_t *, u_int16_t *, u_int32_t,
			    u_int8_t);
#ifdef INET6
void			 pf_change_a6(struct pf_addr *, u_int16_t *,
			    struct pf_addr *, u_int8_t);
#endif /* INET6 */
void			 pf_change_icmp(struct pf_addr *, u_int16_t *,
			    struct pf_addr *, struct pf_addr *, u_int16_t,
			    u_int16_t *, u_int16_t *, u_int16_t *,
			    u_int16_t *, u_int8_t, sa_family_t);
void			 pf_send_reset(int, struct tcphdr *,
			    struct pf_pdesc *, sa_family_t, u_int8_t,
			    struct pf_rule *);
void			 pf_send_icmp(struct mbuf *, u_int8_t, u_int8_t,
			    sa_family_t, struct pf_rule *);
struct pf_rule		*pf_match_translation(int, struct ifnet *, u_int8_t,
			    struct pf_addr *, u_int16_t, struct pf_addr *,
			    u_int16_t, sa_family_t, int);
struct pf_rule		*pf_get_translation(int, struct ifnet *, u_int8_t,
			    struct pf_addr *, u_int16_t,
			    struct pf_addr *, u_int16_t,
			    struct pf_addr *, u_int16_t *, sa_family_t);
int			 pf_test_tcp(struct pf_rule **, int, struct ifnet *,
			    struct mbuf *, int, int, void *, struct pf_pdesc *);
int			 pf_test_udp(struct pf_rule **, int, struct ifnet *,
			    struct mbuf *, int, int, void *, struct pf_pdesc *);
int			 pf_test_icmp(struct pf_rule **, int, struct ifnet *,
			    struct mbuf *, int, int, void *, struct pf_pdesc *);
int			 pf_test_other(struct pf_rule **, int, struct ifnet *,
			    struct mbuf *, void *, struct pf_pdesc *);
int			 pf_test_fragment(struct pf_rule **, int,
			    struct ifnet *, struct mbuf *, void *,
			    struct pf_pdesc *);
int			 pf_test_state_tcp(struct pf_state **, int,
			    struct ifnet *, struct mbuf *, int, int,
			    void *, struct pf_pdesc *);
int			 pf_test_state_udp(struct pf_state **, int,
			    struct ifnet *, struct mbuf *, int, int,
			    void *, struct pf_pdesc *);
int			 pf_test_state_icmp(struct pf_state **, int,
			    struct ifnet *, struct mbuf *, int, int,
			    void *, struct pf_pdesc *);
int			 pf_test_state_other(struct pf_state **, int,
			    struct ifnet *, struct pf_pdesc *);
void			*pf_pull_hdr(struct mbuf *, int, void *, int,
			    u_short *, u_short *, sa_family_t);
void			 pf_calc_skip_steps(struct pf_rulequeue *);

#ifdef INET6
void			 pf_poolmask(struct pf_addr *, struct pf_addr*,
			    struct pf_addr *, struct pf_addr *, u_int8_t);
void			 pf_addr_inc(struct pf_addr *, sa_family_t);
#endif /* INET6 */

void			 pf_hash(struct pf_addr *, struct pf_addr *,
			    struct pf_poolhashkey *, sa_family_t);
int			 pf_map_addr(u_int8_t, struct pf_pool *,
			    struct pf_addr *, struct pf_addr *,
			    struct pf_addr *);
int			 pf_get_sport(sa_family_t, u_int8_t,
			    struct pf_pool *, struct pf_addr *, u_int16_t,
			    struct pf_addr *, u_int16_t, struct pf_addr *,
			    u_int16_t*, u_int16_t, u_int16_t);
int			 pf_normalize_tcp(int, struct ifnet *, struct mbuf *,
			    int, int, void *, struct pf_pdesc *);
void			 pf_route(struct mbuf **, struct pf_rule *, int,
			    struct ifnet *, struct pf_state *);
void			 pf_route6(struct mbuf **, struct pf_rule *, int,
			    struct ifnet *, struct pf_state *);
int			 pf_socket_lookup(uid_t *, gid_t *, int, sa_family_t,
			    int, struct pf_pdesc *);

struct pf_pool_limit pf_pool_limits[PF_LIMIT_MAX] =
    { { &pf_state_pl, PFSTATE_HIWAT }, { &pf_frent_pl, PFFRAG_FRENT_HIWAT } };

#define	STATE_TRANSLATE(s) \
	(s)->lan.addr.addr32[0] != (s)->gwy.addr.addr32[0] || \
	((s)->af == AF_INET6 && \
	((s)->lan.addr.addr32[1] != (s)->gwy.addr.addr32[1] || \
	(s)->lan.addr.addr32[2] != (s)->gwy.addr.addr32[2] || \
	(s)->lan.addr.addr32[3] != (s)->gwy.addr.addr32[3])) || \
	(s)->lan.port != (s)->gwy.port

#define TIMEOUT(r,i) \
	(((r) && (r)->timeout[(i)]) ? (r)->timeout[(i)] : *pftm_timeouts[(i)])

static __inline int pf_state_compare(struct pf_tree_node *,
			struct pf_tree_node *);

struct pf_state_tree tree_lan_ext, tree_ext_gwy;
RB_GENERATE(pf_state_tree, pf_tree_node, entry, pf_state_compare);

static __inline int
pf_state_compare(struct pf_tree_node *a, struct pf_tree_node *b)
{
	int	diff;

	if ((diff = a->proto - b->proto) != 0)
		return (diff);
	if ((diff = a->af - b->af) != 0)
		return (diff);
	switch (a->af) {
#ifdef INET
	case AF_INET:
		if (a->addr[0].addr32[0] > b->addr[0].addr32[0])
			return (1);
		if (a->addr[0].addr32[0] < b->addr[0].addr32[0])
			return (-1);
		if (a->addr[1].addr32[0] > b->addr[1].addr32[0])
			return (1);
		if (a->addr[1].addr32[0] < b->addr[1].addr32[0])
			return (-1);
		break;
#endif /* INET */
#ifdef INET6
	case AF_INET6:
		if (a->addr[0].addr32[0] > b->addr[0].addr32[0])
			return (1);
		if (a->addr[0].addr32[0] < b->addr[0].addr32[0])
			return (-1);
		if (a->addr[0].addr32[1] > b->addr[0].addr32[1])
			return (1);
		if (a->addr[0].addr32[1] < b->addr[0].addr32[1])
			return (-1);
		if (a->addr[0].addr32[2] > b->addr[0].addr32[2])
			return (1);
		if (a->addr[0].addr32[2] < b->addr[0].addr32[2])
			return (-1);
		if (a->addr[0].addr32[3] > b->addr[0].addr32[3])
			return (1);
		if (a->addr[0].addr32[3] < b->addr[0].addr32[3])
			return (-1);
		if (a->addr[1].addr32[0] > b->addr[1].addr32[0])
			return (1);
		if (a->addr[1].addr32[0] < b->addr[1].addr32[0])
			return (-1);
		if (a->addr[1].addr32[1] > b->addr[1].addr32[1])
			return (1);
		if (a->addr[1].addr32[1] < b->addr[1].addr32[1])
			return (-1);
		if (a->addr[1].addr32[2] > b->addr[1].addr32[2])
			return (1);
		if (a->addr[1].addr32[2] < b->addr[1].addr32[2])
			return (-1);
		if (a->addr[1].addr32[3] > b->addr[1].addr32[3])
			return (1);
		if (a->addr[1].addr32[3] < b->addr[1].addr32[3])
			return (-1);
		break;
#endif /* INET6 */
	}

	if ((diff = a->port[0] - b->port[0]) != 0)
		return (diff);
	if ((diff = a->port[1] - b->port[1]) != 0)
		return (diff);

	return (0);
}

#ifdef INET6
void
pf_addrcpy(struct pf_addr *dst, struct pf_addr *src, sa_family_t af)
{
	switch(af) {
#ifdef INET
	case AF_INET:
		dst->addr32[0] = src->addr32[0];
		break;
#endif /* INET */
	case AF_INET6:
		dst->addr32[0] = src->addr32[0];
		dst->addr32[1] = src->addr32[1];
		dst->addr32[2] = src->addr32[2];
		dst->addr32[3] = src->addr32[3];
		break;
	}
}
#endif

struct pf_state *
pf_find_state(struct pf_state_tree *tree, struct pf_tree_node *key)
{
	struct pf_tree_node	*k;

	pf_status.fcounters[FCNT_STATE_SEARCH]++;
	k = RB_FIND(pf_state_tree, tree, key);
	if (k)
		return (k->state);
	else
		return (NULL);
}

int
pf_insert_state(struct pf_state *state)
{
	struct pf_tree_node	*keya, *keyb;

	keya = pool_get(&pf_tree_pl, PR_NOWAIT);
	if (keya == NULL)
		return (-1);
	keya->state = state;
	keya->proto = state->proto;
	keya->af = state->af;
	PF_ACPY(&keya->addr[0], &state->lan.addr, state->af);
	keya->port[0] = state->lan.port;
	PF_ACPY(&keya->addr[1], &state->ext.addr, state->af);
	keya->port[1] = state->ext.port;

	/* Thou MUST NOT insert multiple duplicate keys */
	if (RB_INSERT(pf_state_tree, &tree_lan_ext, keya) != NULL) {
		if (pf_status.debug >= PF_DEBUG_MISC) {
			printf("pf: state insert failed: tree_lan_ext");
			printf(" lan: ");
			pf_print_host(&state->lan.addr, state->lan.port,
			    state->af);
			printf(" gwy: ");
			pf_print_host(&state->gwy.addr, state->gwy.port,
			    state->af);
			printf(" ext: ");
			pf_print_host(&state->ext.addr, state->ext.port,
			    state->af);
			printf("\n");
		}
		pool_put(&pf_tree_pl, keya);
		return (-1);
	}

	keyb = pool_get(&pf_tree_pl, PR_NOWAIT);
	if (keyb == NULL) {
		/* Need to pull out the other state */
		RB_REMOVE(pf_state_tree, &tree_lan_ext, keya);
		pool_put(&pf_tree_pl, keya);
		return (-1);
	}
	keyb->state = state;
	keyb->proto = state->proto;
	keyb->af = state->af;
	PF_ACPY(&keyb->addr[0], &state->ext.addr, state->af);
	keyb->port[0] = state->ext.port;
	PF_ACPY(&keyb->addr[1], &state->gwy.addr, state->af);
	keyb->port[1] = state->gwy.port;

	if (RB_INSERT(pf_state_tree, &tree_ext_gwy, keyb) != NULL) {
		if (pf_status.debug >= PF_DEBUG_MISC) {
			printf("pf: state insert failed: tree_ext_gwy");
			printf(" lan: ");
			pf_print_host(&state->lan.addr, state->lan.port,
			    state->af);
			printf(" gwy: ");
			pf_print_host(&state->gwy.addr, state->gwy.port,
			    state->af);
			printf(" ext: ");
			pf_print_host(&state->ext.addr, state->ext.port,
			    state->af);
			printf("\n");
		}
		RB_REMOVE(pf_state_tree, &tree_lan_ext, keya);
		pool_put(&pf_tree_pl, keya);
		pool_put(&pf_tree_pl, keyb);
		return (-1);
	}

	pf_status.fcounters[FCNT_STATE_INSERT]++;
	pf_status.states++;
#if NPFSYNC
	pfsync_insert_state(state);
#endif
	return (0);
}

void
pf_purge_timeout(void *arg)
{
	struct timeout	*to = arg;
	int		 s;

	s = splsoftnet();
	pf_purge_expired_states();
	pf_purge_expired_fragments();
	splx(s);

	timeout_add(to, pftm_interval * hz);
}

void
pf_purge_expired_states(void)
{
	struct pf_tree_node	*cur, *peer, *next;
	struct pf_tree_node	 key;

	for (cur = RB_MIN(pf_state_tree, &tree_ext_gwy); cur; cur = next) {
		next = RB_NEXT(pf_state_tree, &tree_ext_gwy, cur);

		if (cur->state->expire <= (unsigned)time.tv_sec) {
			RB_REMOVE(pf_state_tree, &tree_ext_gwy, cur);

			/* Need this key's peer (in the other tree) */
			key.state = cur->state;
			key.proto = cur->state->proto;
			key.af = cur->state->af;
			PF_ACPY(&key.addr[0], &cur->state->lan.addr,
			    cur->state->af);
			key.port[0] = cur->state->lan.port;
			PF_ACPY(&key.addr[1], &cur->state->ext.addr,
			    cur->state->af);
			key.port[1] = cur->state->ext.port;

			peer = RB_FIND(pf_state_tree, &tree_lan_ext, &key);
			KASSERT(peer);
			KASSERT(peer->state == cur->state);
			RB_REMOVE(pf_state_tree, &tree_lan_ext, peer);

#if NPFSYNC
			pfsync_delete_state(cur->state);
#endif
			if (cur->state->rule.ptr != NULL)
				cur->state->rule.ptr->states--;
			if (cur->state->nat_rule != NULL)
				cur->state->nat_rule->states--;
			pool_put(&pf_state_pl, cur->state);
			pool_put(&pf_tree_pl, cur);
			pool_put(&pf_tree_pl, peer);
			pf_status.fcounters[FCNT_STATE_REMOVALS]++;
			pf_status.states--;
		}
	}
}

int
pf_dynaddr_setup(struct pf_addr_wrap *aw, sa_family_t af)
{
	if (aw->addr_dyn == NULL)
		return (0);
	aw->addr_dyn = pool_get(&pf_addr_pl, PR_NOWAIT);
	if (aw->addr_dyn == NULL)
		return (1);
	bcopy(aw->addr.pfa.ifname, aw->addr_dyn->ifname,
	    sizeof(aw->addr_dyn->ifname));
	aw->addr_dyn->ifp = ifunit(aw->addr_dyn->ifname);
	if (aw->addr_dyn->ifp == NULL) {
		pool_put(&pf_addr_pl, aw->addr_dyn);
		aw->addr_dyn = NULL;
		return (1);
	}
	aw->addr_dyn->addr = &aw->addr;
	aw->addr_dyn->af = af;
	aw->addr_dyn->undefined = 1;
	aw->addr_dyn->hook_cookie = hook_establish(
	    aw->addr_dyn->ifp->if_addrhooks, 1,
	    pf_dynaddr_update, aw->addr_dyn);
	if (aw->addr_dyn->hook_cookie == NULL) {
		pool_put(&pf_addr_pl, aw->addr_dyn);
		aw->addr_dyn = NULL;
		return (1);
	}
	pf_dynaddr_update(aw->addr_dyn);
	return (0);
}

void
pf_dynaddr_update(void *p)
{
	struct pf_addr_dyn	*ad = (struct pf_addr_dyn *)p;
	struct ifaddr		*ia;
	int			 s, changed = 0;

	if (ad == NULL || ad->ifp == NULL)
		panic("pf_dynaddr_update");
	s = splsoftnet();
	TAILQ_FOREACH(ia, &ad->ifp->if_addrlist, ifa_list)
		if (ia->ifa_addr != NULL &&
		    ia->ifa_addr->sa_family == ad->af) {
			if (ad->af == AF_INET) {
				struct in_addr *a, *b;

				a = &ad->addr->v4;
				b = &((struct sockaddr_in *)ia->ifa_addr)
				    ->sin_addr;
				if (ad->undefined ||
				    memcmp(a, b, sizeof(*a))) {
					bcopy(b, a, sizeof(*a));
					changed = 1;
				}
			} else if (ad->af == AF_INET6) {
				struct in6_addr *a, *b;

				a = &ad->addr->v6;
				b = &((struct sockaddr_in6 *)ia->ifa_addr)
				    ->sin6_addr;
				if (ad->undefined ||
				    memcmp(a, b, sizeof(*a))) {
					bcopy(b, a, sizeof(*a));
					changed = 1;
				}
			}
			if (changed)
				ad->undefined = 0;
			break;
		}
	if (ia == NULL)
		ad->undefined = 1;
	splx(s);
}

void
pf_dynaddr_remove(struct pf_addr_wrap *aw)
{
	if (aw->addr_dyn == NULL)
		return;
	hook_disestablish(aw->addr_dyn->ifp->if_addrhooks,
	    aw->addr_dyn->hook_cookie);
	pool_put(&pf_addr_pl, aw->addr_dyn);
	aw->addr_dyn = NULL;
}

void
pf_dynaddr_copyout(struct pf_addr_wrap *aw)
{
	if (aw->addr_dyn == NULL)
		return;
	bcopy(aw->addr_dyn->ifname, aw->addr.pfa.ifname,
	    sizeof(aw->addr.pfa.ifname));
	aw->addr_dyn = (struct pf_addr_dyn *)1;
}

void
pf_print_host(struct pf_addr *addr, u_int16_t p, sa_family_t af)
{
	switch(af) {
#ifdef INET
	case AF_INET: {
		u_int32_t a = ntohl(addr->addr32[0]);
		printf("%u.%u.%u.%u", (a>>24)&255, (a>>16)&255,
		    (a>>8)&255, a&255);
		if (p) {
			p = ntohs(p);
			printf(":%u", p);
		}
		break;
	}
#endif /* INET */
#ifdef INET6
	case AF_INET6: {
		u_int16_t b;
		u_int8_t i, curstart = 255, curend = 0,
		    maxstart = 0, maxend = 0;
		for (i = 0; i < 8; i++) {
			if (!addr->addr16[i]) {
				if (curstart == 255)
					curstart = i;
				else
					curend = i;
			} else {
				if (curstart) {
					if ((curend - curstart) >
					    (maxend - maxstart)) {
						maxstart = curstart;
						maxend = curend;
						curstart = 255;
					}
				}
			}
		}
		for (i = 0; i < 8; i++) {
			if (i >= maxstart && i <= maxend) {
				if (maxend != 7) {
					if (i == maxstart)
						printf(":");
				} else {
					if (i == maxend)
						printf(":");
				}
			} else {
				b = ntohs(addr->addr16[i]);
				printf("%x", b);
				if (i < 7)
					printf(":");
			}
		}
		if (p) {
			p = ntohs(p);
			printf("[%u]", p);
		}
		break;
	}
#endif /* INET6 */
	}
}

void
pf_print_state(struct pf_state *s)
{
	switch (s->proto) {
	case IPPROTO_TCP:
		printf("TCP ");
		break;
	case IPPROTO_UDP:
		printf("UDP ");
		break;
	case IPPROTO_ICMP:
		printf("ICMP ");
		break;
	case IPPROTO_ICMPV6:
		printf("ICMPV6 ");
		break;
	default:
		printf("%u ", s->proto);
		break;
	}
	pf_print_host(&s->lan.addr, s->lan.port, s->af);
	printf(" ");
	pf_print_host(&s->gwy.addr, s->gwy.port, s->af);
	printf(" ");
	pf_print_host(&s->ext.addr, s->ext.port, s->af);
	printf(" [lo=%lu high=%lu win=%u modulator=%u]", s->src.seqlo,
	    s->src.seqhi, s->src.max_win, s->src.seqdiff);
	printf(" [lo=%lu high=%lu win=%u modulator=%u]", s->dst.seqlo,
	    s->dst.seqhi, s->dst.max_win, s->dst.seqdiff);
	printf(" %u:%u", s->src.state, s->dst.state);
}

void
pf_print_flags(u_int8_t f)
{
	if (f)
		printf(" ");
	if (f & TH_FIN)
		printf("F");
	if (f & TH_SYN)
		printf("S");
	if (f & TH_RST)
		printf("R");
	if (f & TH_PUSH)
		printf("P");
	if (f & TH_ACK)
		printf("A");
	if (f & TH_URG)
		printf("U");
	if (f & TH_ECE)
		printf("E");
	if (f & TH_CWR)
		printf("W");
}

#define	PF_SET_SKIP_STEPS(i)					\
	do {							\
		while (head[i] != cur) {			\
			head[i]->skip[i].ptr = cur;		\
			head[i] = TAILQ_NEXT(head[i], entries);	\
		}						\
	} while (0)

void
pf_calc_skip_steps(struct pf_rulequeue *rules)
{
	struct pf_rule *cur, *prev, *head[PF_SKIP_COUNT];
	int i;

	cur = TAILQ_FIRST(rules);
	prev = cur;
	for (i = 0; i < PF_SKIP_COUNT; ++i)
		head[i] = cur;
	while (cur != NULL) {

		if (cur->ifp != prev->ifp || cur->ifnot != prev->ifnot)
			PF_SET_SKIP_STEPS(PF_SKIP_IFP);
		if (cur->direction != prev->direction)
			PF_SET_SKIP_STEPS(PF_SKIP_DIR);
		if (cur->af != prev->af)
			PF_SET_SKIP_STEPS(PF_SKIP_AF);
		if (cur->proto != prev->proto)
			PF_SET_SKIP_STEPS(PF_SKIP_PROTO);
		if (cur->src.addr.addr_dyn != NULL ||
		    prev->src.addr.addr_dyn != NULL ||
		    cur->src.not !=  prev->src.not ||
		    !PF_AEQ(&cur->src.addr.addr, &prev->src.addr.addr, 0) ||
		    !PF_AEQ(&cur->src.addr.mask, &prev->src.addr.mask, 0))
			PF_SET_SKIP_STEPS(PF_SKIP_SRC_ADDR);
		if (cur->src.port[0] != prev->src.port[0] ||
		    cur->src.port[1] != prev->src.port[1] ||
		    cur->src.port_op != prev->src.port_op)
			PF_SET_SKIP_STEPS(PF_SKIP_SRC_PORT);
		if (cur->dst.addr.addr_dyn != NULL ||
		    prev->dst.addr.addr_dyn != NULL ||
		    cur->dst.not !=  prev->dst.not ||
		    !PF_AEQ(&cur->dst.addr.addr, &prev->dst.addr.addr, 0) ||
		    !PF_AEQ(&cur->dst.addr.mask, &prev->dst.addr.mask, 0))
			PF_SET_SKIP_STEPS(PF_SKIP_DST_ADDR);
		if (cur->dst.port[0] != prev->dst.port[0] ||
		    cur->dst.port[1] != prev->dst.port[1] ||
		    cur->dst.port_op != prev->dst.port_op)
			PF_SET_SKIP_STEPS(PF_SKIP_DST_PORT);

		prev = cur;
		cur = TAILQ_NEXT(cur, entries);
	}
	for (i = 0; i < PF_SKIP_COUNT; ++i)
		PF_SET_SKIP_STEPS(i);
}

void
pf_update_anchor_rules()
{
	struct pf_rule	*rule;
	int		 i;

	for (i = 0; i < PF_RULESET_MAX; ++i)
		TAILQ_FOREACH(rule, pf_main_ruleset.rules[i].active.ptr,
		    entries)
			if (rule->anchorname[0])
				rule->anchor = pf_find_anchor(rule->anchorname);
			else
				rule->anchor = NULL;
}

u_int16_t
pf_cksum_fixup(u_int16_t cksum, u_int16_t old, u_int16_t new, u_int8_t udp)
{
	u_int32_t	l;

	if (udp && !cksum)
		return (0x0000);
	l = cksum + old - new;
	l = (l >> 16) + (l & 65535);
	l = l & 65535;
	if (udp && !l)
		return (0xFFFF);
	return (l);
}

void
pf_change_ap(struct pf_addr *a, u_int16_t *p, u_int16_t *ic, u_int16_t *pc,
    struct pf_addr *an, u_int16_t pn, u_int8_t u, sa_family_t af)
{
	struct pf_addr	ao;
	u_int16_t	po = *p;

	PF_ACPY(&ao, a, af);
	PF_ACPY(a, an, af);

	*p = pn;

	switch (af) {
#ifdef INET
	case AF_INET:
		*ic = pf_cksum_fixup(pf_cksum_fixup(*ic,
		    ao.addr16[0], an->addr16[0], 0),
		    ao.addr16[1], an->addr16[1], 0);
		*p = pn;
		*pc = pf_cksum_fixup(pf_cksum_fixup(pf_cksum_fixup(*pc,
		    ao.addr16[0], an->addr16[0], u),
		    ao.addr16[1], an->addr16[1], u),
		    po, pn, u);
		break;
#endif /* INET */
#ifdef INET6
	case AF_INET6:
		*pc = pf_cksum_fixup(pf_cksum_fixup(pf_cksum_fixup(
		    pf_cksum_fixup(pf_cksum_fixup(pf_cksum_fixup(
		    pf_cksum_fixup(pf_cksum_fixup(pf_cksum_fixup(*pc,
		    ao.addr16[0], an->addr16[0], u),
		    ao.addr16[1], an->addr16[1], u),
		    ao.addr16[2], an->addr16[2], u),
		    ao.addr16[3], an->addr16[3], u),
		    ao.addr16[4], an->addr16[4], u),
		    ao.addr16[5], an->addr16[5], u),
		    ao.addr16[6], an->addr16[6], u),
		    ao.addr16[7], an->addr16[7], u),
		    po, pn, u);
		break;
#endif /* INET6 */
	}
}

void
pf_change_a(u_int32_t *a, u_int16_t *c, u_int32_t an, u_int8_t u)
{
	u_int32_t	ao = *a;

	*a = an;
	*c = pf_cksum_fixup(pf_cksum_fixup(*c, ao / 65536, an / 65536, u),
	   ao % 65536, an % 65536, u);
}

#ifdef INET6
void
pf_change_a6(struct pf_addr *a, u_int16_t *c, struct pf_addr *an, u_int8_t u)
{
	struct pf_addr	ao;

	PF_ACPY(&ao, a, AF_INET6);
	PF_ACPY(a, an, AF_INET6);

	*c = pf_cksum_fixup(pf_cksum_fixup(pf_cksum_fixup(
	    pf_cksum_fixup(pf_cksum_fixup(pf_cksum_fixup(
	    pf_cksum_fixup(pf_cksum_fixup(*c,
	    ao.addr16[0], an->addr16[0], u),
	    ao.addr16[1], an->addr16[1], u),
	    ao.addr16[2], an->addr16[2], u),
	    ao.addr16[3], an->addr16[3], u),
	    ao.addr16[4], an->addr16[4], u),
	    ao.addr16[5], an->addr16[5], u),
	    ao.addr16[6], an->addr16[6], u),
	    ao.addr16[7], an->addr16[7], u);
}
#endif /* INET6 */

void
pf_change_icmp(struct pf_addr *ia, u_int16_t *ip, struct pf_addr *oa,
    struct pf_addr *na, u_int16_t np, u_int16_t *pc, u_int16_t *h2c,
    u_int16_t *ic, u_int16_t *hc, u_int8_t u, sa_family_t af)
{
	struct pf_addr	oia, ooa;
	u_int32_t	opc, oh2c = *h2c;
	u_int16_t	oip = *ip;

	PF_ACPY(&oia, ia, af);
	PF_ACPY(&ooa, oa, af);

	if (pc != NULL)
		opc = *pc;
	/* Change inner protocol port, fix inner protocol checksum. */
	*ip = np;
	if (pc != NULL)
		*pc = pf_cksum_fixup(*pc, oip, *ip, u);
	*ic = pf_cksum_fixup(*ic, oip, *ip, 0);
	if (pc != NULL)
		*ic = pf_cksum_fixup(*ic, opc, *pc, 0);
	PF_ACPY(ia, na, af);
	/* Change inner ip address, fix inner ipv4 and icmp checksums. */
	switch (af) {
#ifdef INET
	case AF_INET:
		*h2c = pf_cksum_fixup(pf_cksum_fixup(*h2c,
		    oia.addr16[0], ia->addr16[0], 0),
		    oia.addr16[1], ia->addr16[1], 0);
		*ic = pf_cksum_fixup(pf_cksum_fixup(*ic,
		    oia.addr16[0], ia->addr16[0], 0),
		    oia.addr16[1], ia->addr16[1], 0);
		break;
#endif /* INET */
#ifdef INET6
	case AF_INET6:
		*ic = pf_cksum_fixup(pf_cksum_fixup(pf_cksum_fixup(
		    pf_cksum_fixup(pf_cksum_fixup(pf_cksum_fixup(
		    pf_cksum_fixup(pf_cksum_fixup(*ic,
		    oia.addr16[0], ia->addr16[0], u),
		    oia.addr16[1], ia->addr16[1], u),
		    oia.addr16[2], ia->addr16[2], u),
		    oia.addr16[3], ia->addr16[3], u),
		    oia.addr16[4], ia->addr16[4], u),
		    oia.addr16[5], ia->addr16[5], u),
		    oia.addr16[6], ia->addr16[6], u),
		    oia.addr16[7], ia->addr16[7], u);
		break;
#endif /* INET6 */
	}
	*ic = pf_cksum_fixup(*ic, oh2c, *h2c, 0);
	/* Change outer ip address, fix outer ipv4 or icmpv6 checksum. */
	PF_ACPY(oa, na, af);
	switch (af) {
#ifdef INET
	case AF_INET:
		*hc = pf_cksum_fixup(pf_cksum_fixup(*hc,
		    ooa.addr16[0], oa->addr16[0], 0),
		    ooa.addr16[1], oa->addr16[1], 0);
		break;
#endif /* INET */
#ifdef INET6
	case AF_INET6:
		*ic = pf_cksum_fixup(pf_cksum_fixup(pf_cksum_fixup(
		    pf_cksum_fixup(pf_cksum_fixup(pf_cksum_fixup(
		    pf_cksum_fixup(pf_cksum_fixup(*ic,
		    ooa.addr16[0], oa->addr16[0], u),
		    ooa.addr16[1], oa->addr16[1], u),
		    ooa.addr16[2], oa->addr16[2], u),
		    ooa.addr16[3], oa->addr16[3], u),
		    ooa.addr16[4], oa->addr16[4], u),
		    ooa.addr16[5], oa->addr16[5], u),
		    ooa.addr16[6], oa->addr16[6], u),
		    ooa.addr16[7], oa->addr16[7], u);
		break;
#endif /* INET6 */
	}
}

void
pf_send_reset(int off, struct tcphdr *th, struct pf_pdesc *pd, sa_family_t af,
    u_int8_t return_ttl, struct pf_rule *r)
{
	struct mbuf	*m;
	struct m_tag	*mtag;
	int		 len;
#ifdef INET
	struct ip	*h2;
#endif /* INET */
#ifdef INET6
	struct ip6_hdr	*h2_6;
#endif /* INET6 */
	struct tcphdr	*th2;

	switch (af) {
#ifdef INET
	case AF_INET:
		len = sizeof(struct ip) + sizeof(struct tcphdr);
		break;
#endif /* INET */
#ifdef INET6
	case AF_INET6:
		len = sizeof(struct ip6_hdr) + sizeof(struct tcphdr);
		break;
#endif /* INET6 */
	}

	/* don't reply to RST packets */
	if (th->th_flags & TH_RST)
		return;

	/* create outgoing mbuf */
	mtag = m_tag_get(PACKET_TAG_PF_GENERATED, 0, M_NOWAIT);
	if (mtag == NULL)
		return;
	m = m_gethdr(M_DONTWAIT, MT_HEADER);
	if (m == NULL) {
		m_tag_free(mtag);
		return;
	}
	m_tag_prepend(m, mtag);
	m->m_data += max_linkhdr;
	m->m_pkthdr.len = m->m_len = len;
	m->m_pkthdr.rcvif = NULL;
	bzero(m->m_data, len);
	switch (af) {
#ifdef INET
	case AF_INET:
		h2 = mtod(m, struct ip *);

		/* IP header fields included in the TCP checksum */
		h2->ip_p = IPPROTO_TCP;
		h2->ip_len = htons(sizeof(*th2));
		h2->ip_src.s_addr = pd->dst->v4.s_addr;
		h2->ip_dst.s_addr = pd->src->v4.s_addr;

		th2 = (struct tcphdr *)((caddr_t)h2 + sizeof(struct ip));
		break;
#endif /* INET */
#ifdef INET6
	case AF_INET6:
		h2_6 = mtod(m, struct ip6_hdr *);

		/* IP header fields included in the TCP checksum */
		h2_6->ip6_nxt = IPPROTO_TCP;
		h2_6->ip6_plen = htons(sizeof(*th2));
		memcpy(&h2_6->ip6_src, pd->dst, sizeof(struct in6_addr));
		memcpy(&h2_6->ip6_dst, pd->src, sizeof(struct in6_addr));

		th2 = (struct tcphdr *)((caddr_t)h2_6 + sizeof(struct ip6_hdr));
		break;
#endif /* INET6 */
	}

	/* TCP header */
	th2->th_sport = th->th_dport;
	th2->th_dport = th->th_sport;
	if (th->th_flags & TH_ACK) {
		th2->th_seq = th->th_ack;
		th2->th_flags = TH_RST;
	} else {
		int tlen = pd->p_len;
		if (th->th_flags & TH_SYN)
			tlen++;
		if (th->th_flags & TH_FIN)
			tlen++;
		th2->th_ack = htonl(ntohl(th->th_seq) + tlen);
		th2->th_flags = TH_RST | TH_ACK;
	}
	th2->th_off = sizeof(*th2) >> 2;

#ifdef ALTQ
	if (r != NULL && r->qid) {
		struct altq_tag *atag;

		mtag = m_tag_get(PACKET_TAG_PF_QID, sizeof(*atag), M_NOWAIT);
		if (mtag != NULL) {
			atag = (struct altq_tag *)(mtag + 1);
			atag->qid = r->qid;
			/* add hints for ecn */
			atag->af = af;
			atag->hdr = mtod(m, struct ip *);
			m_tag_prepend(m, mtag);
		}
	}
#endif

	switch (af) {
#ifdef INET
	case AF_INET:
		/* TCP checksum */
		th2->th_sum = in_cksum(m, len);

		/* Finish the IP header */
		h2->ip_v = 4;
		h2->ip_hl = sizeof(*h2) >> 2;
		if (!return_ttl)
			return_ttl = ip_defttl;
		h2->ip_ttl = return_ttl;
		h2->ip_sum = 0;
		h2->ip_len = len;
		h2->ip_off = ip_mtudisc ? IP_DF : 0;
		ip_output(m, (void *)NULL, (void *)NULL, 0, (void *)NULL,
		  (void *)NULL);
		break;
#endif /* INET */
#ifdef INET6
	case AF_INET6:
		/* TCP checksum */
		th2->th_sum = in6_cksum(m, IPPROTO_TCP,
		    sizeof(struct ip6_hdr), sizeof(*th));

		h2_6->ip6_vfc |= IPV6_VERSION;
		if (!return_ttl)
			return_ttl = IPV6_DEFHLIM;
		h2_6->ip6_hlim = return_ttl;

		ip6_output(m, NULL, NULL, 0, NULL, NULL);
#endif /* INET6 */
	}
}

void
pf_send_icmp(struct mbuf *m, u_int8_t type, u_int8_t code, sa_family_t af,
    struct pf_rule *r)
{
	struct m_tag	*mtag;
	struct mbuf	*m0;

	mtag = m_tag_get(PACKET_TAG_PF_GENERATED, 0, M_NOWAIT);
	if (mtag == NULL)
		return;
	m0 = m_copy(m, 0, M_COPYALL);
	if (m0 == NULL) {
		m_tag_free(mtag);
		return;
	}
	m_tag_prepend(m0, mtag);

#ifdef ALTQ
	if (r != NULL && r->qid) {
		struct altq_tag *atag;

		mtag = m_tag_get(PACKET_TAG_PF_QID, sizeof(*atag), M_NOWAIT);
		if (mtag != NULL) {
			atag = (struct altq_tag *)(mtag + 1);
			atag->qid = r->qid;
			/* add hints for ecn */
			atag->af = af;
			atag->hdr = mtod(m0, struct ip *);
			m_tag_prepend(m0, mtag);
		}
	}
#endif

	switch (af) {
#ifdef INET
	case AF_INET:
		icmp_error(m0, type, code, 0, 0);
		break;
#endif /* INET */
#ifdef INET6
	case AF_INET6:
		icmp6_error(m0, type, code, 0);
		break;
#endif /* INET6 */
	}
}

/*
 * Return 1 if the addresses a and b match (with mask m), otherwise return 0.
 * If n is 0, they match if they are equal. If n is != 0, they match if they
 * are different.
 */
int
pf_match_addr(u_int8_t n, struct pf_addr *a, struct pf_addr *m,
    struct pf_addr *b, sa_family_t af)
{
	int	match = 0;

	if (m->addr32[0] == PF_TABLE_MASK)
		return (pfr_match_addr(a, m, b, af) != n);

	switch (af) {
#ifdef INET
	case AF_INET:
		if ((a->addr32[0] & m->addr32[0]) ==
		    (b->addr32[0] & m->addr32[0]))
			match++;
		break;
#endif /* INET */
#ifdef INET6
	case AF_INET6:
		if (((a->addr32[0] & m->addr32[0]) ==
		     (b->addr32[0] & m->addr32[0])) &&
		    ((a->addr32[1] & m->addr32[1]) ==
		     (b->addr32[1] & m->addr32[1])) &&
		    ((a->addr32[2] & m->addr32[2]) ==
		     (b->addr32[2] & m->addr32[2])) &&
		    ((a->addr32[3] & m->addr32[3]) ==
		     (b->addr32[3] & m->addr32[3])))
			match++;
		break;
#endif /* INET6 */
	}
	if (match) {
		if (n)
			return (0);
		else
			return (1);
	} else {
		if (n)
			return (1);
		else
			return (0);
	}
}

int
pf_match(u_int8_t op, u_int16_t a1, u_int16_t a2, u_int16_t p)
{
	switch (op) {
	case PF_OP_IRG:
		return ((p > a1) && (p < a2));
	case PF_OP_XRG:
		return ((p < a1) || (p > a2));
	case PF_OP_RRG:
		return ((p >= a1) && (p <= a2));
	case PF_OP_EQ:
		return (p == a1);
	case PF_OP_NE:
		return (p != a1);
	case PF_OP_LT:
		return (p < a1);
	case PF_OP_LE:
		return (p <= a1);
	case PF_OP_GT:
		return (p > a1);
	case PF_OP_GE:
		return (p >= a1);
	}
	return (0); /* never reached */
}

int
pf_match_port(u_int8_t op, u_int16_t a1, u_int16_t a2, u_int16_t p)
{
	NTOHS(a1);
	NTOHS(a2);
	NTOHS(p);
	return (pf_match(op, a1, a2, p));
}

int
pf_match_uid(u_int8_t op, uid_t a1, uid_t a2, uid_t u)
{
	if (u == UID_MAX && op != PF_OP_EQ && op != PF_OP_NE)
		return (0);
	return (pf_match(op, a1, a2, u));
}

int
pf_match_gid(u_int8_t op, gid_t a1, gid_t a2, gid_t g)
{
	if (g == GID_MAX && op != PF_OP_EQ && op != PF_OP_NE)
		return (0);
	return (pf_match(op, a1, a2, g));
}

#define PF_STEP_INTO_ANCHOR(r, a, s, n)					\
	do {								\
		if ((r) == NULL || (r)->anchor == NULL ||		\
		    (s) != NULL || (a) != NULL)				\
			panic("PF_STEP_INTO_ANCHOR");			\
		(a) = (r);						\
		(s) = TAILQ_FIRST(&(r)->anchor->rulesets);		\
		(r) = NULL;						\
		while ((s) != NULL && ((r) =				\
		    TAILQ_FIRST((s)->rules[n].active.ptr)) == NULL)	\
			(s) = TAILQ_NEXT((s), entries);			\
		if ((r) == NULL) {					\
			(r) = TAILQ_NEXT((a), entries);			\
			(a) = NULL;					\
		}							\
	} while (0)

#define PF_STEP_OUT_OF_ANCHOR(r, a, s, n)				\
	do {								\
		if ((r) != NULL || (a) == NULL || (s) == NULL)		\
			panic("PF_STEP_OUT_OF_ANCHOR");			\
		(s) = TAILQ_NEXT((s), entries);				\
		while ((s) != NULL && ((r) =				\
		    TAILQ_FIRST((s)->rules[n].active.ptr)) == NULL)	\
			(s) = TAILQ_NEXT((s), entries);			\
		if ((r) == NULL) {					\
			(r) = TAILQ_NEXT((a), entries);			\
			(a) = NULL;					\
		}							\
	} while (0)

#ifdef INET6
void
pf_poolmask(struct pf_addr *naddr, struct pf_addr *raddr,
    struct pf_addr *rmask, struct pf_addr *saddr, sa_family_t af)
{
	switch (af) {
#ifdef INET
	case AF_INET:
		naddr->addr32[0] = (raddr->addr32[0] & rmask->addr32[0]) |
		((rmask->addr32[0] ^ 0xffffffff ) & saddr->addr32[0]);
		break;
#endif /* INET */
	case AF_INET6:
		naddr->addr32[0] = (raddr->addr32[0] & rmask->addr32[0]) |
		((rmask->addr32[0] ^ 0xffffffff ) & saddr->addr32[0]);
		naddr->addr32[1] = (raddr->addr32[1] & rmask->addr32[1]) |
		((rmask->addr32[1] ^ 0xffffffff ) & saddr->addr32[1]);
		naddr->addr32[2] = (raddr->addr32[2] & rmask->addr32[2]) |
		((rmask->addr32[2] ^ 0xffffffff ) & saddr->addr32[2]);
		naddr->addr32[3] = (raddr->addr32[3] & rmask->addr32[3]) |
		((rmask->addr32[3] ^ 0xffffffff ) & saddr->addr32[3]);
		break;
	}
}

void
pf_addr_inc(struct pf_addr *addr, u_int8_t af)
{
	switch (af) {
#ifdef INET
	case AF_INET:
		addr->addr32[0] = htonl(ntohl(addr->addr32[0]) + 1);
		break;
#endif /* INET */
	case AF_INET6:
		if (addr->addr32[3] == 0xffffffff) {
			addr->addr32[3] = 0;
			if (addr->addr32[2] == 0xffffffff) {
				addr->addr32[2] = 0;
				if (addr->addr32[1] == 0xffffffff) {
					addr->addr32[1] = 0;
					addr->addr32[0] =
					    htonl(ntohl(addr->addr32[0]) + 1);
				} else
					addr->addr32[1] =
					    htonl(ntohl(addr->addr32[1]) + 1);
			} else
				addr->addr32[2] =
				    htonl(ntohl(addr->addr32[2]) + 1);
		} else
			addr->addr32[3] =
			    htonl(ntohl(addr->addr32[3]) + 1);
		break;
	}
}
#endif /* INET6 */

#define mix(a,b,c) \
	do {				    \
		a -= b; a -= c; a ^= (c >> 13); \
		b -= c; b -= a; b ^= (a << 8);  \
		c -= a; c -= b; c ^= (b >> 13); \
		a -= b; a -= c; a ^= (c >> 12); \
		b -= c; b -= a; b ^= (a << 16); \
		c -= a; c -= b; c ^= (b >> 5);  \
		a -= b; a -= c; a ^= (c >> 3);  \
		b -= c; b -= a; b ^= (a << 10); \
		c -= a; c -= b; c ^= (b >> 15); \
	} while(0)

/*
 * hash function based on bridge_hash in if_bridge.c
 */
void
pf_hash(struct pf_addr *inaddr, struct pf_addr *hash,
    struct pf_poolhashkey *key, sa_family_t af)
{
	u_int32_t	a = 0x9e3779b9, b = 0x9e3779b9, c = key->key32[0];

	switch (af) {
#ifdef INET
	case AF_INET:
		a += inaddr->addr32[0];
		b += key->key32[1];
		mix(a, b, c);
		hash->addr32[0] = c + key->key32[2];
		break;
#endif /* INET */
#ifdef INET6
	case AF_INET6:
		a += inaddr->addr32[0];
		b += inaddr->addr32[2];
		mix(a, b, c);
		hash->addr32[0] = c;
		a += inaddr->addr32[1];
		b += inaddr->addr32[3];
		c += key->key32[1];
		mix(a, b, c);
		hash->addr32[1] = c;
		a += inaddr->addr32[2];
		b += inaddr->addr32[1];
		c += key->key32[2];
		mix(a, b, c);
		hash->addr32[2] = c;
		a += inaddr->addr32[3];
		b += inaddr->addr32[0];
		c += key->key32[3];
		mix(a, b, c);
		hash->addr32[3] = c;
		break;
#endif /* INET6 */
	}
}

int
pf_map_addr(u_int8_t af, struct pf_pool *rpool, struct pf_addr *saddr,
    struct pf_addr *naddr, struct pf_addr *init_addr)
{
	unsigned char		 hash[16];
	struct pf_pooladdr	*cur = rpool->cur;
	struct pf_addr		*raddr = &rpool->cur->addr.addr.addr;
	struct pf_addr		*rmask = &rpool->cur->addr.addr.mask;

	if (cur->addr.addr.addr_dyn != NULL &&
	    cur->addr.addr.addr_dyn->undefined)
		return (1);

	switch (rpool->opts & PF_POOL_TYPEMASK) {
	case PF_POOL_NONE:
		PF_ACPY(naddr, raddr, af);
		break;
	case PF_POOL_BITMASK:
		PF_POOLMASK(naddr, raddr, rmask, saddr, af);
		break;
	case PF_POOL_RANDOM:
		if (init_addr != NULL && PF_AZERO(init_addr, af)) {
			switch (af) {
#ifdef INET
			case AF_INET:
				rpool->counter.addr32[0] = arc4random();
				break;
#endif /* INET */
#ifdef INET6
			case AF_INET6:
				if (rmask->addr32[3] != 0xffffffff)
					rpool->counter.addr32[3] = arc4random();
				else
					break;
				if (rmask->addr32[2] != 0xffffffff)
					rpool->counter.addr32[2] = arc4random();
				else
					break;
				if (rmask->addr32[1] != 0xffffffff)
					rpool->counter.addr32[1] = arc4random();
				else
					break;
				if (rmask->addr32[0] != 0xffffffff)
					rpool->counter.addr32[0] = arc4random();
				break;
#endif /* INET6 */
			}
			PF_POOLMASK(naddr, raddr, rmask, &rpool->counter, af);
			PF_ACPY(init_addr, naddr, af);

		} else {
			PF_AINC(&rpool->counter, af);
			PF_POOLMASK(naddr, raddr, rmask, &rpool->counter, af);
		}
		break;
	case PF_POOL_SRCHASH:
		pf_hash(saddr, (struct pf_addr *)&hash, &rpool->key, af);
		PF_POOLMASK(naddr, raddr, rmask, (struct pf_addr *)&hash, af);
		break;
	case PF_POOL_ROUNDROBIN:
		if (pf_match_addr(0, &cur->addr.addr.addr, &cur->addr.addr.mask,
		    &rpool->counter, af)) {
			PF_ACPY(naddr, &rpool->counter, af);
			PF_AINC(&rpool->counter, af);
		} else {
			if ((rpool->cur =
			    TAILQ_NEXT(rpool->cur, entries)) == NULL)
				rpool->cur = TAILQ_FIRST(&rpool->list);
			PF_ACPY(naddr, &cur->addr.addr.addr, af);
			PF_ACPY(&rpool->counter, &cur->addr.addr.addr, af);
			PF_AINC(&rpool->counter, af);
		}
		break;
	}

	if (pf_status.debug >= PF_DEBUG_MISC) {
		printf("pf_map_addr: selected address: ");
		pf_print_host(naddr, 0, af);
		printf("\n");
	}

	return (0);
}

int
pf_get_sport(sa_family_t af, u_int8_t proto, struct pf_pool *rpool,
    struct pf_addr *saddr, u_int16_t sport, struct pf_addr *daddr,
    u_int16_t dport, struct pf_addr *naddr, u_int16_t *nport, u_int16_t low,
    u_int16_t high)
{
	struct pf_tree_node	key;
	struct pf_addr		init_addr;
	int			step;
	u_int16_t		cut;

	bzero(&init_addr, sizeof(init_addr));
	if (pf_map_addr(af, rpool, saddr, naddr, &init_addr))
		return (1);

	do {
		key.af = af;
		key.proto = proto;
		PF_ACPY(&key.addr[0], daddr, key.af);
		PF_ACPY(&key.addr[1], naddr, key.af);
		key.port[0] = dport;

		/*
		 * port search; start random, step;
		 * similar 2 portloop in in_pcbbind
		 */
		if (!(proto == IPPROTO_TCP || proto == IPPROTO_UDP)) {
			key.port[1] = 0;
			if (pf_find_state(&tree_ext_gwy, &key) == NULL)
				return (0);
		} else if (rpool->opts & PF_POOL_STATICPORT) {
			key.port[1] = sport;
			if (pf_find_state(&tree_ext_gwy, &key) == NULL) {
				*nport = ntohs(sport);
				return (0);
			}
		} else if (low == 0 && high == 0) {
			key.port[1] = *nport;
			if (pf_find_state(&tree_ext_gwy, &key) == NULL) {
				NTOHS(*nport);
				return (0);
			}
		} else if (low == high) {
			key.port[1] = htons(low);
			if (pf_find_state(&tree_ext_gwy, &key) == NULL) {
				*nport = low;
				return (0);
			}
		} else {
			if (low < high) {
				step = 1;
				cut = arc4random() % (1 + high - low) + low;
			} else {
				step = -1;
				cut = arc4random() % (1 + low - high) + high;
			}

			*nport = cut - step;
			do {
				*nport += step;
				key.port[1] = htons(*nport);
				if (pf_find_state(&tree_ext_gwy, &key) == NULL)
					return (0);
			} while (*nport != low && *nport != high);

			step = -step;
			*nport = cut;
			do {
				*nport += step;
				key.port[1] = htons(*nport);
				if (pf_find_state(&tree_ext_gwy, &key) == NULL)
					return (0);
			} while (*nport != low && *nport != high);
		}

		switch (rpool->opts & PF_POOL_TYPEMASK) {
		case PF_POOL_RANDOM:
		case PF_POOL_ROUNDROBIN:
			if (pf_map_addr(af, rpool, saddr, naddr, &init_addr))
				return (1);
			break;
		case PF_POOL_NONE:
		case PF_POOL_SRCHASH:
		case PF_POOL_BITMASK:
		default:
			return (1);
			break;
		}
	} while (! PF_AEQ(&init_addr, naddr, af) );

	return (1);					/* none available */
}

struct pf_rule *
pf_match_translation(int direction, struct ifnet *ifp, u_int8_t proto,
    struct pf_addr *saddr, u_int16_t sport, struct pf_addr *daddr,
    u_int16_t dport, sa_family_t af, int rs_num)
{
	struct pf_rule		*r, *rm = NULL, *anchorrule = NULL;
	struct pf_ruleset	*ruleset = NULL;

	r = TAILQ_FIRST(pf_main_ruleset.rules[rs_num].active.ptr);
	while (r && rm == NULL) {
		struct pf_rule_addr	*src = NULL;

		if (r->action == PF_BINAT && direction == PF_IN) {
			if (r->rpool.cur != NULL)
				src = &r->rpool.cur->addr;
		} else
			src = &r->src;

		r->evaluations++;
		if (r->ifp != NULL && ((r->ifp != ifp && !r->ifnot) ||
		    (r->ifp == ifp && r->ifnot)))
			r = r->skip[PF_SKIP_IFP].ptr;
		else if (r->direction && r->direction != direction)
			r = r->skip[PF_SKIP_DIR].ptr;
		else if (r->af && r->af != af)
			r = r->skip[PF_SKIP_AF].ptr;
		else if (r->proto && r->proto != proto)
			r = r->skip[PF_SKIP_PROTO].ptr;
		else if (src != NULL && !PF_AZERO(&src->addr.mask, af) &&
		    !PF_MATCHA(src->not,
		    &src->addr.addr, &src->addr.mask, saddr, af))
			r = r->skip[PF_SKIP_SRC_ADDR].ptr;
		else if (src != NULL && src->port_op &&
		    !pf_match_port(src->port_op, src->port[0],
		    src->port[1], sport))
			r = r->skip[PF_SKIP_SRC_PORT].ptr;
		else if (!PF_AZERO(&r->dst.addr.mask, af) &&
		    !PF_MATCHA(r->dst.not,
		    &r->dst.addr.addr, &r->dst.addr.mask, daddr, af))
			r = r->skip[PF_SKIP_DST_ADDR].ptr;
		else if (r->dst.port_op && !pf_match_port(r->dst.port_op,
		    r->dst.port[0], r->dst.port[1], dport))
			r = r->skip[PF_SKIP_DST_PORT].ptr;
		else if (r->anchorname[0] && r->anchor == NULL)
			r = TAILQ_NEXT(r, entries);
		else if (r->anchor == NULL)
				rm = r;
		else
			PF_STEP_INTO_ANCHOR(r, anchorrule, ruleset, rs_num);
		if (r == NULL && anchorrule != NULL)
			PF_STEP_OUT_OF_ANCHOR(r, anchorrule, ruleset,
			    rs_num);
	}
	if (rm != NULL && (rm->action == PF_NONAT ||
	    rm->action == PF_NORDR || rm->action == PF_NOBINAT))
		return (NULL);
	return (rm);
}

struct pf_rule *
pf_get_translation(int direction, struct ifnet *ifp, u_int8_t proto,
    struct pf_addr *saddr, u_int16_t sport,
    struct pf_addr *daddr, u_int16_t dport,
    struct pf_addr *naddr, u_int16_t *nport, sa_family_t af)
{
	struct pf_rule	*r = NULL;

	if (direction == PF_OUT) {
		r = pf_match_translation(direction, ifp, proto,
		    saddr, sport, daddr, dport, af, PF_RULESET_BINAT);
		if (r == NULL)
			r = pf_match_translation(direction, ifp, proto,
			    saddr, sport, daddr, dport, af, PF_RULESET_NAT);
	} else {
		r = pf_match_translation(direction, ifp, proto,
		    saddr, sport, daddr, dport, af, PF_RULESET_RDR);
		if (r == NULL)
			r = pf_match_translation(direction, ifp, proto,
			    saddr, sport, daddr, dport, af, PF_RULESET_BINAT);
	}

	if (r != NULL) {
		switch (r->action) {
		case PF_NONAT:
		case PF_NOBINAT:
		case PF_NORDR:
			return (NULL);
			break;
		case PF_NAT:
			if (pf_get_sport(af, proto,
			    &r->rpool, saddr, sport, daddr,
			    dport, naddr, nport, r->rpool.proxy_port[0],
			    r->rpool.proxy_port[1])) {
				DPFPRINTF(PF_DEBUG_MISC,
				    ("pf: NAT proxy port allocation "
				    "(%u-%u) failed\n",
				    r->rpool.proxy_port[0],
				    r->rpool.proxy_port[1]));
				return (NULL);
			}
			break;
		case PF_BINAT:
			switch (direction) {
			case PF_OUT:
				if (r->rpool.cur->addr.addr.addr_dyn != NULL &&
				    r->rpool.cur->addr.addr.addr_dyn->undefined)
					return (NULL);
				else
					PF_POOLMASK(naddr,
					    &r->rpool.cur->addr.addr.addr,
					    &r->rpool.cur->addr.addr.mask,
					    saddr, af);
				break;
			case PF_IN:
				if (r->src.addr.addr_dyn != NULL &&
				    r->src.addr.addr_dyn->undefined)
					return (NULL);
				else
					PF_POOLMASK(naddr, &r->src.addr.addr,
					    &r->src.addr.mask, saddr, af);
				break;
			}
			break;
		case PF_RDR: {
			if (pf_map_addr(r->af, &r->rpool,
			    &r->src.addr.addr, naddr, NULL))
				return (NULL);

			if (r->dst.port_op == PF_OP_RRG) {
				u_int32_t	tmp_nport;
				tmp_nport = ntohs(r->rpool.proxy_port[0])
				    + (ntohs(dport) - ntohs(r->dst.port[0]));
				/* wrap around if necessary */
				if (tmp_nport > 65535)
					tmp_nport -= 65535;
				*nport = htons((u_int16_t)tmp_nport);
			} else if (r->rpool.proxy_port[0])
				*nport = r->rpool.proxy_port[0];
			break;
		}
		default:
			return (NULL);
			break;
		}
	}

	return (r);
}

int
pf_socket_lookup(uid_t *uid, gid_t *gid, int direction, sa_family_t af,
    int proto, struct pf_pdesc *pd)
{
	struct pf_addr		*saddr, *daddr;
	u_int16_t		 sport, dport;
	struct inpcbtable	*tb;
	struct inpcb		*inp;

	*uid = UID_MAX;
	*gid = GID_MAX;
	if (af != AF_INET)
		return (0);
	switch (proto) {
	case IPPROTO_TCP:
		sport = pd->hdr.tcp->th_sport;
		dport = pd->hdr.tcp->th_dport;
		tb = &tcbtable;
		break;
	case IPPROTO_UDP:
		sport = pd->hdr.udp->uh_sport;
		dport = pd->hdr.udp->uh_dport;
		tb = &udbtable;
		break;
	default:
		return (0);
	}
	if (direction == PF_IN) {
		saddr = pd->src;
		daddr = pd->dst;
	} else {
		u_int16_t	p;

		p = sport;
		sport = dport;
		dport = p;
		saddr = pd->dst;
		daddr = pd->src;
	}
	inp = in_pcbhashlookup(tb, saddr->v4, sport, daddr->v4, dport);
	if (inp == NULL) {
		inp = in_pcblookup(tb, &saddr->v4, sport, &daddr->v4, dport,
		    INPLOOKUP_WILDCARD);
		if (inp == NULL)
			return (0);
	}
	*uid = inp->inp_socket->so_euid;
	*gid = inp->inp_socket->so_egid;
	return (1);
}

int
pf_test_tcp(struct pf_rule **rm, int direction, struct ifnet *ifp,
    struct mbuf *m, int ipoff, int off, void *h, struct pf_pdesc *pd)
{
	struct pf_rule		*nat = NULL, *rdr = NULL;
	struct pf_addr		*saddr = pd->src, *daddr = pd->dst;
	struct pf_addr		 baddr, naddr;
	struct tcphdr		*th = pd->hdr.tcp;
	u_int16_t		 bport, nport = 0;
	sa_family_t		 af = pd->af;
	int			 lookup = -1;
	uid_t			 uid;
	gid_t			 gid;
	struct pf_rule		*r, *rs = NULL, *anchorrule = NULL;
	struct pf_ruleset	*ruleset = NULL;
	u_short			 reason;
	int			 rewrite = 0;

	*rm = NULL;

	if (direction == PF_OUT) {
		bport = nport = th->th_sport;
		/* check outgoing packet for BINAT/NAT */
		if ((nat = pf_get_translation(PF_OUT, ifp, IPPROTO_TCP,
	            saddr, th->th_sport, daddr, th->th_dport,
		    &naddr, &nport, af)) != NULL) {
			PF_ACPY(&baddr, saddr, af);
			pf_change_ap(saddr, &th->th_sport, pd->ip_sum,
			    &th->th_sum, &naddr, th->th_sport, 0, af);
			rewrite++;
		}
	} else {
		bport = nport = th->th_dport;
		/* check incoming packet for BINAT/RDR */
		if ((rdr = pf_get_translation(PF_IN, ifp, IPPROTO_TCP,
	            saddr, th->th_sport, daddr, th->th_dport,
		    &naddr, &nport, af)) != NULL) {
			PF_ACPY(&baddr, daddr, af);
			pf_change_ap(daddr, &th->th_dport, pd->ip_sum,
			    &th->th_sum, &naddr, nport, 0, af);
			rewrite++;
		}
	}

	r = TAILQ_FIRST(pf_main_ruleset.rules[PF_RULESET_FILTER].active.ptr);
	while (r != NULL) {
		r->evaluations++;
		if (r->ifp != NULL && ((r->ifp != ifp && !r->ifnot) ||
		    (r->ifp == ifp && r->ifnot)))
			r = r->skip[PF_SKIP_IFP].ptr;
		else if (r->direction && r->direction != direction)
			r = r->skip[PF_SKIP_DIR].ptr;
		else if (r->af && r->af != af)
			r = r->skip[PF_SKIP_AF].ptr;
		else if (r->proto && r->proto != IPPROTO_TCP)
			r = r->skip[PF_SKIP_PROTO].ptr;
		else if (r->src.noroute && pf_routable(saddr, af))
			r = TAILQ_NEXT(r, entries);
		else if (!r->src.noroute &&
		    !PF_AZERO(&r->src.addr.mask, af) && !PF_MATCHA(r->src.not,
		    &r->src.addr.addr, &r->src.addr.mask, saddr, af))
			r = r->skip[PF_SKIP_SRC_ADDR].ptr;
		else if (r->src.port_op && !pf_match_port(r->src.port_op,
		    r->src.port[0], r->src.port[1], th->th_sport))
			r = r->skip[PF_SKIP_SRC_PORT].ptr;
		else if (r->dst.noroute && pf_routable(daddr, af))
			r = TAILQ_NEXT(r, entries);
		else if (!r->dst.noroute &&
		    !PF_AZERO(&r->dst.addr.mask, af) && !PF_MATCHA(r->dst.not,
		    &r->dst.addr.addr, &r->dst.addr.mask, daddr, af))
			r = r->skip[PF_SKIP_DST_ADDR].ptr;
		else if (r->dst.port_op && !pf_match_port(r->dst.port_op,
		    r->dst.port[0], r->dst.port[1], th->th_dport))
			r = r->skip[PF_SKIP_DST_PORT].ptr;
		else if (r->tos && !(r->tos & pd->tos))
			r = TAILQ_NEXT(r, entries);
		else if (r->rule_flag & PFRULE_FRAGMENT)
			r = TAILQ_NEXT(r, entries);
		else if ((r->flagset & th->th_flags) != r->flags)
			r = TAILQ_NEXT(r, entries);
		else if (r->uid.op && (lookup != -1 || (lookup =
		    pf_socket_lookup(&uid, &gid, direction, af, IPPROTO_TCP,
		    pd), 1)) &&
		    !pf_match_uid(r->uid.op, r->uid.uid[0], r->uid.uid[1],
		    uid))
			r = TAILQ_NEXT(r, entries);
		else if (r->gid.op && (lookup != -1 || (lookup =
		    pf_socket_lookup(&uid, &gid, direction, af, IPPROTO_TCP,
		    pd), 1)) &&
		    !pf_match_gid(r->gid.op, r->gid.gid[0], r->gid.gid[1],
		    gid))
			r = TAILQ_NEXT(r, entries);
		else if (r->anchorname[0] && r->anchor == NULL)
			r = TAILQ_NEXT(r, entries);
		else {
			if (r->anchor == NULL) {
				*rm = r;
				rs = (anchorrule == NULL ? r : anchorrule);
				if ((*rm)->quick)
					break;
				r = TAILQ_NEXT(r, entries);
			} else
				PF_STEP_INTO_ANCHOR(r, anchorrule, ruleset,
				    PF_RULESET_FILTER);
		}
		if (r == NULL && anchorrule != NULL)
			PF_STEP_OUT_OF_ANCHOR(r, anchorrule, ruleset,
			    PF_RULESET_FILTER);
	}

	if (*rm != NULL) {
		(*rm)->packets++;
		(*rm)->bytes += pd->tot_len;
		if (rs != *rm) {
			rs->packets++;
			rs->bytes += pd->tot_len;
		}
		REASON_SET(&reason, PFRES_MATCH);

		if ((*rm)->log) {
			if (rewrite)
				m_copyback(m, off, sizeof(*th), (caddr_t)th);
			PFLOG_PACKET(ifp, h, m, af, direction, reason, rs);
		}

		if (((*rm)->action == PF_DROP) &&
		    (((*rm)->rule_flag & PFRULE_RETURNRST) ||
		    ((*rm)->rule_flag & PFRULE_RETURNICMP) ||
		    ((*rm)->rule_flag & PFRULE_RETURN))) {
			/* undo NAT/RST changes, if they have taken place */
			if (nat != NULL) {
				pf_change_ap(saddr, &th->th_sport, pd->ip_sum,
				    &th->th_sum, &baddr, bport, 0, af);
				rewrite++;
			} else if (rdr != NULL) {
				pf_change_ap(daddr, &th->th_dport, pd->ip_sum,
				    &th->th_sum, &baddr, bport, 0, af);
				rewrite++;
			}
			if (((*rm)->rule_flag & PFRULE_RETURNRST) ||
			    ((*rm)->rule_flag & PFRULE_RETURN))
				pf_send_reset(off, th, pd, af,
				    (*rm)->return_ttl, *rm);
			else if ((af == AF_INET) && (*rm)->return_icmp)
				pf_send_icmp(m, (*rm)->return_icmp >> 8,
				    (*rm)->return_icmp & 255, af, *rm);
			else if ((af == AF_INET6) && (*rm)->return_icmp6)
				pf_send_icmp(m, (*rm)->return_icmp6 >> 8,
				    (*rm)->return_icmp6 & 255, af, *rm);
		}

		if ((*rm)->action == PF_DROP)
			return (PF_DROP);
	}

	if (((*rm != NULL) && (*rm)->keep_state) ||
	    nat != NULL || rdr != NULL) {
		/* create new state */
		u_int16_t	 len;
		struct pf_state	*s = NULL;

		len = pd->tot_len - off - (th->th_off << 2);
		if (*rm == NULL || !(*rm)->max_states ||
		    (*rm)->states < (*rm)->max_states)
			s = pool_get(&pf_state_pl, PR_NOWAIT);
		if (s == NULL) {
			REASON_SET(&reason, PFRES_MEMORY);
			return (PF_DROP);
		}
		bzero(s, sizeof(*s));
		if (rs != NULL)
			rs->states++;

		s->rule.ptr = rs;
		if (nat != NULL)
			s->nat_rule = nat;
		else
			s->nat_rule = rdr;
		if (s->nat_rule != NULL)
			s->nat_rule->states++;
		s->allow_opts = *rm && (*rm)->allow_opts;
		s->log = *rm && ((*rm)->log & 2);
		s->proto = IPPROTO_TCP;
		s->direction = direction;
		s->af = af;
		if (direction == PF_OUT) {
			PF_ACPY(&s->gwy.addr, saddr, af);
			s->gwy.port = th->th_sport;		/* sport */
			PF_ACPY(&s->ext.addr, daddr, af);
			s->ext.port = th->th_dport;
			if (nat != NULL) {
				PF_ACPY(&s->lan.addr, &baddr, af);
				s->lan.addr = baddr;
				s->lan.port = bport;
			} else {
				PF_ACPY(&s->lan.addr, &s->gwy.addr, af);
				s->lan.port = s->gwy.port;
			}
		} else {
			PF_ACPY(&s->lan.addr, daddr, af);
			s->lan.port = th->th_dport;
			PF_ACPY(&s->ext.addr, saddr, af);
			s->ext.port = th->th_sport;
			if (rdr != NULL) {
				PF_ACPY(&s->gwy.addr, &baddr, af);
				s->gwy.port = bport;
			} else {
				PF_ACPY(&s->gwy.addr, &s->lan.addr, af);
				s->gwy.port = s->lan.port;
			}
		}

		s->src.seqlo = ntohl(th->th_seq);
		s->src.seqhi = s->src.seqlo + len + 1;
		if ((th->th_flags & (TH_SYN|TH_ACK)) == TH_SYN &&
		    *rm != NULL && (*rm)->keep_state == PF_STATE_MODULATE) {
			/* Generate sequence number modulator */
			while ((s->src.seqdiff = arc4random()) == 0)
				;
			pf_change_a(&th->th_seq, &th->th_sum,
			    htonl(s->src.seqlo + s->src.seqdiff), 0);
			rewrite = 1;
		} else
			s->src.seqdiff = 0;
		if (th->th_flags & TH_SYN)
			s->src.seqhi++;
		if (th->th_flags & TH_FIN)
			s->src.seqhi++;
		s->src.max_win = MAX(ntohs(th->th_win), 1);
		s->dst.seqlo = 0;	/* Haven't seen these yet */
		s->dst.seqhi = 1;
		s->dst.max_win = 1;
		s->dst.seqdiff = 0;	/* Defer random generation */
		s->src.state = TCPS_SYN_SENT;
		s->dst.state = TCPS_CLOSED;
		s->creation = time.tv_sec;
		s->expire = s->creation + TIMEOUT(*rm, PFTM_TCP_FIRST_PACKET);
		s->packets = 1;
		s->bytes = pd->tot_len;
		if (pf_insert_state(s)) {
			REASON_SET(&reason, PFRES_MEMORY);
			pool_put(&pf_state_pl, s);
			return (PF_DROP);
		}
	}

	/* copy back packet headers if we performed NAT operations */
	if (rewrite)
		m_copyback(m, off, sizeof(*th), (caddr_t)th);

	return (PF_PASS);
}

int
pf_test_udp(struct pf_rule **rm, int direction, struct ifnet *ifp,
    struct mbuf *m, int ipoff, int off, void *h, struct pf_pdesc *pd)
{
	struct pf_rule		*nat = NULL, *rdr = NULL;
	struct pf_addr		*saddr = pd->src, *daddr = pd->dst;
	struct pf_addr		 baddr, naddr;
	struct udphdr		*uh = pd->hdr.udp;
	u_int16_t		 bport, nport = 0;
	sa_family_t		 af = pd->af;
	int			 lookup = -1;
	uid_t			 uid;
	gid_t			 gid;
	struct pf_rule		*r, *rs = NULL, *anchorrule = NULL;
	struct pf_ruleset	*ruleset = NULL;
	u_short			 reason;
	int			 rewrite = 0;

	*rm = NULL;

	if (direction == PF_OUT) {
		bport = nport = uh->uh_sport;
		/* check outgoing packet for BINAT/NAT */
		if ((nat = pf_get_translation(PF_OUT, ifp, IPPROTO_UDP,
	            saddr, uh->uh_sport, daddr, uh->uh_dport,
		    &naddr, &nport, af)) != NULL) {
			PF_ACPY(&baddr, saddr, af);
			pf_change_ap(saddr, &uh->uh_sport, pd->ip_sum,
			    &uh->uh_sum, &naddr, uh->uh_sport, 1, af);
			rewrite++;
		}
	} else {
		bport = nport = uh->uh_dport;
		/* check incoming packet for BINAT/RDR */
		if ((rdr = pf_get_translation(PF_IN, ifp, IPPROTO_UDP,
	            saddr, uh->uh_sport, daddr, uh->uh_dport,
		    &naddr, &nport, af)) != NULL) {
			PF_ACPY(&baddr, daddr, af);
			pf_change_ap(daddr, &uh->uh_dport, pd->ip_sum,
			    &uh->uh_sum, &naddr, nport, 1, af);
			rewrite++;
		}
	}

	r = TAILQ_FIRST(pf_main_ruleset.rules[PF_RULESET_FILTER].active.ptr);
	while (r != NULL) {
		r->evaluations++;
		if (r->ifp != NULL && ((r->ifp != ifp && !r->ifnot) ||
		    (r->ifp == ifp && r->ifnot)))
			r = r->skip[PF_SKIP_IFP].ptr;
		else if (r->direction && r->direction != direction)
			r = r->skip[PF_SKIP_DIR].ptr;
		else if (r->af && r->af != af)
			r = r->skip[PF_SKIP_AF].ptr;
		else if (r->proto && r->proto != IPPROTO_UDP)
			r = r->skip[PF_SKIP_PROTO].ptr;
		else if (r->src.noroute && pf_routable(saddr, af))
			r = TAILQ_NEXT(r, entries);
		else if (!r->src.noroute &&
		    !PF_AZERO(&r->src.addr.mask, af) &&
		    !PF_MATCHA(r->src.not, &r->src.addr.addr, &r->src.addr.mask,
		    saddr, af))
			r = r->skip[PF_SKIP_SRC_ADDR].ptr;
		else if (r->src.port_op && !pf_match_port(r->src.port_op,
		    r->src.port[0], r->src.port[1], uh->uh_sport))
			r = r->skip[PF_SKIP_SRC_PORT].ptr;
		else if (r->dst.noroute && pf_routable(daddr, af))
			r = TAILQ_NEXT(r, entries);
		else if (!r->dst.noroute &&
		    !PF_AZERO(&r->dst.addr.mask, af) &&
		    !PF_MATCHA(r->dst.not, &r->dst.addr.addr, &r->dst.addr.mask,
			daddr, af))
			r = r->skip[PF_SKIP_DST_ADDR].ptr;
		else if (r->dst.port_op && !pf_match_port(r->dst.port_op,
		    r->dst.port[0], r->dst.port[1], uh->uh_dport))
			r = r->skip[PF_SKIP_DST_PORT].ptr;
		else if (r->tos && !(r->tos & pd->tos))
			r = TAILQ_NEXT(r, entries);
		else if (r->rule_flag & PFRULE_FRAGMENT)
			r = TAILQ_NEXT(r, entries);
		else if (r->uid.op && (lookup != -1 || (lookup =
		    pf_socket_lookup(&uid, &gid, direction, af, IPPROTO_UDP,
		    pd), 1)) &&
		    !pf_match_uid(r->uid.op, r->uid.uid[0], r->uid.uid[1],
		    uid))
			r = TAILQ_NEXT(r, entries);
		else if (r->gid.op && (lookup != -1 || (lookup =
		    pf_socket_lookup(&uid, &gid, direction, af, IPPROTO_UDP,
		    pd), 1)) &&
		    !pf_match_gid(r->gid.op, r->gid.gid[0], r->gid.gid[1],
		    gid))
			r = TAILQ_NEXT(r, entries);
		else if (r->anchorname[0] && r->anchor == NULL)
			r = TAILQ_NEXT(r, entries);
		else {
			if (r->anchor == NULL) {
				*rm = r;
				rs = (anchorrule == NULL ? r : anchorrule);
				if ((*rm)->quick)
					break;
				r = TAILQ_NEXT(r, entries);
			} else
				PF_STEP_INTO_ANCHOR(r, anchorrule, ruleset,
				    PF_RULESET_FILTER);
		}
		if (r == NULL && anchorrule != NULL)
			PF_STEP_OUT_OF_ANCHOR(r, anchorrule, ruleset,
			    PF_RULESET_FILTER);
	}

	if (*rm != NULL) {
		(*rm)->packets++;
		(*rm)->bytes += pd->tot_len;
		if (rs != *rm) {
			rs->packets++;
			rs->bytes += pd->tot_len;
		}
		REASON_SET(&reason, PFRES_MATCH);

		if ((*rm)->log) {
			if (rewrite)
				m_copyback(m, off, sizeof(*uh), (caddr_t)uh);
			PFLOG_PACKET(ifp, h, m, af, direction, reason, rs);
		}

		if (((*rm)->action == PF_DROP) &&
		    (((*rm)->rule_flag & PFRULE_RETURNICMP) ||
		    ((*rm)->rule_flag & PFRULE_RETURN))) {
			/* undo NAT/RST changes, if they have taken place */
			if (nat != NULL) {
				pf_change_ap(saddr, &uh->uh_sport, pd->ip_sum,
				    &uh->uh_sum, &baddr, bport, 1, af);
				rewrite++;
			} else if (rdr != NULL) {
				pf_change_ap(daddr, &uh->uh_dport, pd->ip_sum,
				    &uh->uh_sum, &baddr, bport, 1, af);
				rewrite++;
			}
			if ((af == AF_INET) && (*rm)->return_icmp)
				pf_send_icmp(m, (*rm)->return_icmp >> 8,
				    (*rm)->return_icmp & 255, af, *rm);
			else if ((af == AF_INET6) && (*rm)->return_icmp6)
				pf_send_icmp(m, (*rm)->return_icmp6 >> 8,
				    (*rm)->return_icmp6 & 255, af, *rm);
		}

		if ((*rm)->action == PF_DROP)
			return (PF_DROP);
	}

	if ((*rm != NULL && (*rm)->keep_state) ||
	    nat != NULL || rdr != NULL) {
		/* create new state */
		struct pf_state	*s = NULL;

		if (*rm == NULL || !(*rm)->max_states ||
		    (*rm)->states < (*rm)->max_states)
			s = pool_get(&pf_state_pl, PR_NOWAIT);
		if (s == NULL)
			return (PF_DROP);
		bzero(s, sizeof(*s));
		if (rs != NULL)
			rs->states++;

		s->rule.ptr = rs;
		if (nat != NULL)
			s->nat_rule = nat;
		else
			s->nat_rule = rdr;
		if (s->nat_rule != NULL)
			s->nat_rule->states++;
		s->allow_opts = *rm && (*rm)->allow_opts;
		s->log = *rm && ((*rm)->log & 2);
		s->proto = IPPROTO_UDP;
		s->direction = direction;
		s->af = af;
		if (direction == PF_OUT) {
			PF_ACPY(&s->gwy.addr, saddr, af);
			s->gwy.port = uh->uh_sport;
			PF_ACPY(&s->ext.addr, daddr, af);
			s->ext.port = uh->uh_dport;
			if (nat != NULL) {
				PF_ACPY(&s->lan.addr, &baddr, af);
				s->lan.port = bport;
			} else {
				PF_ACPY(&s->lan.addr, &s->gwy.addr, af);
				s->lan.port = s->gwy.port;
			}
		} else {
			PF_ACPY(&s->lan.addr, daddr, af);
			s->lan.port = uh->uh_dport;
			PF_ACPY(&s->ext.addr, saddr, af);
			s->ext.port = uh->uh_sport;
			if (rdr != NULL) {
				PF_ACPY(&s->gwy.addr, &baddr, af);
				s->gwy.port = bport;
			} else {
				PF_ACPY(&s->gwy.addr, &s->lan.addr, af);
				s->gwy.port = s->lan.port;
			}
		}
		s->src.seqlo = 0;
		s->src.seqhi = 0;
		s->src.seqdiff = 0;
		s->src.max_win = 0;
		s->src.state = PFUDPS_SINGLE;
		s->dst.seqlo = 0;
		s->dst.seqhi = 0;
		s->dst.seqdiff = 0;
		s->dst.max_win = 0;
		s->dst.state = PFUDPS_NO_TRAFFIC;
		s->creation = time.tv_sec;
		s->expire = s->creation + TIMEOUT(*rm, PFTM_UDP_FIRST_PACKET);
		s->packets = 1;
		s->bytes = pd->tot_len;
		if (pf_insert_state(s)) {
			REASON_SET(&reason, PFRES_MEMORY);
			pool_put(&pf_state_pl, s);
			return (PF_DROP);
		}
	}

	/* copy back packet headers if we performed NAT operations */
	if (rewrite)
		m_copyback(m, off, sizeof(*uh), (caddr_t)uh);

	return (PF_PASS);
}

int
pf_test_icmp(struct pf_rule **rm, int direction, struct ifnet *ifp,
    struct mbuf *m, int ipoff, int off, void *h, struct pf_pdesc *pd)
{
	struct pf_rule		*nat = NULL, *rdr = NULL;
	struct pf_addr		*saddr = pd->src, *daddr = pd->dst;
	struct pf_addr		 baddr, naddr;
	struct pf_rule		*r, *rs = NULL, *anchorrule = NULL;
	struct pf_ruleset	*ruleset = NULL;
	u_short			 reason;
	u_int16_t		 icmpid;
	sa_family_t		 af = pd->af;
	u_int8_t		 icmptype, icmpcode;
	int			 state_icmp = 0;
#ifdef INET6
	int			 rewrite = 0;
#endif /* INET6 */

	*rm = NULL;

	switch (pd->proto) {
#ifdef INET
	case IPPROTO_ICMP:
		icmptype = pd->hdr.icmp->icmp_type;
		icmpcode = pd->hdr.icmp->icmp_code;
		icmpid = pd->hdr.icmp->icmp_id;

		if (icmptype == ICMP_UNREACH ||
		    icmptype == ICMP_SOURCEQUENCH ||
		    icmptype == ICMP_REDIRECT ||
		    icmptype == ICMP_TIMXCEED ||
		    icmptype == ICMP_PARAMPROB)
			state_icmp++;
		break;
#endif /* INET */
#ifdef INET6
	case IPPROTO_ICMPV6:
		icmptype = pd->hdr.icmp6->icmp6_type;
		icmpcode = pd->hdr.icmp6->icmp6_code;
		icmpid = pd->hdr.icmp6->icmp6_id;

		if (icmptype == ICMP6_DST_UNREACH ||
		    icmptype == ICMP6_PACKET_TOO_BIG ||
		    icmptype == ICMP6_TIME_EXCEEDED ||
		    icmptype == ICMP6_PARAM_PROB)
			state_icmp++;
		break;
#endif /* INET6 */
	}

	if (direction == PF_OUT) {
		/* check outgoing packet for BINAT/NAT */
		if ((nat = pf_get_translation(PF_OUT, ifp, pd->proto,
	            saddr, 0, daddr, 0, &naddr, NULL, af)) != NULL) {
			PF_ACPY(&baddr, saddr, af);
			switch (af) {
#ifdef INET
			case AF_INET:
				pf_change_a(&saddr->v4.s_addr, pd->ip_sum,
				    naddr.v4.s_addr, 0);
				break;
#endif /* INET */
#ifdef INET6
			case AF_INET6:
				pf_change_a6(saddr, &pd->hdr.icmp6->icmp6_cksum,
				    &naddr, 0);
				rewrite++;
				break;
#endif /* INET6 */
			}
		}
	} else {
		/* check incoming packet for BINAT/RDR */
		if ((rdr = pf_get_translation(PF_IN, ifp, pd->proto,
	            saddr, 0, daddr, 0, &naddr, NULL, af)) != NULL) {
			PF_ACPY(&baddr, daddr, af);
			switch (af) {
#ifdef INET
			case AF_INET:
				pf_change_a(&daddr->v4.s_addr,
				    pd->ip_sum, naddr.v4.s_addr, 0);
				break;
#endif /* INET */
#ifdef INET6
			case AF_INET6:
				pf_change_a6(daddr, &pd->hdr.icmp6->icmp6_cksum,
				    &naddr, 0);
				rewrite++;
				break;
#endif /* INET6 */
			}
		}
	}

	r = TAILQ_FIRST(pf_main_ruleset.rules[PF_RULESET_FILTER].active.ptr);
	while (r != NULL) {
		r->evaluations++;
		if (r->ifp != NULL && ((r->ifp != ifp && !r->ifnot) ||
		    (r->ifp == ifp && r->ifnot)))
			r = r->skip[PF_SKIP_IFP].ptr;
		else if (r->direction && r->direction != direction)
			r = r->skip[PF_SKIP_DIR].ptr;
		else if (r->af && r->af != af)
			r = r->skip[PF_SKIP_AF].ptr;
		else if (r->proto && r->proto != pd->proto)
			r = r->skip[PF_SKIP_PROTO].ptr;
		else if (r->src.noroute && pf_routable(saddr, af))
			r = TAILQ_NEXT(r, entries);
		else if (!r->src.noroute &&
		    !PF_AZERO(&r->src.addr.mask, af) && !PF_MATCHA(r->src.not,
		    &r->src.addr.addr, &r->src.addr.mask, saddr, af))
			r = r->skip[PF_SKIP_SRC_ADDR].ptr;
		else if (r->dst.noroute && pf_routable(daddr, af))
			r = TAILQ_NEXT(r, entries);
		else if (!r->dst.noroute &&
		    !PF_AZERO(&r->dst.addr.mask, af) && !PF_MATCHA(r->dst.not,
		    &r->dst.addr.addr, &r->dst.addr.mask, daddr, af))
			r = r->skip[PF_SKIP_DST_ADDR].ptr;
		else if (r->type && r->type != icmptype + 1)
			r = TAILQ_NEXT(r, entries);
		else if (r->code && r->code != icmpcode + 1)
			r = TAILQ_NEXT(r, entries);
		else if (r->tos && !(r->tos & pd->tos))
			r = TAILQ_NEXT(r, entries);
		else if (r->rule_flag & PFRULE_FRAGMENT)
			r = TAILQ_NEXT(r, entries);
		else if (r->anchorname[0] && r->anchor == NULL)
			r = TAILQ_NEXT(r, entries);
		else {
			if (r->anchor == NULL) {
				*rm = r;
				rs = (anchorrule == NULL ? r : anchorrule);
				if ((*rm)->quick)
					break;
				r = TAILQ_NEXT(r, entries);
			} else
				PF_STEP_INTO_ANCHOR(r, anchorrule, ruleset,
				    PF_RULESET_FILTER);
		}
		if (r == NULL && anchorrule != NULL)
			PF_STEP_OUT_OF_ANCHOR(r, anchorrule, ruleset,
			    PF_RULESET_FILTER);
	}

	if (*rm != NULL) {
		(*rm)->packets++;
		(*rm)->bytes += pd->tot_len;
		if (rs != *rm) {
			rs->packets++;
			rs->bytes += pd->tot_len;
		}
		REASON_SET(&reason, PFRES_MATCH);

		if ((*rm)->log) {
#ifdef INET6
			if (rewrite)
				m_copyback(m, off, ICMP_MINLEN,
				    (caddr_t)pd->hdr.icmp6);
#endif /* INET6 */
			PFLOG_PACKET(ifp, h, m, af, direction, reason, rs);
		}

		if ((*rm)->action != PF_PASS)
			return (PF_DROP);
	}

	if (!state_icmp && ((*rm != NULL && (*rm)->keep_state) ||
	    nat != NULL || rdr != NULL)) {
		/* create new state */
		struct pf_state	*s = NULL;

		if (*rm == NULL || !(*rm)->max_states ||
		    (*rm)->states < (*rm)->max_states)
			s = pool_get(&pf_state_pl, PR_NOWAIT);
		if (s == NULL)
			return (PF_DROP);
		bzero(s, sizeof(*s));
		if (rs != NULL)
			rs->states++;

		s->rule.ptr = rs;
		if (nat != NULL)
			s->nat_rule = nat;
		else
			s->nat_rule = rdr;
		if (s->nat_rule != NULL)
			s->nat_rule->states++;
		s->allow_opts = *rm && (*rm)->allow_opts;
		s->log = *rm && ((*rm)->log & 2);
		s->proto = pd->proto;
		s->direction = direction;
		s->af = af;
		if (direction == PF_OUT) {
			PF_ACPY(&s->gwy.addr, saddr, af);
			s->gwy.port = icmpid;
			PF_ACPY(&s->ext.addr, daddr, af);
			s->ext.port = icmpid;
			if (nat != NULL)
				PF_ACPY(&s->lan.addr, &baddr, af);
			else
				PF_ACPY(&s->lan.addr, &s->gwy.addr, af);
			s->lan.port = icmpid;
		} else {
			PF_ACPY(&s->lan.addr, daddr, af);
			s->lan.port = icmpid;
			PF_ACPY(&s->ext.addr, saddr, af);
			s->ext.port = icmpid;
			if (rdr != NULL)
				PF_ACPY(&s->gwy.addr, &baddr, af);
			else
				PF_ACPY(&s->gwy.addr, &s->lan.addr, af);
			s->gwy.port = icmpid;
		}
		s->src.seqlo = 0;
		s->src.seqhi = 0;
		s->src.seqdiff = 0;
		s->src.max_win = 0;
		s->src.state = 0;
		s->dst.seqlo = 0;
		s->dst.seqhi = 0;
		s->dst.seqdiff = 0;
		s->dst.max_win = 0;
		s->dst.state = 0;
		s->creation = time.tv_sec;
		s->expire = s->creation + TIMEOUT(*rm, PFTM_ICMP_FIRST_PACKET);
		s->packets = 1;
		s->bytes = pd->tot_len;
		if (pf_insert_state(s)) {
			REASON_SET(&reason, PFRES_MEMORY);
			pool_put(&pf_state_pl, s);
			return (PF_DROP);
		}
	}

#ifdef INET6
	/* copy back packet headers if we performed IPv6 NAT operations */
	if (rewrite)
		m_copyback(m, off, ICMP_MINLEN,
		    (caddr_t)pd->hdr.icmp6);
#endif /* INET6 */

	return (PF_PASS);
}

int
pf_test_other(struct pf_rule **rm, int direction, struct ifnet *ifp,
    struct mbuf *m, void *h, struct pf_pdesc *pd)
{
	struct pf_rule		*nat = NULL, *rdr = NULL;
	struct pf_rule		*r, *rs = NULL, *anchorrule = NULL;
	struct pf_ruleset	*ruleset = NULL;
	struct pf_addr		*saddr = pd->src, *daddr = pd->dst;
	struct pf_addr		 baddr, naddr;
	sa_family_t		 af = pd->af;
	u_short			 reason;

	*rm = NULL;

	if (direction == PF_OUT) {
		/* check outgoing packet for BINAT/NAT */
		if ((nat = pf_get_translation(PF_OUT, ifp, pd->proto,
	            saddr, 0, daddr, 0, &naddr, NULL, af)) != NULL) {
			PF_ACPY(&baddr, saddr, af);
			switch (af) {
#ifdef INET
			case AF_INET:
				pf_change_a(&saddr->v4.s_addr, pd->ip_sum,
				    naddr.v4.s_addr, 0);
				break;
#endif /* INET */
#ifdef INET6
			case AF_INET6:
				PF_ACPY(saddr, &naddr, af);
				break;
#endif /* INET6 */
			}
		}
	} else {
		/* check incoming packet for BINAT/RDR */
		if ((rdr = pf_get_translation(PF_IN, ifp, pd->proto,
	            saddr, 0, daddr, 0, &naddr, NULL, af)) != NULL) {
			switch (af) {
#ifdef INET
			case AF_INET:
				pf_change_a(&daddr->v4.s_addr,
				    pd->ip_sum, naddr.v4.s_addr, 0);
				break;
#endif /* INET */
#ifdef INET6
			case AF_INET6:
				PF_ACPY(daddr, &naddr, af);
				break;
#endif /* INET6 */
			}
		}
	}

	r = TAILQ_FIRST(pf_main_ruleset.rules[PF_RULESET_FILTER].active.ptr);
	while (r != NULL) {
		r->evaluations++;
		if (r->ifp != NULL && ((r->ifp != ifp && !r->ifnot) ||
		    (r->ifp == ifp && r->ifnot)))
			r = r->skip[PF_SKIP_IFP].ptr;
		else if (r->direction && r->direction != direction)
			r = r->skip[PF_SKIP_DIR].ptr;
		else if (r->af && r->af != af)
			r = r->skip[PF_SKIP_AF].ptr;
		else if (r->proto && r->proto != pd->proto)
			r = r->skip[PF_SKIP_PROTO].ptr;
		else if (r->src.noroute && pf_routable(pd->src, af))
			r = TAILQ_NEXT(r, entries);
		else if (!r->src.noroute &&
		    !PF_AZERO(&r->src.addr.mask, af) && !PF_MATCHA(r->src.not,
		    &r->src.addr.addr, &r->src.addr.mask, pd->src, af))
			r = r->skip[PF_SKIP_SRC_ADDR].ptr;
		else if (r->dst.noroute && pf_routable(pd->dst, af))
			r = TAILQ_NEXT(r, entries);
		else if (!r->src.noroute &&
		    !PF_AZERO(&r->dst.addr.mask, af) && !PF_MATCHA(r->dst.not,
		    &r->dst.addr.addr, &r->dst.addr.mask, pd->dst, af))
			r = r->skip[PF_SKIP_DST_ADDR].ptr;
		else if (r->tos && !(r->tos & pd->tos))
			r = TAILQ_NEXT(r, entries);
		else if (r->rule_flag & PFRULE_FRAGMENT)
			r = TAILQ_NEXT(r, entries);
		else if (r->anchorname[0] && r->anchor == NULL)
			r = TAILQ_NEXT(r, entries);
		else {
			if (r->anchor == NULL) {
				*rm = r;
				rs = (anchorrule == NULL ? r : anchorrule);
				if ((*rm)->quick)
					break;
				r = TAILQ_NEXT(r, entries);
			} else
				PF_STEP_INTO_ANCHOR(r, anchorrule, ruleset,
				    PF_RULESET_FILTER);
		}
		if (r == NULL && anchorrule != NULL)
			PF_STEP_OUT_OF_ANCHOR(r, anchorrule, ruleset,
			    PF_RULESET_FILTER);
	}

	if (*rm != NULL) {
		(*rm)->packets++;
		(*rm)->bytes += pd->tot_len;
		if (rs != *rm) {
			rs->packets++;
			rs->bytes += pd->tot_len;
		}
		REASON_SET(&reason, PFRES_MATCH);
		if ((*rm)->log)
			PFLOG_PACKET(ifp, h, m, af, direction, reason, rs);

		if ((*rm)->action != PF_PASS)
			return (PF_DROP);
	}

	if ((*rm != NULL && (*rm)->keep_state) || nat != NULL ||
	    rdr != NULL) {
		/* create new state */
		struct pf_state	*s = NULL;

		if (*rm == NULL || !(*rm)->max_states ||
		    (*rm)->states < (*rm)->max_states)
			s = pool_get(&pf_state_pl, PR_NOWAIT);
		if (s == NULL)
			return (PF_DROP);
		bzero(s, sizeof(*s));
		if (rs != NULL)
			rs->states++;

		s->rule.ptr = rs;
		if (nat != NULL)
			s->nat_rule = nat;
		else
			s->nat_rule = rdr;
		if (s->nat_rule != NULL)
			s->nat_rule->states++;
		s->allow_opts = *rm && (*rm)->allow_opts;
		s->log = *rm && ((*rm)->log & 2);
		s->proto = pd->proto;
		s->direction = direction;
		s->af = af;
		if (direction == PF_OUT) {
			PF_ACPY(&s->gwy.addr, saddr, af);
			s->gwy.port = 0;
			PF_ACPY(&s->ext.addr, daddr, af);
			s->ext.port = 0;
			if (nat != NULL)
				PF_ACPY(&s->lan.addr, &baddr, af);
			else
				PF_ACPY(&s->lan.addr, &s->gwy.addr, af);
			s->lan.port = 0;
		} else {
			PF_ACPY(&s->lan.addr, daddr, af);
			s->lan.port = 0;
			PF_ACPY(&s->ext.addr, saddr, af);
			s->ext.port = 0;
			if (rdr != NULL)
				PF_ACPY(&s->gwy.addr, &baddr, af);
			else
				PF_ACPY(&s->gwy.addr, &s->lan.addr, af);
			s->gwy.port = 0;
		}
		s->src.seqlo = 0;
		s->src.seqhi = 0;
		s->src.seqdiff = 0;
		s->src.max_win = 0;
		s->src.state = PFOTHERS_SINGLE;
		s->dst.seqlo = 0;
		s->dst.seqhi = 0;
		s->dst.seqdiff = 0;
		s->dst.max_win = 0;
		s->dst.state = PFOTHERS_NO_TRAFFIC;
		s->creation = time.tv_sec;
		s->expire = s->creation + TIMEOUT(*rm, PFTM_OTHER_FIRST_PACKET);
		s->packets = 1;
		s->bytes = pd->tot_len;
		if (pf_insert_state(s)) {
			REASON_SET(&reason, PFRES_MEMORY);
			if (*rm && (*rm)->log)
				PFLOG_PACKET(ifp, h, m, af, direction, reason,
				    rs);
			pool_put(&pf_state_pl, s);
			return (PF_DROP);
		}
	}

	return (PF_PASS);
}

int
pf_test_fragment(struct pf_rule **rm, int direction, struct ifnet *ifp,
    struct mbuf *m, void *h, struct pf_pdesc *pd)
{
	struct pf_rule		*r, *rs = NULL, *anchorrule = NULL;
	struct pf_ruleset	*ruleset = NULL;
	sa_family_t		 af = pd->af;

	*rm = NULL;

	r = TAILQ_FIRST(pf_main_ruleset.rules[PF_RULESET_FILTER].active.ptr);
	while (r != NULL) {
		r->evaluations++;
		if (r->ifp != NULL && ((r->ifp != ifp && !r->ifnot) ||
		    (r->ifp == ifp && r->ifnot)))
			r = r->skip[PF_SKIP_IFP].ptr;
		else if (r->direction && r->direction != direction)
			r = r->skip[PF_SKIP_DIR].ptr;
		else if (r->af && r->af != af)
			r = r->skip[PF_SKIP_AF].ptr;
		else if (r->proto && r->proto != pd->proto)
			r = r->skip[PF_SKIP_PROTO].ptr;
		else if (r->src.noroute && pf_routable(pd->src, af))
			r = TAILQ_NEXT(r, entries);
		else if (!r->src.noroute &&
		    !PF_AZERO(&r->src.addr.mask, af) && !PF_MATCHA(r->src.not,
		    &r->src.addr.addr, &r->src.addr.mask, pd->src, af))
			r = r->skip[PF_SKIP_SRC_ADDR].ptr;
		else if (r->dst.noroute && pf_routable(pd->dst, af))
			r = TAILQ_NEXT(r, entries);
		else if (!r->src.noroute &&
		    !PF_AZERO(&r->dst.addr.mask, af) && !PF_MATCHA(r->dst.not,
		    &r->dst.addr.addr, &r->dst.addr.mask, pd->dst, af))
			r = r->skip[PF_SKIP_DST_ADDR].ptr;
		else if (r->tos && !(r->tos & pd->tos))
			r = TAILQ_NEXT(r, entries);
		else if (r->src.port_op || r->dst.port_op ||
		    r->flagset || r->type || r->code)
			r = TAILQ_NEXT(r, entries);
		else if (r->anchorname[0] && r->anchor == NULL)
			r = TAILQ_NEXT(r, entries);
		else {
			if (r->anchor == NULL) {
				*rm = r;
				rs = (anchorrule == NULL ? r : anchorrule);
				if ((*rm)->quick)
					break;
				r = TAILQ_NEXT(r, entries);
			} else
				PF_STEP_INTO_ANCHOR(r, anchorrule, ruleset,
				    PF_RULESET_FILTER);
		}
		if (r == NULL && anchorrule != NULL)
			PF_STEP_OUT_OF_ANCHOR(r, anchorrule, ruleset,
			    PF_RULESET_FILTER);
	}

	if (*rm != NULL) {
		u_short	reason;

		(*rm)->packets++;
		(*rm)->bytes += pd->tot_len;
		if (rs != *rm) {
			rs->packets++;
			rs->bytes += pd->tot_len;
		}
		REASON_SET(&reason, PFRES_MATCH);
		if ((*rm)->log)
			PFLOG_PACKET(ifp, h, m, af, direction, reason, rs);

		if ((*rm)->action != PF_PASS)
			return (PF_DROP);
	}

	return (PF_PASS);
}

int
pf_test_state_tcp(struct pf_state **state, int direction, struct ifnet *ifp,
    struct mbuf *m, int ipoff, int off, void *h, struct pf_pdesc *pd)
{
	struct pf_tree_node	 key;
	struct tcphdr		*th = pd->hdr.tcp;
	u_int16_t		 win = ntohs(th->th_win);
	u_int32_t		 ack, end, seq;
	int			 ackskew;
	struct pf_state_peer	*src, *dst;

	key.af = pd->af;
	key.proto = IPPROTO_TCP;
	PF_ACPY(&key.addr[0], pd->src, key.af);
	PF_ACPY(&key.addr[1], pd->dst, key.af);
	key.port[0] = th->th_sport;
	key.port[1] = th->th_dport;

	if (direction == PF_IN)
		*state = pf_find_state(&tree_ext_gwy, &key);
	else
		*state = pf_find_state(&tree_lan_ext, &key);
	if (*state == NULL)
		return (PF_DROP);

	if (direction == (*state)->direction) {
		src = &(*state)->src;
		dst = &(*state)->dst;
	} else {
		src = &(*state)->dst;
		dst = &(*state)->src;
	}

	/*
	 * Sequence tracking algorithm from Guido van Rooij's paper:
	 *   http://www.madison-gurkha.com/publications/tcp_filtering/
	 *	tcp_filtering.ps
	 */

	seq = ntohl(th->th_seq);
	if (src->seqlo == 0) {
		/* First packet from this end. Set its state */

		/* Deferred generation of sequence number modulator */
		if (dst->seqdiff) {
			while ((src->seqdiff = arc4random()) == 0)
				;
			ack = ntohl(th->th_ack) - dst->seqdiff;
			pf_change_a(&th->th_seq, &th->th_sum, htonl(seq +
			    src->seqdiff), 0);
			pf_change_a(&th->th_ack, &th->th_sum, htonl(ack), 0);
		} else {
			ack = ntohl(th->th_ack);
		}

		end = seq + pd->p_len;
		if (th->th_flags & TH_SYN)
			end++;
		if (th->th_flags & TH_FIN)
			end++;

		src->seqlo = seq;
		if (src->state < TCPS_SYN_SENT)
			src->state = TCPS_SYN_SENT;

		/*
		 * May need to slide the window (seqhi may have been set by
		 * the crappy stack check or if we picked up the connection
		 * after establishment)
		 */
		if (src->seqhi == 1 ||
		    SEQ_GEQ(end + MAX(1, dst->max_win), src->seqhi))
			src->seqhi = end + MAX(1, dst->max_win);
		if (win > src->max_win)
			src->max_win = win;

	} else {
		ack = ntohl(th->th_ack) - dst->seqdiff;
		if (src->seqdiff) {
			/* Modulate sequence numbers */
			pf_change_a(&th->th_seq, &th->th_sum, htonl(seq +
			    src->seqdiff), 0);
			pf_change_a(&th->th_ack, &th->th_sum, htonl(ack), 0);
		}
		end = seq + pd->p_len;
		if (th->th_flags & TH_SYN)
			end++;
		if (th->th_flags & TH_FIN)
			end++;
	}

	if ((th->th_flags & TH_ACK) == 0) {
		/* Let it pass through the ack skew check */
		ack = dst->seqlo;
	} else if ((ack == 0 &&
	    (th->th_flags & (TH_ACK|TH_RST)) == (TH_ACK|TH_RST)) ||
	    /* broken tcp stacks do not set ack */
	    (dst->state < TCPS_SYN_SENT)) {
	    /* Many stacks (ours included) will set the ACK number in an
	     * FIN|ACK if the SYN times out -- no sequence to ACK.
	     */
		ack = dst->seqlo;
	}

	if (seq == end) {
		/* Ease sequencing restrictions on no data packets */
		seq = src->seqlo;
		end = seq;
	}

	ackskew = dst->seqlo - ack;

#define MAXACKWINDOW (0xffff + 1500)	/* 1500 is an arbitrary fudge factor */
	if (SEQ_GEQ(src->seqhi, end) &&
	    /* Last octet inside other's window space */
	    SEQ_GEQ(seq, src->seqlo - dst->max_win) &&
	    /* Retrans: not more than one window back */
	    (ackskew >= -MAXACKWINDOW) &&
	    /* Acking not more than one window back */
	    (ackskew <= MAXACKWINDOW)) {
	    /* Acking not more than one window forward */

		(*state)->packets++;
		(*state)->bytes += pd->tot_len;

		/* update max window */
		if (src->max_win < win)
			src->max_win = win;
		/* synchronize sequencing */
		if (SEQ_GT(end, src->seqlo))
			src->seqlo = end;
		/* slide the window of what the other end can send */
		if (SEQ_GEQ(ack + win, dst->seqhi))
			dst->seqhi = ack + MAX(win, 1);


		/* update states */
		if (th->th_flags & TH_SYN)
			if (src->state < TCPS_SYN_SENT)
				src->state = TCPS_SYN_SENT;
		if (th->th_flags & TH_FIN)
			if (src->state < TCPS_CLOSING)
				src->state = TCPS_CLOSING;
		if (th->th_flags & TH_ACK) {
			if (dst->state == TCPS_SYN_SENT)
				dst->state = TCPS_ESTABLISHED;
			else if (dst->state == TCPS_CLOSING)
				dst->state = TCPS_FIN_WAIT_2;
		}
		if (th->th_flags & TH_RST)
			src->state = dst->state = TCPS_TIME_WAIT;

		/* update expire time */
		if (src->state >= TCPS_FIN_WAIT_2 &&
		    dst->state >= TCPS_FIN_WAIT_2)
			(*state)->expire = time.tv_sec +
			    TIMEOUT((*state)->rule.ptr, PFTM_TCP_CLOSED);
		else if (src->state >= TCPS_FIN_WAIT_2 ||
		    dst->state >= TCPS_FIN_WAIT_2)
			(*state)->expire = time.tv_sec +
			    TIMEOUT((*state)->rule.ptr, PFTM_TCP_FIN_WAIT);
		else if (src->state >= TCPS_CLOSING ||
		    dst->state >= TCPS_CLOSING)
			(*state)->expire = time.tv_sec +
			    TIMEOUT((*state)->rule.ptr, PFTM_TCP_CLOSING);
		else if (src->state < TCPS_ESTABLISHED ||
		    dst->state < TCPS_ESTABLISHED)
			(*state)->expire = time.tv_sec +
			    TIMEOUT((*state)->rule.ptr, PFTM_TCP_OPENING);
		else
			(*state)->expire = time.tv_sec +
			    TIMEOUT((*state)->rule.ptr, PFTM_TCP_ESTABLISHED);

		/* Fall through to PASS packet */

	} else if ((dst->state < TCPS_SYN_SENT ||
		dst->state >= TCPS_FIN_WAIT_2 ||
		src->state >= TCPS_FIN_WAIT_2) &&
	    SEQ_GEQ(src->seqhi + MAXACKWINDOW, end) &&
	    /* Within a window forward of the originating packet */
	    SEQ_GEQ(seq, src->seqlo - MAXACKWINDOW)) {
	    /* Within a window backward of the originating packet */

		/*
		 * This currently handles three situations:
		 *  1) Stupid stacks will shotgun SYNs before their peer
		 *     replies.
		 *  2) When PF catches an already established stream (the
		 *     firewall rebooted, the state table was flushed, routes
		 *     changed...)
		 *  3) Packets get funky immediately after the connection
		 *     closes (this should catch Solaris spurious ACK|FINs
		 *     that web servers like to spew after a close)
		 *
		 * This must be a little more careful than the above code
		 * since packet floods will also be caught here. We don't
		 * update the TTL here to mitigate the damage of a packet
		 * flood and so the same code can handle awkward establishment
		 * and a loosened connection close.
		 * In the establishment case, a correct peer response will
		 * validate the connection, go through the normal state code
		 * and keep updating the state TTL.
		 */

		if (pf_status.debug >= PF_DEBUG_MISC) {
			printf("pf: loose state match: ");
			pf_print_state(*state);
			pf_print_flags(th->th_flags);
			printf(" seq=%lu ack=%lu len=%u ackskew=%d pkts=%d\n",
			    seq, ack, pd->p_len, ackskew, (*state)->packets);
		}

		(*state)->packets++;
		(*state)->bytes += pd->tot_len;

		/* update max window */
		if (src->max_win < win)
			src->max_win = win;
		/* synchronize sequencing */
		if (SEQ_GT(end, src->seqlo))
			src->seqlo = end;
		/* slide the window of what the other end can send */
		if (SEQ_GEQ(ack + win, dst->seqhi))
			dst->seqhi = ack + MAX(win, 1);

		/*
		 * Cannot set dst->seqhi here since this could be a shotgunned
		 * SYN and not an already established connection.
		 */

		if (th->th_flags & TH_FIN)
			if (src->state < TCPS_CLOSING)
				src->state = TCPS_CLOSING;
		if (th->th_flags & TH_RST)
			src->state = dst->state = TCPS_TIME_WAIT;

		/* Fall through to PASS packet */

	} else {
		if (pf_status.debug >= PF_DEBUG_MISC) {
			printf("pf: BAD state: ");
			pf_print_state(*state);
			pf_print_flags(th->th_flags);
			printf(" seq=%lu ack=%lu len=%u ackskew=%d pkts=%d "
			    "dir=%s,%s\n", seq, ack, pd->p_len, ackskew,
			    ++(*state)->packets,
			    direction == PF_IN ? "in" : "out",
			    direction == (*state)->direction ? "fwd" : "rev");
			printf("pf: State failure on: %c %c %c %c | %c %c\n",
			    SEQ_GEQ(src->seqhi, end) ? ' ' : '1',
			    SEQ_GEQ(seq, src->seqlo - dst->max_win) ? ' ': '2',
			    (ackskew >= -MAXACKWINDOW) ? ' ' : '3',
			    (ackskew <= MAXACKWINDOW) ? ' ' : '4',
			    SEQ_GEQ(src->seqhi + MAXACKWINDOW, end) ?' ' :'5',
			    SEQ_GEQ(seq, src->seqlo - MAXACKWINDOW) ?' ' :'6');
		}
		return (PF_DROP);
	}

	/* Any packets which have gotten here are to be passed */

	/* translate source/destination address, if needed */
	if (STATE_TRANSLATE(*state)) {
		if (direction == PF_OUT)
			pf_change_ap(pd->src, &th->th_sport, pd->ip_sum,
			    &th->th_sum, &(*state)->gwy.addr,
			    (*state)->gwy.port, 0, pd->af);
		else
			pf_change_ap(pd->dst, &th->th_dport, pd->ip_sum,
			    &th->th_sum, &(*state)->lan.addr,
			    (*state)->lan.port, 0, pd->af);
		m_copyback(m, off, sizeof(*th), (caddr_t)th);
	} else if (src->seqdiff) {
		/* Copyback sequence modulation */
		m_copyback(m, off, sizeof(*th), (caddr_t)th);
	}

	if ((*state)->rule.ptr != NULL) {
		(*state)->rule.ptr->packets++;
		(*state)->rule.ptr->bytes += pd->tot_len;
	}
	if ((*state)->nat_rule != NULL) {
		(*state)->nat_rule->packets++;
		(*state)->nat_rule->bytes += pd->tot_len;
	}
	return (PF_PASS);
}

int
pf_test_state_udp(struct pf_state **state, int direction, struct ifnet *ifp,
    struct mbuf *m, int ipoff, int off, void *h, struct pf_pdesc *pd)
{
	struct pf_state_peer	*src, *dst;
	struct pf_tree_node	 key;
	struct udphdr		*uh = pd->hdr.udp;

	key.af = pd->af;
	key.proto = IPPROTO_UDP;
	PF_ACPY(&key.addr[0], pd->src, key.af);
	PF_ACPY(&key.addr[1], pd->dst, key.af);
	key.port[0] = pd->hdr.udp->uh_sport;
	key.port[1] = pd->hdr.udp->uh_dport;

	if (direction == PF_IN)
		*state = pf_find_state(&tree_ext_gwy, &key);
	else
		*state = pf_find_state(&tree_lan_ext, &key);
	if (*state == NULL)
		return (PF_DROP);

	if (direction == (*state)->direction) {
		src = &(*state)->src;
		dst = &(*state)->dst;
	} else {
		src = &(*state)->dst;
		dst = &(*state)->src;
	}

	(*state)->packets++;
	(*state)->bytes += pd->tot_len;

	/* update states */
	if (src->state < PFUDPS_SINGLE)
		src->state = PFUDPS_SINGLE;
	if (dst->state == PFUDPS_SINGLE)
		dst->state = PFUDPS_MULTIPLE;

	/* update expire time */
	if (src->state == PFUDPS_MULTIPLE && dst->state == PFUDPS_MULTIPLE)
		(*state)->expire = time.tv_sec +
		    TIMEOUT((*state)->rule.ptr, PFTM_UDP_MULTIPLE);
	else
		(*state)->expire = time.tv_sec +
		    TIMEOUT((*state)->rule.ptr, PFTM_UDP_SINGLE);

	/* translate source/destination address, if necessary */
	if (STATE_TRANSLATE(*state)) {
		if (direction == PF_OUT)
			pf_change_ap(pd->src, &uh->uh_sport, pd->ip_sum,
			    &uh->uh_sum, &(*state)->gwy.addr,
			    (*state)->gwy.port, 1, pd->af);
		else
			pf_change_ap(pd->dst, &uh->uh_dport, pd->ip_sum,
			    &uh->uh_sum, &(*state)->lan.addr,
			    (*state)->lan.port, 1, pd->af);
		m_copyback(m, off, sizeof(*uh), (caddr_t)uh);
	}

	if ((*state)->rule.ptr != NULL) {
		(*state)->rule.ptr->packets++;
		(*state)->rule.ptr->bytes += pd->tot_len;
	}
	if ((*state)->nat_rule != NULL) {
		(*state)->nat_rule->packets++;
		(*state)->nat_rule->bytes += pd->tot_len;
	}
	return (PF_PASS);
}

int
pf_test_state_icmp(struct pf_state **state, int direction, struct ifnet *ifp,
    struct mbuf *m, int ipoff, int off, void *h, struct pf_pdesc *pd)
{
	struct pf_addr	*saddr = pd->src, *daddr = pd->dst;
	u_int16_t	 icmpid, *icmpsum;
	u_int8_t	 icmptype;
	int		 state_icmp = 0;

	switch (pd->proto) {
#ifdef INET
	case IPPROTO_ICMP:
		icmptype = pd->hdr.icmp->icmp_type;
		icmpid = pd->hdr.icmp->icmp_id;
		icmpsum = &pd->hdr.icmp->icmp_cksum;

		if (icmptype == ICMP_UNREACH ||
		    icmptype == ICMP_SOURCEQUENCH ||
		    icmptype == ICMP_REDIRECT ||
		    icmptype == ICMP_TIMXCEED ||
		    icmptype == ICMP_PARAMPROB)
			state_icmp++;
		break;
#endif /* INET */
#ifdef INET6
	case IPPROTO_ICMPV6:
		icmptype = pd->hdr.icmp6->icmp6_type;
		icmpid = pd->hdr.icmp6->icmp6_id;
		icmpsum = &pd->hdr.icmp6->icmp6_cksum;

		if (icmptype == ICMP6_DST_UNREACH ||
		    icmptype == ICMP6_PACKET_TOO_BIG ||
		    icmptype == ICMP6_TIME_EXCEEDED ||
		    icmptype == ICMP6_PARAM_PROB)
			state_icmp++;
		break;
#endif /* INET6 */
	}

	if (!state_icmp) {

		/*
		 * ICMP query/reply message not related to a TCP/UDP packet.
		 * Search for an ICMP state.
		 */
		struct pf_tree_node	key;

		key.af = pd->af;
		key.proto = pd->proto;
		PF_ACPY(&key.addr[0], saddr, key.af);
		PF_ACPY(&key.addr[1], daddr, key.af);
		key.port[0] = icmpid;
		key.port[1] = icmpid;

		if (direction == PF_IN)
			*state = pf_find_state(&tree_ext_gwy, &key);
		else
			*state = pf_find_state(&tree_lan_ext, &key);
		if (*state == NULL)
			return (PF_DROP);

		(*state)->packets++;
		(*state)->bytes += pd->tot_len;
		(*state)->expire = time.tv_sec +
		    TIMEOUT((*state)->rule.ptr, PFTM_ICMP_ERROR_REPLY);

		/* translate source/destination address, if needed */
		if (PF_ANEQ(&(*state)->lan.addr, &(*state)->gwy.addr, pd->af)) {
			if (direction == PF_OUT) {
				switch (pd->af) {
#ifdef INET
				case AF_INET:
					pf_change_a(&saddr->v4.s_addr,
					    pd->ip_sum,
					    (*state)->gwy.addr.v4.s_addr, 0);
					break;
#endif /* INET */
#ifdef INET6
				case AF_INET6:
					pf_change_a6(saddr,
					    &pd->hdr.icmp6->icmp6_cksum,
					    &(*state)->gwy.addr, 0);
					m_copyback(m, off, ICMP_MINLEN,
					    (caddr_t)pd->hdr.icmp6);
					break;
#endif /* INET6 */
				}
			} else {
				switch (pd->af) {
#ifdef INET
				case AF_INET:
					pf_change_a(&daddr->v4.s_addr,
					    pd->ip_sum,
					    (*state)->lan.addr.v4.s_addr, 0);
					break;
#endif /* INET */
#ifdef INET6
				case AF_INET6:
					pf_change_a6(daddr,
					    &pd->hdr.icmp6->icmp6_cksum,
					    &(*state)->lan.addr, 0);
					m_copyback(m, off, ICMP_MINLEN,
					    (caddr_t)pd->hdr.icmp6);
					break;
#endif /* INET6 */
				}
			}
		}

		return (PF_PASS);

	} else {
		/*
		 * ICMP error message in response to a TCP/UDP packet.
		 * Extract the inner TCP/UDP header and search for that state.
		 */

		struct pf_pdesc	pd2;
#ifdef INET
		struct ip	h2;
#endif /* INET */
#ifdef INET6
		struct ip6_hdr	h2_6;
		int		terminal = 0;
#endif /* INET6 */
		int		ipoff2;
		int		off2;

		pd2.af = pd->af;
		switch (pd->af) {
#ifdef INET
		case AF_INET:
			/* offset of h2 in mbuf chain */
			ipoff2 = off + ICMP_MINLEN;

			if (!pf_pull_hdr(m, ipoff2, &h2, sizeof(h2),
			    NULL, NULL, pd2.af)) {
				DPFPRINTF(PF_DEBUG_MISC,
				    ("pf: ICMP error message too short "
				    "(ip)\n"));
				return (PF_DROP);
			}
			/*
			 * ICMP error messages don't refer to non-first
			 * fragments
			 */
			if (ntohs(h2.ip_off) & IP_OFFMASK)
				return (PF_DROP);

			/* offset of protocol header that follows h2 */
			off2 = ipoff2 + (h2.ip_hl << 2);

			pd2.proto = h2.ip_p;
			pd2.src = (struct pf_addr *)&h2.ip_src;
			pd2.dst = (struct pf_addr *)&h2.ip_dst;
			pd2.ip_sum = &h2.ip_sum;
			break;
#endif /* INET */
#ifdef INET6
		case AF_INET6:
			ipoff2 = off + sizeof(struct icmp6_hdr);

			if (!pf_pull_hdr(m, ipoff2, &h2_6, sizeof(h2_6),
			    NULL, NULL, pd2.af)) {
				DPFPRINTF(PF_DEBUG_MISC,
				    ("pf: ICMP error message too short "
				    "(ip6)\n"));
				return (PF_DROP);
			}
			pd2.proto = h2_6.ip6_nxt;
			pd2.src = (struct pf_addr *)&h2_6.ip6_src;
			pd2.dst = (struct pf_addr *)&h2_6.ip6_dst;
			pd2.ip_sum = NULL;
			off2 = ipoff2 + sizeof(h2_6);
			do {
				switch (pd2.proto) {
				case IPPROTO_FRAGMENT:
					/*
					 * ICMPv6 error messages for
					 * non-first fragments
					 */
					return (PF_DROP);
				case IPPROTO_AH:
				case IPPROTO_HOPOPTS:
				case IPPROTO_ROUTING:
				case IPPROTO_DSTOPTS: {
					/* get next header and header length */
					struct ip6_ext opt6;

					if (!pf_pull_hdr(m, off2, &opt6,
					    sizeof(opt6), NULL, NULL, pd2.af)) {
						DPFPRINTF(PF_DEBUG_MISC,
						    ("pf: ICMPv6 short opt\n"));
						return (PF_DROP);
					}
					if (pd2.proto == IPPROTO_AH)
						off2 += (opt6.ip6e_len + 2) * 4;
					else
						off2 += (opt6.ip6e_len + 1) * 8;
					pd2.proto = opt6.ip6e_nxt;
					/* goto the next header */
					break;
				}
				default:
					terminal++;
					break;
				}
			} while (!terminal);
			break;
#endif /* INET6 */
		}

		switch (pd2.proto) {
		case IPPROTO_TCP: {
			struct tcphdr		 th;
			u_int32_t		 seq;
			struct pf_tree_node	 key;
			struct pf_state_peer	*src, *dst;

			/*
			 * Only the first 8 bytes of the TCP header can be
			 * expected. Don't access any TCP header fields after
			 * th_seq, an ackskew test is not possible.
			 */
			if (!pf_pull_hdr(m, off2, &th, 8, NULL, NULL, pd2.af)) {
				DPFPRINTF(PF_DEBUG_MISC,
				    ("pf: ICMP error message too short "
				    "(tcp)\n"));
				return (PF_DROP);
			}

			key.af = pd2.af;
			key.proto = IPPROTO_TCP;
			PF_ACPY(&key.addr[0], pd2.dst, pd2.af);
			key.port[0] = th.th_dport;
			PF_ACPY(&key.addr[1], pd2.src, pd2.af);
			key.port[1] = th.th_sport;

			if (direction == PF_IN)
				*state = pf_find_state(&tree_ext_gwy, &key);
			else
				*state = pf_find_state(&tree_lan_ext, &key);
			if (*state == NULL)
				return (PF_DROP);

			if (direction == (*state)->direction) {
				src = &(*state)->dst;
				dst = &(*state)->src;
			} else {
				src = &(*state)->src;
				dst = &(*state)->dst;
			}

			/* Demodulate sequence number */
			seq = ntohl(th.th_seq) - src->seqdiff;
			if (src->seqdiff)
				pf_change_a(&th.th_seq, &th.th_sum,
				    htonl(seq), 0);

			if (!SEQ_GEQ(src->seqhi, seq) ||
			    !SEQ_GEQ(seq, src->seqlo - dst->max_win)) {
				if (pf_status.debug >= PF_DEBUG_MISC) {
					printf("pf: BAD ICMP state: ");
					pf_print_state(*state);
					printf(" seq=%lu\n", seq);
				}
				return (PF_DROP);
			}

			if (STATE_TRANSLATE(*state)) {
				if (direction == PF_IN) {
					pf_change_icmp(pd2.src, &th.th_sport,
					    saddr, &(*state)->lan.addr,
					    (*state)->lan.port, NULL,
					    pd2.ip_sum, icmpsum,
					    pd->ip_sum, 0, pd2.af);
				} else {
					pf_change_icmp(pd2.dst, &th.th_dport,
					    saddr, &(*state)->gwy.addr,
					    (*state)->gwy.port, NULL,
					    pd2.ip_sum, icmpsum,
					    pd->ip_sum, 0, pd2.af);
				}
				switch (pd2.af) {
#ifdef INET
				case AF_INET:
					m_copyback(m, off, ICMP_MINLEN,
					    (caddr_t)pd->hdr.icmp);
					m_copyback(m, ipoff2, sizeof(h2),
					    (caddr_t)&h2);
					break;
#endif /* INET */
#ifdef INET6
				case AF_INET6:
					m_copyback(m, off, ICMP_MINLEN,
					    (caddr_t)pd->hdr.icmp6);
					m_copyback(m, ipoff2, sizeof(h2_6),
					    (caddr_t)&h2_6);
					break;
#endif /* INET6 */
				}
				m_copyback(m, off2, 8, (caddr_t)&th);
			} else if (src->seqdiff) {
				m_copyback(m, off2, 8, (caddr_t)&th);
			}

			return (PF_PASS);
			break;
		}
		case IPPROTO_UDP: {
			struct udphdr		uh;
			struct pf_tree_node	key;

			if (!pf_pull_hdr(m, off2, &uh, sizeof(uh),
			    NULL, NULL, pd2.af)) {
				DPFPRINTF(PF_DEBUG_MISC,
				    ("pf: ICMP error message too short "
				    "(udp)\n"));
				return (PF_DROP);
			}

			key.af = pd2.af;
			key.proto = IPPROTO_UDP;
			PF_ACPY(&key.addr[0], pd2.dst, pd2.af);
			key.port[0] = uh.uh_dport;
			PF_ACPY(&key.addr[1], pd2.src, pd2.af);
			key.port[1] = uh.uh_sport;

			if (direction == PF_IN)
				*state = pf_find_state(&tree_ext_gwy, &key);
			else
				*state = pf_find_state(&tree_lan_ext, &key);
			if (*state == NULL)
				return (PF_DROP);

			if (STATE_TRANSLATE(*state)) {
				if (direction == PF_IN) {
					pf_change_icmp(pd2.src, &uh.uh_sport,
					    daddr, &(*state)->lan.addr,
					    (*state)->lan.port, &uh.uh_sum,
					    pd2.ip_sum, icmpsum,
					    pd->ip_sum, 1, pd2.af);
				} else {
					pf_change_icmp(pd2.dst, &uh.uh_dport,
					    saddr, &(*state)->gwy.addr,
					    (*state)->gwy.port, &uh.uh_sum,
					    pd2.ip_sum, icmpsum,
					    pd->ip_sum, 1, pd2.af);
				}
				switch (pd2.af) {
#ifdef INET
				case AF_INET:
					m_copyback(m, off, ICMP_MINLEN,
					    (caddr_t)pd->hdr.icmp);
					m_copyback(m, ipoff2, sizeof(h2),
					    (caddr_t)&h2);
					break;
#endif /* INET */
#ifdef INET6
				case AF_INET6:
					m_copyback(m, off, ICMP_MINLEN,
					    (caddr_t)pd->hdr.icmp6);
					m_copyback(m, ipoff2, sizeof(h2_6),
					    (caddr_t)&h2_6);
					break;
#endif /* INET6 */
				}
				m_copyback(m, off2, sizeof(uh),
				    (caddr_t)&uh);
			}

			return (PF_PASS);
			break;
		}
#ifdef INET
		case IPPROTO_ICMP: {
			struct icmp		iih;
			struct pf_tree_node	key;

			if (!pf_pull_hdr(m, off2, &iih, ICMP_MINLEN,
			    NULL, NULL, pd2.af)) {
				DPFPRINTF(PF_DEBUG_MISC,
				    ("pf: ICMP error message too short i"
				    "(icmp)\n"));
				return (PF_DROP);
			}

			key.af = pd2.af;
			key.proto = IPPROTO_ICMP;
			PF_ACPY(&key.addr[0], pd2.dst, pd2.af);
			key.port[0] = iih.icmp_id;
			PF_ACPY(&key.addr[1], pd2.src, pd2.af);
			key.port[1] = iih.icmp_id;

			if (direction == PF_IN)
				*state = pf_find_state(&tree_ext_gwy, &key);
			else
				*state = pf_find_state(&tree_lan_ext, &key);
			if (*state == NULL)
				return (PF_DROP);

			if (STATE_TRANSLATE(*state)) {
				if (direction == PF_IN) {
					pf_change_icmp(pd2.src, &iih.icmp_id,
					    daddr, &(*state)->lan.addr,
					    (*state)->lan.port, NULL,
					    pd2.ip_sum, icmpsum,
					    pd->ip_sum, 0, AF_INET);
				} else {
					pf_change_icmp(pd2.dst, &iih.icmp_id,
					    saddr, &(*state)->gwy.addr,
					    (*state)->gwy.port, NULL,
					    pd2.ip_sum, icmpsum,
					    pd->ip_sum, 0, AF_INET);
				}
				m_copyback(m, off, ICMP_MINLEN,
				    (caddr_t)pd->hdr.icmp);
				m_copyback(m, ipoff2, sizeof(h2),
				    (caddr_t)&h2);
				m_copyback(m, off2, ICMP_MINLEN,
				    (caddr_t)&iih);
			}

			return (PF_PASS);
			break;
		}
#endif /* INET */
#ifdef INET6
		case IPPROTO_ICMPV6: {
			struct icmp6_hdr	iih;
			struct pf_tree_node	key;

			if (!pf_pull_hdr(m, off2, &iih, ICMP_MINLEN,
			    NULL, NULL, pd2.af)) {
				DPFPRINTF(PF_DEBUG_MISC,
				    ("pf: ICMP error message too short "
				    "(icmp6)\n"));
				return (PF_DROP);
			}

			key.af = pd2.af;
			key.proto = IPPROTO_ICMPV6;
			PF_ACPY(&key.addr[0], pd2.dst, pd2.af);
			key.port[0] = iih.icmp6_id;
			PF_ACPY(&key.addr[1], pd2.src, pd2.af);
			key.port[1] = iih.icmp6_id;

			if (direction == PF_IN)
				*state = pf_find_state(&tree_ext_gwy, &key);
			else
				*state = pf_find_state(&tree_lan_ext, &key);
			if (*state == NULL)
				return (PF_DROP);

			if (STATE_TRANSLATE(*state)) {
				if (direction == PF_IN) {
					pf_change_icmp(pd2.src, &iih.icmp6_id,
					    daddr, &(*state)->lan.addr,
					    (*state)->lan.port, NULL,
					    pd2.ip_sum, icmpsum,
					    pd->ip_sum, 0, AF_INET6);
				} else {
					pf_change_icmp(pd2.dst, &iih.icmp6_id,
					    saddr, &(*state)->gwy.addr,
					    (*state)->gwy.port, NULL,
					    pd2.ip_sum, icmpsum,
					    pd->ip_sum, 0, AF_INET6);
				}
				m_copyback(m, off, ICMP_MINLEN,
				    (caddr_t)pd->hdr.icmp6);
				m_copyback(m, ipoff2, sizeof(h2_6),
				    (caddr_t)&h2_6);
				m_copyback(m, off2, ICMP_MINLEN,
				    (caddr_t)&iih);
			}

			return (PF_PASS);
			break;
		}
#endif /* INET6 */
		default:
			DPFPRINTF(PF_DEBUG_MISC,
			    ("pf: ICMP error message for bad proto\n"));
			return (PF_DROP);
		}

	}
}

int
pf_test_state_other(struct pf_state **state, int direction, struct ifnet *ifp,
    struct pf_pdesc *pd)
{
	struct pf_state_peer	*src, *dst;
	struct pf_tree_node	 key;

	key.af = pd->af;
	key.proto = pd->proto;
	PF_ACPY(&key.addr[0], pd->src, key.af);
	PF_ACPY(&key.addr[1], pd->dst, key.af);
	key.port[0] = 0;
	key.port[1] = 0;

	if (direction == PF_IN)
		*state = pf_find_state(&tree_ext_gwy, &key);
	else
		*state = pf_find_state(&tree_lan_ext, &key);
	if (*state == NULL)
		return (PF_DROP);

	if (direction == (*state)->direction) {
		src = &(*state)->src;
		dst = &(*state)->dst;
	} else {
		src = &(*state)->dst;
		dst = &(*state)->src;
	}

	(*state)->packets++;
	(*state)->bytes += pd->tot_len;

	/* update states */
	if (src->state < PFOTHERS_SINGLE)
		src->state = PFOTHERS_SINGLE;
	if (dst->state == PFOTHERS_SINGLE)
		dst->state = PFOTHERS_MULTIPLE;

	/* update expire time */
	if (src->state == PFOTHERS_MULTIPLE && dst->state == PFOTHERS_MULTIPLE)
		(*state)->expire = time.tv_sec +
		    TIMEOUT((*state)->rule.ptr, PFTM_OTHER_MULTIPLE);
	else
		(*state)->expire = time.tv_sec +
		    TIMEOUT((*state)->rule.ptr, PFTM_OTHER_SINGLE);

	/* translate source/destination address, if necessary */
	if (STATE_TRANSLATE(*state)) {
		if (direction == PF_OUT)
			switch (pd->af) {
#ifdef INET
			case AF_INET:
				pf_change_a(&pd->src->v4.s_addr,
				    pd->ip_sum, (*state)->gwy.addr.v4.s_addr,
				    0);
				break;
#endif /* INET */
#ifdef INET6
			case AF_INET6:
				PF_ACPY(pd->src, &(*state)->gwy.addr, pd->af);
				break;
#endif /* INET6 */
			}
		else
			switch (pd->af) {
#ifdef INET
			case AF_INET:
				pf_change_a(&pd->dst->v4.s_addr,
				    pd->ip_sum, (*state)->lan.addr.v4.s_addr,
				    0);
				break;
#endif /* INET */
#ifdef INET6
			case AF_INET6:
				PF_ACPY(pd->dst, &(*state)->lan.addr, pd->af);
				break;
#endif /* INET6 */
			}
	}

	if ((*state)->rule.ptr != NULL) {
		(*state)->rule.ptr->packets++;
		(*state)->rule.ptr->bytes += pd->tot_len;
	}
	if ((*state)->nat_rule != NULL) {
		(*state)->nat_rule->packets++;
		(*state)->nat_rule->bytes += pd->tot_len;
	}
	return (PF_PASS);
}

/*
 * ipoff and off are measured from the start of the mbuf chain.
 * h must be at "ipoff" on the mbuf chain.
 */
void *
pf_pull_hdr(struct mbuf *m, int off, void *p, int len,
    u_short *actionp, u_short *reasonp, sa_family_t af)
{
	switch (af) {
#ifdef INET
	case AF_INET: {
		struct ip	*h = mtod(m, struct ip *);
		u_int16_t	 fragoff = (h->ip_off & IP_OFFMASK) << 3;

		if (fragoff) {
			if (fragoff >= len)
				ACTION_SET(actionp, PF_PASS);
			else {
				ACTION_SET(actionp, PF_DROP);
				REASON_SET(reasonp, PFRES_FRAG);
			}
			return (NULL);
		}
		if (m->m_pkthdr.len < off + len || h->ip_len < off + len) {
			ACTION_SET(actionp, PF_DROP);
			REASON_SET(reasonp, PFRES_SHORT);
			return (NULL);
		}
		break;
	}
#endif /* INET */
#ifdef INET6
	case AF_INET6: {
		struct ip6_hdr	*h = mtod(m, struct ip6_hdr *);

		if (m->m_pkthdr.len < off + len ||
		    (ntohs(h->ip6_plen) + sizeof(struct ip6_hdr)) <
		    (unsigned)(off + len)) {
			ACTION_SET(actionp, PF_DROP);
			REASON_SET(reasonp, PFRES_SHORT);
			return (NULL);
		}
		break;
	}
#endif /* INET6 */
	}
	m_copydata(m, off, len, p);
	return (p);
}

int
pf_routable(struct pf_addr *addr, sa_family_t af)
{
	struct sockaddr_in	*dst;
	struct route		 ro;
	int			 ret = 0;

	bzero(&ro, sizeof(ro));
	dst = satosin(&ro.ro_dst);
	dst->sin_family = af;
	dst->sin_len = sizeof(*dst);
	dst->sin_addr = addr->v4;
	rtalloc_noclone(&ro, NO_CLONING);

	if (ro.ro_rt != NULL) {
		ret = 1;
		RTFREE(ro.ro_rt);
	}

	return (ret);
}

#ifdef INET
void
pf_route(struct mbuf **m, struct pf_rule *r, int dir, struct ifnet *oifp,
    struct pf_state *s)
{
	struct mbuf		*m0, *m1;
	struct route		 iproute;
	struct route		*ro;
	struct sockaddr_in	*dst;
	struct ip		*ip;
	struct ifnet		*ifp = NULL;
	struct m_tag		*mtag;
	struct pf_addr		 naddr;
	int			 error = 0;

	if (m == NULL || *m == NULL || r == NULL ||
	    (dir != PF_IN && dir != PF_OUT) || oifp == NULL)
		panic("pf_route: invalid parameters");

	if (r->rt == PF_DUPTO) {
		m0 = m_copym2(*m, 0, M_COPYALL, M_NOWAIT);
		if (m0 == NULL)
			return;
	} else {
		if ((r->rt == PF_REPLYTO) == (r->direction == dir))
			return;
		m0 = *m;
	}

	if (m0->m_len < sizeof(struct ip))
		panic("pf_route: m0->m_len < sizeof(struct ip)");
	ip = mtod(m0, struct ip *);

	ro = &iproute;
	bzero((caddr_t)ro, sizeof(*ro));
	dst = satosin(&ro->ro_dst);
	dst->sin_family = AF_INET;
	dst->sin_len = sizeof(*dst);
	dst->sin_addr = ip->ip_dst;

	if (r->rt == PF_FASTROUTE) {
		rtalloc(ro);
		if (ro->ro_rt == 0) {
			ipstat.ips_noroute++;
			goto bad;
		}

		ifp = ro->ro_rt->rt_ifp;
		ro->ro_rt->rt_use++;

		if (ro->ro_rt->rt_flags & RTF_GATEWAY)
			dst = satosin(ro->ro_rt->rt_gateway);
	} else {
		if (TAILQ_EMPTY(&r->rpool.list))
			panic("pf_route: TAILQ_EMPTY(&r->rpool.list)");
		if (s == NULL) {
			pf_map_addr(AF_INET, &r->rpool,
			    (struct pf_addr *)&ip->ip_src,
			    &naddr, NULL);
			if (!PF_AZERO(&naddr, AF_INET))
				dst->sin_addr.s_addr = naddr.v4.s_addr;
			ifp = r->rpool.cur->ifp;
		} else {
			if (s->rt_ifp == NULL) {
				pf_map_addr(AF_INET, &r->rpool,
				    (struct pf_addr *)&ip->ip_src,
				    &s->rt_addr, NULL);
				s->rt_ifp = r->rpool.cur->ifp;
			}
			if (!PF_AZERO(&s->rt_addr, AF_INET))
				dst->sin_addr.s_addr =
				    s->rt_addr.v4.s_addr;
			ifp = s->rt_ifp;
		}
	}

	if (ifp == NULL)
		goto bad;

	if (oifp != ifp) {
		mtag = m_tag_find(m0, PACKET_TAG_PF_ROUTED, NULL);
		if (mtag == NULL) {
			if (pf_test(PF_OUT, ifp, &m0) != PF_PASS)
				goto bad;
			else if (m0 == NULL)
				goto done;
			mtag = m_tag_get(PACKET_TAG_PF_ROUTED, 0, M_NOWAIT);
			if (mtag == NULL)
				goto bad;
			m_tag_prepend(m0, mtag);
			if (m0->m_len < sizeof(struct ip))
				panic("pf_route: m0->m_len < sizeof(struct ip)");
			ip = mtod(m0, struct ip *);
		}
	}

	/* Copied from ip_output. */
	if (ip->ip_len <= ifp->if_mtu) {
		ip->ip_len = htons((u_int16_t)ip->ip_len);
		ip->ip_off = htons((u_int16_t)ip->ip_off);
		if ((ifp->if_capabilities & IFCAP_CSUM_IPv4) &&
		    ifp->if_bridge == NULL) {
			m0->m_pkthdr.csum |= M_IPV4_CSUM_OUT;
			ipstat.ips_outhwcsum++;
		} else {
			ip->ip_sum = 0;
			ip->ip_sum = in_cksum(m0, ip->ip_hl << 2);
		}
		/* Update relevant hardware checksum stats for TCP/UDP */
		if (m0->m_pkthdr.csum & M_TCPV4_CSUM_OUT)
			tcpstat.tcps_outhwcsum++;
		else if (m0->m_pkthdr.csum & M_UDPV4_CSUM_OUT)
			udpstat.udps_outhwcsum++;
		error = (*ifp->if_output)(ifp, m0, sintosa(dst), NULL);
		goto done;
	}

	/*
	 * Too large for interface; fragment if possible.
	 * Must be able to put at least 8 bytes per fragment.
	 */
	if (ip->ip_off & IP_DF) {
		ipstat.ips_cantfrag++;
		if (r->rt != PF_DUPTO) {
			icmp_error(m0, ICMP_UNREACH, ICMP_UNREACH_NEEDFRAG, 0,
			    ifp);
			goto done;
		} else
			goto bad;
	}

	m1 = m0;
	error = ip_fragment(m0, ifp, ifp->if_mtu);
	if (error == EMSGSIZE)
		goto bad;

	for (m0 = m1; m0; m0 = m1) {
		m1 = m0->m_nextpkt;
		m0->m_nextpkt = 0;
		if (error == 0)
			error = (*ifp->if_output)(ifp, m0, sintosa(dst),
			    NULL);
		else
			m_freem(m0);
	}

	if (error == 0)
		ipstat.ips_fragmented++;

done:
	if (r->rt != PF_DUPTO)
		*m = NULL;
	if (ro == &iproute && ro->ro_rt)
		RTFREE(ro->ro_rt);
	return;

bad:
	m_freem(m0);
	goto done;
}
#endif /* INET */

#ifdef INET6
void
pf_route6(struct mbuf **m, struct pf_rule *r, int dir, struct ifnet *oifp,
    struct pf_state *s)
{
	struct mbuf		*m0;
	struct m_tag		*mtag;
	struct route_in6	 ip6route;
	struct route_in6	*ro;
	struct sockaddr_in6	*dst;
	struct ip6_hdr		*ip6;
	struct ifnet		*ifp = NULL;
	struct pf_addr		 naddr;
	int			 error = 0;

	if (m == NULL || *m == NULL || r == NULL ||
	    (dir != PF_IN && dir != PF_OUT) || oifp == NULL)
		panic("pf_route6: invalid parameters");

	if (r->rt == PF_DUPTO) {
		m0 = m_copym2(*m, 0, M_COPYALL, M_NOWAIT);
		if (m0 == NULL)
			return;
	} else {
		if ((r->rt == PF_REPLYTO) == (r->direction == dir))
			return;
		m0 = *m;
	}

	if (m0->m_len < sizeof(struct ip6_hdr))
		panic("pf_route6: m0->m_len < sizeof(struct ip6_hdr)");
	ip6 = mtod(m0, struct ip6_hdr *);

	ro = &ip6route;
	bzero((caddr_t)ro, sizeof(*ro));
	dst = (struct sockaddr_in6 *)&ro->ro_dst;
	dst->sin6_family = AF_INET6;
	dst->sin6_len = sizeof(*dst);
	dst->sin6_addr = ip6->ip6_dst;

	/* Cheat. */
	if (r->rt == PF_FASTROUTE) {
		mtag = m_tag_get(PACKET_TAG_PF_GENERATED, 0, M_NOWAIT);
		if (mtag == NULL)
			goto bad;
		m_tag_prepend(m0, mtag);
		ip6_output(m0, NULL, NULL, NULL, NULL, NULL);
		return;
	}

	if (TAILQ_EMPTY(&r->rpool.list))
		panic("pf_route6: TAILQ_EMPTY(&r->rpool.list)");
	if (s == NULL) {
		pf_map_addr(AF_INET6, &r->rpool,
		    (struct pf_addr *)&ip6->ip6_src, &naddr, NULL);
		if (!PF_AZERO(&naddr, AF_INET6))
			PF_ACPY((struct pf_addr *)&dst->sin6_addr,
			    &naddr, AF_INET6);
		ifp = r->rpool.cur->ifp;
	} else {
		if (s->rt_ifp == NULL) {
			pf_map_addr(AF_INET6, &r->rpool,
			    (struct pf_addr *)&ip6->ip6_src,
			    &s->rt_addr, NULL);
			s->rt_ifp = r->rpool.cur->ifp;
		}
		if (!PF_AZERO(&s->rt_addr, AF_INET6))
			PF_ACPY((struct pf_addr *)&dst->sin6_addr,
			    &s->rt_addr, AF_INET6);
		ifp = s->rt_ifp;
	}

	if (ifp == NULL)
		goto bad;

	if (oifp != ifp) {
		mtag = m_tag_find(m0, PACKET_TAG_PF_ROUTED, NULL);
		if (mtag == NULL) {
			if (pf_test(PF_OUT, ifp, &m0) != PF_PASS)
				goto bad;
			else if (m0 == NULL)
				goto done;
			mtag = m_tag_get(PACKET_TAG_PF_ROUTED, 0, M_NOWAIT);
			if (mtag == NULL)
				goto bad;
			m_tag_prepend(m0, mtag);
		}
	}

	/*
	 * If the packet is too large for the outgoing interface,
	 * send back an icmp6 error.
	 */
	if ((u_long)m0->m_pkthdr.len <= ifp->if_mtu) {
		error = (*ifp->if_output)(ifp, m0, (struct sockaddr *)dst,
		    NULL);
	} else {
		in6_ifstat_inc(ifp, ifs6_in_toobig);
		if (r->rt != PF_DUPTO)
			icmp6_error(m0, ICMP6_PACKET_TOO_BIG, 0, ifp->if_mtu);
		else
			goto bad;
	}

done:
	if (r->rt != PF_DUPTO)
		*m = NULL;
	return;

bad:
	m_freem(m0);
	goto done;
}
#endif /* INET6 */

#ifdef INET
int
pf_test(int dir, struct ifnet *ifp, struct mbuf **m0)
{
	u_short		 action, reason = 0, log = 0;
	struct mbuf	*m = *m0;
	struct ip	*h;
	struct pf_rule	*r = NULL;
	struct pf_state	*s = NULL;
	struct pf_pdesc	 pd;
	int		 off;

	if (!pf_status.running ||
	    (m_tag_find(m, PACKET_TAG_PF_GENERATED, NULL) != NULL))
		return (PF_PASS);

#ifdef DIAGNOSTIC
	if ((m->m_flags & M_PKTHDR) == 0)
		panic("non-M_PKTHDR is passed to pf_test");
#endif

	if (m->m_pkthdr.len < (int)sizeof(*h)) {
		action = PF_DROP;
		REASON_SET(&reason, PFRES_SHORT);
		log = 1;
		goto done;
	}

	/* We do IP header normalization and packet reassembly here */
	if (pf_normalize_ip(m0, dir, ifp, &reason) != PF_PASS) {
		ACTION_SET(&action, PF_DROP);
		goto done;
	}
	m = *m0;
	h = mtod(m, struct ip *);

	off = h->ip_hl << 2;
	if (off < (int)sizeof(*h)) {
		action = PF_DROP;
		REASON_SET(&reason, PFRES_SHORT);
		log = 1;
		goto done;
	}

	pd.src = (struct pf_addr *)&h->ip_src;
	pd.dst = (struct pf_addr *)&h->ip_dst;
	pd.ip_sum = &h->ip_sum;
	pd.proto = h->ip_p;
	pd.af = AF_INET;
	pd.tos = h->ip_tos;
	pd.tot_len = h->ip_len;

	/* handle fragments that didn't get reassembled by normalization */
	if (h->ip_off & (IP_MF | IP_OFFMASK)) {
		action = pf_test_fragment(&r, dir, ifp, m, h, &pd);
		goto done;
	}

	switch (h->ip_p) {

	case IPPROTO_TCP: {
		struct tcphdr	th;

		pd.hdr.tcp = &th;
		if (!pf_pull_hdr(m, off, &th, sizeof(th),
		    &action, &reason, AF_INET)) {
			log = action != PF_PASS;
			goto done;
		}
		pd.p_len = pd.tot_len - off - (th.th_off << 2);
		action = pf_normalize_tcp(dir, ifp, m, 0, off, h, &pd);
		if (action == PF_DROP)
			break;
		action = pf_test_state_tcp(&s, dir, ifp, m, 0, off, h, &pd);
		if (action == PF_PASS) {
			r = s->rule.ptr;
			log = s->log;
		} else if (s == NULL)
			action = pf_test_tcp(&r, dir, ifp, m, 0, off, h, &pd);
		break;
	}

	case IPPROTO_UDP: {
		struct udphdr	uh;

		pd.hdr.udp = &uh;
		if (!pf_pull_hdr(m, off, &uh, sizeof(uh),
		    &action, &reason, AF_INET)) {
			log = action != PF_PASS;
			goto done;
		}
		action = pf_test_state_udp(&s, dir, ifp, m, 0, off, h, &pd);
		if (action == PF_PASS) {
			r = s->rule.ptr;
			log = s->log;
		} else if (s == NULL)
			action = pf_test_udp(&r, dir, ifp, m, 0, off, h, &pd);
		break;
	}

	case IPPROTO_ICMP: {
		struct icmp	ih;

		pd.hdr.icmp = &ih;
		if (!pf_pull_hdr(m, off, &ih, ICMP_MINLEN,
		    &action, &reason, AF_INET)) {
			log = action != PF_PASS;
			goto done;
		}
		action = pf_test_state_icmp(&s, dir, ifp, m, 0, off, h, &pd);
		if (action == PF_PASS) {
			r = s->rule.ptr;
			if (r != NULL) {
				r->packets++;
				r->bytes += h->ip_len;
			}
			log = s->log;
		} else if (s == NULL)
			action = pf_test_icmp(&r, dir, ifp, m, 0, off, h, &pd);
		break;
	}

	default:
		action = pf_test_state_other(&s, dir, ifp, &pd);
		if (action == PF_PASS) {
			r = s->rule.ptr;
			log = s->log;
		} else if (s == NULL)
			action = pf_test_other(&r, dir, ifp, m, h, &pd);
		break;
	}

	if (ifp == status_ifp) {
		pf_status.bcounters[0][dir == PF_OUT] += pd.tot_len;
		pf_status.pcounters[0][dir == PF_OUT][action]++;
	}

done:
	if (r != NULL &&  r->src.addr.mask.addr32[0] == PF_TABLE_MASK)
		pfr_update_stats(&r->src.addr.addr, &r->src.addr.mask,
		    (r->direction == dir) ? pd.src : pd.dst,
		    pd.af, pd.tot_len, dir == PF_OUT,
		    r->action == PF_PASS, r->src.not);
	if (r != NULL && r->dst.addr.mask.addr32[0] == PF_TABLE_MASK)
		pfr_update_stats(&r->dst.addr.addr, &r->dst.addr.mask,
		    (r->direction == dir) ? pd.dst : pd.src,
		    pd.af, pd.tot_len, dir == PF_OUT,
		    r->action == PF_PASS, r->dst.not);

	if (action != PF_DROP && h->ip_hl > 5 &&
	    !((s && s->allow_opts) || (r && r->allow_opts))) {
		action = PF_DROP;
		REASON_SET(&reason, PFRES_SHORT);
		log = 1;
		DPFPRINTF(PF_DEBUG_MISC,
		    ("pf: dropping packet with ip options\n"));
	}

#ifdef ALTQ
	if (action != PF_DROP && r != NULL && r->qid) {
		struct m_tag	*mtag;
		struct altq_tag	*atag;

		mtag = m_tag_get(PACKET_TAG_PF_QID, sizeof(*atag), M_NOWAIT);
		if (mtag != NULL) {
			atag = (struct altq_tag *)(mtag + 1);
			if (pd.tos == IPTOS_LOWDELAY)
				atag->qid = r->pqid;
			else
				atag->qid = r->qid;
			/* add hints for ecn */
			atag->af = AF_INET;
			atag->hdr = h;
			m_tag_prepend(m, mtag);
		}
	}
#endif

	if (log) {
		if (r == NULL) {
			struct pf_rule	r0;

			r0.ifp = ifp;
			r0.action = action;
			r0.nr = -1;
			PFLOG_PACKET(ifp, h, m, AF_INET, dir, reason, &r0);
		} else
			PFLOG_PACKET(ifp, h, m, AF_INET, dir, reason, r);
	}

	/* pf_route can free the mbuf causing *m0 to become NULL */
	if (r != NULL && r->rt)
		pf_route(m0, r, dir, ifp, s);

	return (action);
}
#endif /* INET */

#ifdef INET6
int
pf_test6(int dir, struct ifnet *ifp, struct mbuf **m0)
{
	u_short		 action, reason = 0, log = 0;
	struct mbuf	*m = *m0;
	struct ip6_hdr	*h;
	struct pf_rule	*r = NULL;
	struct pf_state	*s = NULL;
	struct pf_pdesc pd;
	int		off, terminal = 0;

	if (!pf_status.running ||
	    (m_tag_find(m, PACKET_TAG_PF_GENERATED, NULL) != NULL))
		return (PF_PASS);

#ifdef DIAGNOSTIC
	if ((m->m_flags & M_PKTHDR) == 0)
		panic("non-M_PKTHDR is passed to pf_test");
#endif

	if (m->m_pkthdr.len < (int)sizeof(*h)) {
		action = PF_DROP;
		REASON_SET(&reason, PFRES_SHORT);
		log = 1;
		goto done;
	}

	m = *m0;
	h = mtod(m, struct ip6_hdr *);

	pd.src = (struct pf_addr *)&h->ip6_src;
	pd.dst = (struct pf_addr *)&h->ip6_dst;
	pd.ip_sum = NULL;
	pd.af = AF_INET6;
	pd.tos = 0;
	pd.tot_len = ntohs(h->ip6_plen) + sizeof(struct ip6_hdr);

	off = ((caddr_t)h - m->m_data) + sizeof(struct ip6_hdr);
	pd.proto = h->ip6_nxt;
	do {
		switch (pd.proto) {
		case IPPROTO_FRAGMENT:
			action = pf_test_fragment(&r, dir, ifp, m, h, &pd);
			if (action == PF_DROP)
				REASON_SET(&reason, PFRES_FRAG);
			goto done;
		case IPPROTO_AH:
		case IPPROTO_HOPOPTS:
		case IPPROTO_ROUTING:
		case IPPROTO_DSTOPTS: {
			/* get next header and header length */
			struct ip6_ext	opt6;

			if (!pf_pull_hdr(m, off, &opt6, sizeof(opt6),
			    NULL, NULL, pd.af)) {
				DPFPRINTF(PF_DEBUG_MISC,
				    ("pf: IPv6 short opt\n"));
				action = PF_DROP;
				REASON_SET(&reason, PFRES_SHORT);
				log = 1;
				goto done;
			}
			if (pd.proto == IPPROTO_AH)
				off += (opt6.ip6e_len + 2) * 4;
			else
				off += (opt6.ip6e_len + 1) * 8;
			pd.proto = opt6.ip6e_nxt;
			/* goto the next header */
			break;
		}
		default:
			terminal++;
			break;
		}
	} while (!terminal);

	switch (pd.proto) {

	case IPPROTO_TCP: {
		struct tcphdr	th;

		pd.hdr.tcp = &th;
		if (!pf_pull_hdr(m, off, &th, sizeof(th),
		    &action, &reason, AF_INET6)) {
			log = action != PF_PASS;
			goto done;
		}
		pd.p_len = pd.tot_len - off - (th.th_off << 2);
		action = pf_normalize_tcp(dir, ifp, m, 0, off, h, &pd);
		if (action == PF_DROP)
			break;
		action = pf_test_state_tcp(&s, dir, ifp, m, 0, off, h, &pd);
		if (action == PF_PASS) {
			r = s->rule.ptr;
			log = s->log;
		} else if (s == NULL)
			action = pf_test_tcp(&r, dir, ifp, m, 0, off, h, &pd);
		break;
	}

	case IPPROTO_UDP: {
		struct udphdr	uh;

		pd.hdr.udp = &uh;
		if (!pf_pull_hdr(m, off, &uh, sizeof(uh),
		    &action, &reason, AF_INET6)) {
			log = action != PF_PASS;
			goto done;
		}
		action = pf_test_state_udp(&s, dir, ifp, m, 0, off, h, &pd);
		if (action == PF_PASS) {
			r = s->rule.ptr;
			log = s->log;
		} else if (s == NULL)
			action = pf_test_udp(&r, dir, ifp, m, 0, off, h, &pd);
		break;
	}

	case IPPROTO_ICMPV6: {
		struct icmp6_hdr	ih;

		pd.hdr.icmp6 = &ih;
		if (!pf_pull_hdr(m, off, &ih, sizeof(ih),
		    &action, &reason, AF_INET6)) {
			log = action != PF_PASS;
			goto done;
		}
		action = pf_test_state_icmp(&s, dir, ifp, m, 0, off, h, &pd);
		if (action == PF_PASS) {
			r = s->rule.ptr;
			if (r != NULL) {
				r->packets++;
				r->bytes += h->ip6_plen;
			}
			log = s->log;
		} else if (s == NULL)
			action = pf_test_icmp(&r, dir, ifp, m, 0, off, h, &pd);
		break;
	}

	default:
		action = pf_test_other(&r, dir, ifp, m, h, &pd);
		break;
	}

	if (ifp == status_ifp) {
		pf_status.bcounters[1][dir == PF_OUT] += pd.tot_len;
		pf_status.pcounters[1][dir == PF_OUT][action]++;
	}

done:
	/* XXX handle IPv6 options, if not allowed. not implemented. */

	if (log) {
		if (r == NULL) {
			struct pf_rule	r0;

			r0.ifp = ifp;
			r0.action = action;
			r0.nr = -1;
			PFLOG_PACKET(ifp, h, m, AF_INET6, dir, reason, &r0);
		} else
			PFLOG_PACKET(ifp, h, m, AF_INET6, dir, reason, r);
	}

	/* pf_route6 can free the mbuf causing *m0 to become NULL */
	if (r != NULL && r->rt)
		pf_route6(m0, r, dir, ifp, s);

	return (action);
}
#endif /* INET6 */
