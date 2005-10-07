/*	$OpenBSD: hostapd.h,v 1.8 2005/10/07 22:32:52 reyk Exp $	*/

/*
 * Copyright (c) 2004, 2005 Reyk Floeter <reyk@vantronix.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _HOSTAPD_H
#define _HOSTAPD_H

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/tree.h>

#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <event.h>
#include <syslog.h>

#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>

/*
 * hostapd (IAPP) <-> Host AP (APME)
 */

struct hostapd_node {
	u_int8_t	ni_macaddr[IEEE80211_ADDR_LEN];
	u_int8_t	ni_bssid[IEEE80211_ADDR_LEN];
	u_int32_t	ni_associd;
	u_int16_t	ni_capinfo;
	u_int16_t	ni_flags;
	u_int16_t	ni_rxseq;
	u_int16_t	ni_rssi;
};

/*
 * IAPP -> switches (LLC)
 */

struct hostapd_llc {
	struct ether_header x_hdr;
	struct llc x_llc;
} __packed;

#define IAPP_LLC	LLC_XID
#define IAPP_LLC_XID	0x81
#define IAPP_LLC_CLASS	1
#define IAPP_LLC_WINDOW	1 << 1

/*
 * hostapd configuration
 */

struct hostapd_counter {
	u_int64_t	cn_tx_llc;	/* sent LLC messages */
	u_int64_t	cn_rx_iapp;	/* received IAPP messages */
	u_int64_t	cn_tx_iapp;	/* sent IAPP messages */
	u_int64_t	cn_rx_apme;	/* received Host AP messages */
	u_int64_t	cn_tx_apme;	/* sent Host AP messages */
};

#define HOSTAPD_ENTRY_MASK_ADD(_a, _m)	do {					\
	(_a)[0] &= (_m)[0];							\
	(_a)[1] &= (_m)[1];							\
	(_a)[2] &= (_m)[2];							\
	(_a)[3] &= (_m)[3];							\
	(_a)[4] &= (_m)[4];							\
	(_a)[5] &= (_m)[5];							\
} while (0);
#define HOSTAPD_ENTRY_MASK_MATCH(_e, _b)	(				\
	((_e)->e_lladdr[0] == ((_b)[0] & (_e)->e_addr.a_mask[0])) &&		\
	((_e)->e_lladdr[1] == ((_b)[1] & (_e)->e_addr.a_mask[1])) &&		\
	((_e)->e_lladdr[2] == ((_b)[2] & (_e)->e_addr.a_mask[2])) &&		\
	((_e)->e_lladdr[3] == ((_b)[3] & (_e)->e_addr.a_mask[3])) &&		\
	((_e)->e_lladdr[4] == ((_b)[4] & (_e)->e_addr.a_mask[4])) &&		\
	((_e)->e_lladdr[5] == ((_b)[5] & (_e)->e_addr.a_mask[5]))		\
)

struct hostapd_entry {
	u_int8_t			e_lladdr[IEEE80211_ADDR_LEN];
	u_int8_t			e_flags;

#define HOSTAPD_ENTRY_F_LLADDR		0x00
#define HOSTAPD_ENTRY_F_MASK		0x01
#define HOSTAPD_ENTRY_F_IPV4		0x02

	union {
		u_int8_t		a_mask[IEEE80211_ADDR_LEN];
		struct in_addr		a_ipv4;
	}				e_addr;

	RB_ENTRY(hostapd_entry)		e_nodes;
	TAILQ_ENTRY(hostapd_entry)	e_entries;
};

#define e_mask				e_addr.a_mask
#define e_ipv4				e_addr.a_ipv4

#define HOSTAPD_TABLE_NAMELEN		32

RB_HEAD(hostapd_tree, hostapd_entry);

struct hostapd_table {
	char				t_name[HOSTAPD_TABLE_NAMELEN];
	u_int8_t			t_flags;

#define HOSTAPD_TABLE_F_CONST		0x01

	struct hostapd_tree		t_tree;
	TAILQ_HEAD(, hostapd_entry)	t_mask_head;
	TAILQ_ENTRY(hostapd_table)	t_entries;
};

struct hostapd_ieee80211_frame {
	u_int8_t	i_fc[2];
	u_int8_t	i_dur[2];
	u_int8_t	i_from[IEEE80211_ADDR_LEN];
	u_int8_t	i_to[IEEE80211_ADDR_LEN];
	u_int8_t	i_bssid[IEEE80211_ADDR_LEN];
	u_int8_t	i_seq[2];
	void		*i_data;
	u_int		i_data_len;
};

enum hostapd_action {
	HOSTAPD_ACTION_NONE	= 0,
	HOSTAPD_ACTION_LOG	= 1,
	HOSTAPD_ACTION_RADIOTAP	= 2,
	HOSTAPD_ACTION_FRAME	= 3,
	HOSTAPD_ACTION_ADDNODE	= 4,
	HOSTAPD_ACTION_DELNODE	= 5,
	HOSTAPD_ACTION_RESEND	= 6
};

