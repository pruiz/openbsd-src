/*	$OpenBSD: bgpctl.c,v 1.28 2004/01/17 18:27:37 henning Exp $ */

/*
 * Copyright (c) 2003 Henning Brauer <henning@openbsd.org>
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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <net/if.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bgpd.h"
#include "session.h"
#include "log.h"

enum actions {
	SHOW,
	SHOW_SUMMARY,
	SHOW_NEIGHBOR,
	SHOW_NEIGHBOR_TIMERS,
	SHOW_FIB,
	SHOW_NEXTHOP,
	SHOW_INTERFACE,
	RELOAD,
	FIB,
	FIB_COUPLE,
	FIB_DECOUPLE,
	NEIGHBOR,
	NEIGHBOR_UP,
	NEIGHBOR_DOWN
};

enum neighbor_views {
	NV_DEFAULT,
	NV_TIMERS
};

struct keywords {
	const char	*keyword;
	int		 value;
};

static const struct keywords keywords_main[] = {
	{ "reload",	RELOAD},
	{ "show",	SHOW},
	{ "fib",	FIB},
	{ "neighbor",	NEIGHBOR}
};

static const struct keywords keywords_show[] = {
	{ "neighbor",	SHOW_NEIGHBOR},
	{ "summary",	SHOW_SUMMARY},
	{ "fib",	SHOW_FIB},
	{ "nexthop",	SHOW_NEXTHOP},
	{ "interfaces",	SHOW_INTERFACE}
};

static const struct keywords keywords_show_neighbor[] = {
	{ "timers",	SHOW_NEIGHBOR_TIMERS},
	{ "messages",	SHOW_NEIGHBOR}
};

static const struct keywords keywords_fib[] = {
	{ "couple",	FIB_COUPLE},
	{ "decouple",	FIB_DECOUPLE}
};

static const struct keywords keywords_neighbor[] = {
	{ "up",		NEIGHBOR_UP},
	{ "down",	NEIGHBOR_DOWN}
};

int		 main(int, char *[]);
int		 match_keyword(const char *, const struct keywords [], size_t);
void		 show_summary_head(void);
int		 show_summary_msg(struct imsg *);
int		 show_neighbor_msg(struct imsg *, enum neighbor_views);
void		 print_neighbor_msgstats(struct peer *);
void		 print_neighbor_timers(struct peer *);
void		 print_timer(const char *, time_t, u_int);
static char	*fmt_timeframe(time_t t);
static char	*fmt_timeframe_core(time_t t);
int		 parse_addr(const char *, struct bgpd_addr *);
void		 show_fib_head(void);
int		 show_fib_msg(struct imsg *);
void		 show_nexthop_head(void);
int		 show_nexthop_msg(struct imsg *);
void		 show_interface_head(void);
int		 show_interface_msg(struct imsg *);

struct imsgbuf	ibuf;

int
main(int argc, char *argv[])
{
	struct sockaddr_un	 sun;
	int			 fd, n, done;
	int			 i, flags;
	struct imsg		 imsg;
	enum actions		 action = SHOW_SUMMARY;
	struct bgpd_addr	 addr;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		err(1, "control_init: socket");
		exit(1);
	}

	bzero(&sun, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strlcpy(sun.sun_path, SOCKET_NAME, sizeof(sun.sun_path));
	if (connect(fd, (struct sockaddr *)&sun, sizeof(sun)) == -1) {
		err(1, "connect: %s", SOCKET_NAME);
		exit(1);
	}

	imsg_init(&ibuf, fd);
	done = 0;

	if (argc >= 2)
		action = match_keyword(argv[1], keywords_main,
		    sizeof(keywords_main)/sizeof(keywords_main[0]));

again:
	switch (action) {
	case SHOW:
		if (argc >= 3) {
			action = match_keyword(argv[2], keywords_show,
			    sizeof(keywords_show)/sizeof(keywords_show[0]));
			goto again;
		}
		/* fallthrough */
	case SHOW_SUMMARY:
		if (argc >= 4)
			errx(1, "\"show summary\" does not take arguments");
		imsg_compose(&ibuf, IMSG_CTL_SHOW_NEIGHBOR, 0, NULL, 0);
		show_summary_head();
		break;
	case SHOW_FIB:
		flags = 0;
		bzero(&addr, sizeof(addr));
		for (i = 3; i < argc; i++)
			if (!strncmp(argv[i], "connected", strlen(argv[i])))
				flags |= F_CONNECTED;
			else if (!strncmp(argv[i], "static", strlen(argv[i])))
				flags |= F_STATIC;
			else if (!strncmp(argv[i], "bgp", strlen(argv[i])))
				flags |= F_BGPD_INSERTED;
			else if (!strncmp(argv[i], "nexthop", strlen(argv[i])))
				flags |= F_NEXTHOP;
			else if (!parse_addr(argv[i], &addr))
				errx(1, "usage: \"show fib connected|static|"
				    "bgp|nexthop|[address]");
		if (!addr.af)
			imsg_compose(&ibuf, IMSG_CTL_KROUTE, 0, &flags,
			    sizeof(flags));
		else
			imsg_compose(&ibuf, IMSG_CTL_KROUTE_ADDR, 0, &addr,
			    sizeof(addr));
		show_fib_head();
		break;
	case SHOW_NEXTHOP:
		imsg_compose(&ibuf, IMSG_CTL_SHOW_NEXTHOP, 0, NULL, 0);
		show_nexthop_head();
		break;
	case SHOW_INTERFACE:
		imsg_compose(&ibuf, IMSG_CTL_SHOW_INTERFACE, 0, NULL, 0);
		show_interface_head();
		break;
	case SHOW_NEIGHBOR:
	case SHOW_NEIGHBOR_TIMERS:
		/* get ip address of neighbor, limit query to that */
		if (argc >= 4) {
			if (!parse_addr(argv[3], &addr))
				errx(1, "%s: not an IP address", argv[3]);
			imsg_compose(&ibuf, IMSG_CTL_SHOW_NEIGHBOR, 0,
			    &addr, sizeof(addr));
		} else
			imsg_compose(&ibuf, IMSG_CTL_SHOW_NEIGHBOR, 0, NULL, 0);

		if (argc >= 5)
			action = match_keyword(argv[4], keywords_show_neighbor,
			    sizeof(keywords_show_neighbor)/
			    sizeof(keywords_show_neighbor[0]));
		break;
	case RELOAD:
		if (argc >= 3)
			errx(1, "\"reload\" takes no options");
		imsg_compose(&ibuf, IMSG_CTL_RELOAD, 0, NULL, 0);
		printf("reload request sent.\n");
		done = 1;
		break;
	case FIB:
		if (argc >= 3) {
			action = match_keyword(argv[2], keywords_fib,
			    sizeof(keywords_fib)/sizeof(keywords_fib[0]));
			goto again;
		} else
			errx(1, "fib [couple|decouple]");
		break;
	case FIB_COUPLE:
		if (argc >= 4)
			errx(1, "\"fib couple\" takes no options");
		imsg_compose(&ibuf, IMSG_CTL_FIB_COUPLE, 0, NULL, 0);
		printf("couple request sent.\n");
		done = 1;
		break;
	case FIB_DECOUPLE:
		if (argc >= 4)
			errx(1, "\"fib decouple\" takes no options");
		imsg_compose(&ibuf, IMSG_CTL_FIB_DECOUPLE, 0, NULL, 0);
		printf("decouple request sent.\n");
		done = 1;
		break;
	case NEIGHBOR:
		if (argc < 4)
			errx(1, "usage: neighbor address command");
		if (!parse_addr(argv[2], &addr))
			errx(1, "%s: not an IP address", argv[2]);
		action = match_keyword(argv[3], keywords_neighbor,
			    sizeof(keywords_neighbor)/
			    sizeof(keywords_neighbor[0]));
		goto again;
		break;
	case NEIGHBOR_UP:
		imsg_compose(&ibuf, IMSG_CTL_NEIGHBOR_UP, 0,
		    &addr, sizeof(addr));
		printf("request sent.\n");
		done = 1;
		break;
	case NEIGHBOR_DOWN:
		imsg_compose(&ibuf, IMSG_CTL_NEIGHBOR_DOWN, 0,
		    &addr, sizeof(addr));
		printf("request sent.\n");
		done = 1;
		break;
	}

	while (ibuf.w.queued)
		if (msgbuf_write(&ibuf.w) < 0)
			err(1, "write error");

	while (!done) {
		if ((n = imsg_read(&ibuf)) == -1)
			errx(1, "imsg_read error");
		if (n == 0)
			errx(1, "pipe closed");

		while (!done) {
			if ((n = imsg_get(&ibuf, &imsg)) == -1)
				errx(1, "imsg_get error");
			if (n == 0)
				break;
			switch (action) {
			case SHOW:
			case SHOW_SUMMARY:
				done = show_summary_msg(&imsg);
				break;
			case SHOW_FIB:
				done = show_fib_msg(&imsg);
				break;
			case SHOW_NEXTHOP:
				done = show_nexthop_msg(&imsg);
				break;
			case SHOW_INTERFACE:
				done = show_interface_msg(&imsg);
				break;
			case SHOW_NEIGHBOR:
				done = show_neighbor_msg(&imsg, NV_DEFAULT);
				break;
			case SHOW_NEIGHBOR_TIMERS:
				done = show_neighbor_msg(&imsg, NV_TIMERS);
				break;
			case RELOAD:
			case FIB:
			case FIB_COUPLE:
			case FIB_DECOUPLE:
			case NEIGHBOR:
			case NEIGHBOR_UP:
			case NEIGHBOR_DOWN:
				break;
			}
			imsg_free(&imsg);
		}
	}
	close(fd);
}

