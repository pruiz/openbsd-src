/*
 * $Header: /srv/cvs/src/usr.sbin/route6d/route6d.h,v 1.1 1999/12/11 10:36:02 itojun Exp $
 */

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

/* not yet in use 
#define	ROUTE6D_CONF	"/usr/local/v6/etc/route6d.conf"
*/

#define	ROUTE6D_DUMP	"/var/tmp/route6d_dump"
#define	ROUTE6D_PID	"/var/run/route6d.pid"

#define	RIP6_VERSION	1

#define	RIP6_REQUEST	1
#define	RIP6_RESPONSE	2

struct netinfo6 {
	struct	in6_addr	rip6_dest;
	u_short	rip6_tag;
	u_char	rip6_plen;
	u_char	rip6_metric;
};

struct	rip6 {
	u_char	rip6_cmd;
	u_char	rip6_vers;
	u_char	rip6_res1[2];
	union {
		struct	netinfo6	ru6_nets[1];
		char	ru6_tracefile[1];
	} rip6un;
#define	rip6_nets	rip6un.ru6_nets
#define	rip6_tracefile	rip6un.ru6_tracefile
};

#define	HOPCNT_INFINITY6	16
#define	NEXTHOP_METRIC		0xff
#define	RIP6_MAXMTU		1500

#define	IFMINMTU		576

#ifndef	DEBUG
#define	SUPPLY_INTERVAL6	30
#define	RIP_LIFETIME		180
#define	RIP_HOLDDOWN		120
#define	RIP_TRIG_INT6_MAX	5
#define	RIP_TRIG_INT6_MIN	1
#else
/* only for debugging; can not wait for 30sec to appear a bug */
#define	SUPPLY_INTERVAL6	10
#define	RIP_LIFETIME		60
#define	RIP_HOLDDOWN		40
#define	RIP_TRIG_INT6_MAX	5
#define	RIP_TRIG_INT6_MIN	1
#endif

#define	RIP6_PORT		521
#define	RIP6_DEST		"ff02::9"

#define	LOOPBACK_IF		"lo0"