struct hostapd_action_data {
	union {
		struct hostapd_ieee80211_frame	u_frame;
		u_int8_t			u_lladdr[IEEE80211_ADDR_LEN];
	} a_data;
	u_int16_t				a_flags;

#define HOSTAPD_ACTION_F_REF_FROM		0x0001
#define HOSTAPD_ACTION_F_REF_TO			0x0002
#define HOSTAPD_ACTION_F_REF_BSSID		0x0004
#define HOSTAPD_ACTION_F_REF_RANDOM		0x0008
#define HOSTAPD_ACTION_F_REF_FROM_M		0x000f
#define HOSTAPD_ACTION_F_REF_FROM_S		0
#define HOSTAPD_ACTION_F_REF_TO_M		0x00f0
#define HOSTAPD_ACTION_F_REF_TO_S		4
#define HOSTAPD_ACTION_F_REF_BSSID_M		0x0f00
#define HOSTAPD_ACTION_F_REF_BSSID_S		8
#define HOSTAPD_ACTION_F_REF_M			0x0fff
#define HOSTAPD_ACTION_F_OPT_DIR_AUTO		0x1000
#define HOSTAPD_ACTION_F_OPT_LLADDR		0x2000
#define HOSTAPD_ACTION_F_OPT_TABLE		0x4000
};

#define a_frame					a_data.u_frame
#define a_lladdr				a_data.u_lladdr

struct hostapd_frame {
	struct hostapd_ieee80211_frame	f_frame;
	u_int32_t			f_flags;

#define HOSTAPD_FRAME_F_TYPE		0x00000001
#define HOSTAPD_FRAME_F_TYPE_N		0x00000002
#define HOSTAPD_FRAME_F_SUBTYPE		0x00000004
#define HOSTAPD_FRAME_F_SUBTYPE_N	0x00000008
#define HOSTAPD_FRAME_F_DIR		0x00000010
#define HOSTAPD_FRAME_F_DIR_N		0x00000020
#define HOSTAPD_FRAME_F_FROM		0x00000040
#define HOSTAPD_FRAME_F_FROM_N		0x00000080
#define HOSTAPD_FRAME_F_FROM_TABLE	0x00000100
#define HOSTAPD_FRAME_F_FROM_M		0x000001c0
#define HOSTAPD_FRAME_F_TO		0x00000200
#define HOSTAPD_FRAME_F_TO_N		0x00000400
#define HOSTAPD_FRAME_F_TO_TABLE	0x00000800
#define HOSTAPD_FRAME_F_TO_M		0x00000e00
#define HOSTAPD_FRAME_F_BSSID		0x00001000
#define HOSTAPD_FRAME_F_BSSID_N		0x00002000
#define HOSTAPD_FRAME_F_BSSID_TABLE	0x00004000
#define HOSTAPD_FRAME_F_BSSID_M		0x00007000
#define HOSTAPD_FRAME_F_M		0x0fffffff
#define HOSTAPD_FRAME_F_RET_OK		0x00000000
#define HOSTAPD_FRAME_F_RET_QUICK	0x10000000
#define HOSTAPD_FRAME_F_RET_SKIP	0x20000000
#define HOSTAPD_FRAME_F_RET_M		0xf0000000
#define HOSTAPD_FRAME_F_RET_S		28

#define HOSTAPD_FRAME_TABLE						\
	(HOSTAPD_FRAME_F_FROM_TABLE | HOSTAPD_FRAME_F_TO_TABLE |	\
	HOSTAPD_FRAME_F_BSSID_TABLE)
#define HOSTAPD_FRAME_N							\
	(HOSTAPD_FRAME_F_FROM_N | HOSTAPD_FRAME_F_TO_N |		\
	HOSTAPD_FRAME_F_BSSID_N)

	struct hostapd_table		*f_from, *f_to, *f_bssid;
	struct timeval			f_limit, f_then, f_last;
	long				f_rate, f_rate_intval;
	long				f_rate_cnt, f_rate_delay;

	enum hostapd_action		f_action;
	u_int32_t			f_action_flags;

#define HOSTAPD_ACTION_VERBOSE		0x00000001

	struct hostapd_action_data	f_action_data;

	TAILQ_ENTRY(hostapd_frame)	f_entries;
};

struct hostapd_config {
	int				c_apme;
	int				c_apme_raw;
	u_int				c_apme_rawlen;
	struct event			c_apme_ev;
	char				c_apme_iface[IFNAMSIZ];
	int				c_apme_n;
	u_int8_t			c_apme_bssid[IEEE80211_ADDR_LEN];
	u_int				c_apme_dlt;

	u_int16_t			c_iapp;
	int				c_iapp_raw;
	char				c_iapp_iface[IFNAMSIZ];
	int				c_iapp_udp;
	struct event			c_iapp_udp_ev;
	u_int16_t			c_iapp_udp_port;
	struct sockaddr_in		c_iapp_addr;
	struct sockaddr_in		c_iapp_broadcast;
	struct sockaddr_in		c_iapp_multicast;