int
match_keyword(const char *word, const struct keywords table[], size_t cnt)
{
	u_int	match, res, i;

	match = res = 0;

	for (i = 0; i < cnt; i++)
		if (strncmp(word, table[i].keyword,
		    strlen(word)) == 0) {
			match++;
			res = table[i].value;
		}

	if (match > 1)
		errx(1, "ambigous command: %s", word);
	if (match < 1)
		errx(1, "unknown command: %s", word);

	return (res);
}

void
show_summary_head(void)
{
	printf("%-15s %-5s %-10s %-10s %-8s %s\n", "Neighbor", "AS", "MsgRcvd",
	    "MsgSent", "Up/Down", "State");
}

int
show_summary_msg(struct imsg *imsg)
{
	struct peer		*p;

	switch (imsg->hdr.type) {
	case IMSG_CTL_SHOW_NEIGHBOR:
		p = imsg->data;
		printf("%-15s %5u %10llu %10llu %-8s %s\n",
		    inet_ntoa(p->conf.remote_addr.sin_addr),
		    p->conf.remote_as,
		    p->stats.msg_rcvd_open + p->stats.msg_rcvd_notification +
		    p->stats.msg_rcvd_update + p->stats.msg_rcvd_keepalive,
		    p->stats.msg_sent_open + p->stats.msg_sent_notification +
		    p->stats.msg_sent_update + p->stats.msg_sent_keepalive,
		    fmt_timeframe(p->stats.last_updown),
		    statenames[p->state]);
		break;
	case IMSG_CTL_END:
		return (1);
		break;
	default:
		break;
	}

	return (0);
}