	u_int8_t			c_flags;

#define HOSTAPD_CFG_F_APME		0x01
#define HOSTAPD_CFG_F_IAPP		0x02
#define HOSTAPD_CFG_F_IAPP_PASSIVE	0x04
#define HOSTAPD_CFG_F_RAW		0x08
#define HOSTAPD_CFG_F_UDP		0x10
#define HOSTAPD_CFG_F_BRDCAST		0x20
#define HOSTAPD_CFG_F_PRIV		0x40

	struct event			c_priv_ev;

	char				c_config[MAXPATHLEN];

	u_int				c_verbose;
	u_int				c_debug;
	u_int				c_id;

	struct hostapd_counter		c_stats;

	TAILQ_HEAD(, hostapd_table)	c_tables;
	TAILQ_HEAD(, hostapd_frame)	c_frames;
};

#define	HOSTAPD_USER	"_hostapd"
#define HOSTAPD_CONFIG	"/etc/hostapd.conf"
#define HOSTAPD_DLT	DLT_IEEE802_11

#define HOSTAPD_LOG		0
#define HOSTAPD_LOG_VERBOSE	1
#define HOSTAPD_LOG_DEBUG	2

#define PRINTF			hostapd_printf
#define etheraddr_string(_s)	ether_ntoa((struct ether_addr*)_s)
#define TTEST2(var, l)		(						\
	snapend - (l) <= snapend && (const u_char *)&(var) <= snapend - (l)	\
)
#define TTEST(var)		TTEST2(var, sizeof(var))
#define TCHECK2(var, l)		if (!TTEST2(var, l)) goto trunc
#define TCHECK(var)		TCHECK2(var, sizeof(var))

__BEGIN_DECLS

void	 hostapd_log(u_int, const char *, ...);
void	 hostapd_printf(const char *, ...);
void	 hostapd_fatal(const char *, ...);
int	 hostapd_bpf_open(u_int);
void	 hostapd_cleanup(struct hostapd_config *);
int	 hostapd_check_file_secrecy(int, const char *);
void	 hostapd_randval(u_int8_t *, const u_int)
	    __attribute__((__bounded__(__buffer__, 1, 2)));

struct hostapd_table *hostapd_table_add(struct hostapd_config *,
	    const char *);
struct hostapd_table *hostapd_table_lookup(struct hostapd_config *,
	    const char *);
struct hostapd_entry *hostapd_entry_add(struct hostapd_table *,
	    u_int8_t *);
struct hostapd_entry *hostapd_entry_lookup(struct hostapd_table *,
	    u_int8_t *);
void	 hostapd_entry_update(struct hostapd_table *,
	    struct hostapd_entry *);
int	 hostapd_entry_cmp(struct hostapd_entry *, struct hostapd_entry *);

RB_PROTOTYPE(hostapd_tree, hostapd_entry, e_nodes, hostapd_entry_cmp);

int	 hostapd_parse_file(struct hostapd_config *);
int	 hostapd_parse_symset(char *);

void	 hostapd_priv_init(struct hostapd_config *);
int	 hostapd_priv_llc_xid(struct hostapd_config *, struct hostapd_node *);
void	 hostapd_priv_apme_bssid(struct hostapd_config *);
int	 hostapd_priv_apme_getnode(struct hostapd_config *,
	    struct hostapd_node *);
int	 hostapd_priv_apme_setnode(struct hostapd_config *,
	    struct hostapd_node *node, int);

void	 hostapd_apme_init(struct hostapd_config *);
void	 hostapd_apme_input(int, short, void *);
int	 hostapd_apme_output(struct hostapd_config *,
	    struct hostapd_ieee80211_frame *);
int	 hostapd_apme_addnode(struct hostapd_config *,
	    struct hostapd_node *node);
int	 hostapd_apme_delnode(struct hostapd_config *,
	    struct hostapd_node *node);
int	 hostapd_apme_offset(struct hostapd_config *, u_int8_t *,
	    const u_int);

void	 hostapd_iapp_init(struct hostapd_config *);
void	 hostapd_iapp_term(struct hostapd_config *);
int	 hostapd_iapp_add_notify(struct hostapd_config *,
	    struct hostapd_node *);
int	 hostapd_iapp_radiotap(struct hostapd_config *,
	    u_int8_t *, const u_int);
void	 hostapd_iapp_input(int, short, void *);

void	 hostapd_llc_init(struct hostapd_config *);
int	 hostapd_llc_send_xid(struct hostapd_config *, struct hostapd_node *);

int	 hostapd_handle_input(struct hostapd_config *, u_int8_t *, u_int);

void	 hostapd_print_ieee80211(u_int, u_int, u_int8_t *, u_int);

__END_DECLS

#endif /* _HOSTAPD_H */