int
show_neighbor_msg(struct imsg *imsg, enum neighbor_views nv)
{
	struct peer		*p;
	struct sockaddr_in	*sa_in;

	switch (imsg->hdr.type) {
	case IMSG_CTL_SHOW_NEIGHBOR:
		p = imsg->data;
		printf("BGP neighbor is %s, remote AS %u\n",
		    inet_ntoa(p->conf.remote_addr.sin_addr),
		    p->conf.remote_as);
		if (p->conf.descr[0])
			printf(" Description: %s\n", p->conf.descr);
		printf("  BGP version 4, remote router-id %s\n",
		    log_ntoa(p->remote_bgpid));
		printf("  BGP state = %s", statenames[p->state]);
		if (p->stats.last_updown != 0)
			printf(", %s for %s",
			    p->state == STATE_ESTABLISHED ? "up" : "down",
			    fmt_timeframe(p->stats.last_updown));
		printf("\n");
		printf("  Last read %s, holdtime %us, keepalive interval %us\n",
		    fmt_timeframe(p->stats.last_read),
		    p->holdtime, p->holdtime/3);
		printf("\n");
		switch (nv) {
		case NV_DEFAULT:
			print_neighbor_msgstats(p);
			break;
		case NV_TIMERS:
			print_neighbor_timers(p);
			break;
		}
		printf("\n");
		if (p->sa_local.ss_family == AF_INET) {
			sa_in = (struct sockaddr_in *)&p->sa_local;
			printf("  Local host:   %20s, Local port:   %5u\n",
			    log_ntoa(sa_in->sin_addr.s_addr),
			    ntohs(sa_in->sin_port));
		}
		if (p->sa_remote.ss_family == AF_INET) {
			sa_in = (struct sockaddr_in *)&p->sa_remote;
			printf("  Foreign host: %20s, Foreign port: %5u\n",
			    log_ntoa(sa_in->sin_addr.s_addr),
			    ntohs(sa_in->sin_port));
		}
		printf("\n");
		break;
	case IMSG_CTL_END:
		return (1);
		break;
	default:
		break;
	}

	return (0);
}

void
print_neighbor_msgstats(struct peer *p)
{
	printf("  Message statistics:\n");
	printf("  %-15s %-10s %-10s\n", "", "Sent", "Received");
	printf("  %-15s %10llu %10llu\n", "Opens",
	    p->stats.msg_sent_open, p->stats.msg_rcvd_open);
	printf("  %-15s %10llu %10llu\n", "Notifications",
	    p->stats.msg_sent_notification, p->stats.msg_rcvd_notification);
	printf("  %-15s %10llu %10llu\n", "Updates",
	    p->stats.msg_sent_update, p->stats.msg_rcvd_update);
	printf("  %-15s %10llu %10llu\n", "Keepalives",
	    p->stats.msg_sent_keepalive, p->stats.msg_rcvd_keepalive);
	printf("  %-15s %10llu %10llu\n", "Total",
	    p->stats.msg_sent_open + p->stats.msg_sent_notification +
	    p->stats.msg_sent_update + p->stats.msg_sent_keepalive,
	    p->stats.msg_rcvd_open + p->stats.msg_rcvd_notification +
	    p->stats.msg_rcvd_update + p->stats.msg_rcvd_keepalive);
}

void
print_neighbor_timers(struct peer *p)
{
	print_timer("IdleHoldTimer:", p->IdleHoldTimer, p->IdleHoldTime);
	print_timer("ConnectRetryTimer:", p->ConnectRetryTimer,
	    INTERVAL_CONNECTRETRY);
	print_timer("HoldTimer:", p->HoldTimer, p->holdtime);
	print_timer("KeepaliveTimer:", p->KeepaliveTimer, p->holdtime/3);
}

void
print_timer(const char *name, time_t val, u_int interval)
{
	int	d;

	d = val - time(NULL);
	printf("  %-20s ", name);

	if (val == 0)
		printf("%-20s", "not running");
	else if (d <= 0)
		printf("%-20s", "due");
	else
		printf("due in %-13s", fmt_timeframe_core(d));

	printf("Interval: %5us\n", interval);
}

#define TF_BUFS	8
#define TF_LEN	9

static char *
fmt_timeframe(time_t t)
{
	if (t == 0)
		return ("Never");
	else
		return (fmt_timeframe_core(time(NULL) - t));
}

static char *
fmt_timeframe_core(time_t t)
{
	char		*buf;
	static char	 tfbuf[TF_BUFS][TF_LEN];	/* ring buffer */
	static int	 idx = 0;
	unsigned	 sec, min, hrs, day, week;

	buf = tfbuf[idx++];
	if (idx == TF_BUFS)
		idx = 0;

	week = t;

	sec = week % 60;
	week /= 60;
	min = week % 60;
	week /= 60;
	hrs = week % 24;
	week /= 24;
	day = week % 7;
	week /= 7;

	if (week > 0)
		snprintf(buf, TF_LEN, "%02uw%01ud%02uh", week, day, hrs);
	else if (day > 0)
		snprintf(buf, TF_LEN, "%01ud%02uh%02um", day, hrs, min);
	else
		snprintf(buf, TF_LEN, "%02u:%02u:%02u", hrs, min, sec);

	return (buf);
}

int
parse_addr(const char *word, struct bgpd_addr *addr)
{
	struct in_addr	ina;

	bzero(addr, sizeof(struct bgpd_addr));
	bzero(&ina, sizeof(ina));

	if (inet_pton(AF_INET, word, &ina)) {
		addr->af = AF_INET;
		addr->v4 = ina;
		return (1);
	}

	return (0);
}

void
show_fib_head(void)
{
	printf("flags: * = valid, B = BGP, C = Connected, S = Static\n");
	printf("       N = BGP Nexthop reachable via this route\n\n");
	printf("flags destination          gateway\n");
}

int
show_fib_msg(struct imsg *imsg)
{
	struct kroute		*k;
	char			*p;

	switch (imsg->hdr.type) {
	case IMSG_CTL_KROUTE:
		if (imsg->hdr.len < IMSG_HEADER_SIZE + sizeof(struct kroute))
			errx(1, "wrong imsg len");
		k = imsg->data;

		if (k->flags & F_DOWN)
			printf(" ");
		else
			printf("*");

		if (k->flags & F_BGPD_INSERTED)
			printf("B");
		else if (k->flags & F_CONNECTED)
			printf("C");
		else if (k->flags & F_KERNEL)
			printf("S");
		else
			printf(" ");

		if (k->flags & F_NEXTHOP)
			printf("N");
		else
			printf(" ");

		printf("   ");
		if (asprintf(&p, "%s/%u", log_ntoa(k->prefix), k->prefixlen) ==
		   -1)
			err(1, NULL);
		printf("%-20s ", p);
		free(p);

		if (k->nexthop)
			printf("%s", log_ntoa(k->nexthop));
		else if (k->flags & F_CONNECTED)
			printf("link#%u", k->ifindex);
		printf("\n");

		break;
	case IMSG_CTL_END:
		return (1);
		break;
	default:
		break;
	}

	return (0);
}

void
show_nexthop_head(void)
{
	printf("%-20s %s\n", "Nexthop", "State");
}

int
show_nexthop_msg(struct imsg *imsg)
{
	struct ctl_show_nexthop	*p;

	switch (imsg->hdr.type) {
	case IMSG_CTL_SHOW_NEXTHOP:
		p = imsg->data;
		printf("%-20s %s\n", inet_ntoa(p->addr.v4),
		    p->valid ? "valid" : "invalid");
		break;
	case IMSG_CTL_END:
		return (1);
		break;
	default:
		break;
	}

	return (0);
}


void
show_interface_head(void)
{
	printf("%-20s%s\n", "Interface", "Flags");
}

int
show_interface_msg(struct imsg *imsg)
{
	struct kif	*k;

	switch (imsg->hdr.type) {
	case IMSG_CTL_SHOW_INTERFACE:
		k = imsg->data;
		printf("%-20s", k->ifname);
		if (k->flags & IFF_UP)
			printf("UP ");
		printf("\n");
		break;
	case IMSG_CTL_END:
		return (1);
		break;
	default:
printf("beep");
		break;
	}

	return (0);
}
