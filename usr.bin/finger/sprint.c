/*	$OpenBSD: sprint.c,v 1.15 2015/03/15 00:41:28 millert Exp $	*/

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Tony Nardo of the Johns Hopkins University/Applied Physics Lab.
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
 */

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include "finger.h"
#include "extern.h"

void
sflag_print(void)
{
	PERSON *pn;
	WHERE *w;
	int cnt;
	char *p;
	PERSON **list;

	list = sort();
	/*
	 * short format --
	 *	login name
	 *	real name
	 *	terminal name (the XX of ttyXX)
	 *	if terminal writeable (add an '*' to the terminal name
	 *		if not)
	 *	if logged in show idle time and day logged in, else
	 *		show last login date and time.  If > 6 months,
	 *		show year instead of time.  If < 6 days,
	 *		show day name instead of month & day.
	 *	if -h given
	 *		remote host
	 *	else if -o given (overriding -h) (default)
	 *		office location
	 *		office phone
	 */
#define NAME_WIDTH	8
#define	MAXREALNAME	20
#define	MAXHOSTNAME	20
	(void)printf("%-*.*s %-*s %s %s\n",
	    NAME_WIDTH, UT_NAMESIZE, "Login", MAXREALNAME,
	    "Name", "Tty  Idle  Login Time  ",
	    (oflag) ? "Office     Office Phone" : "Where");
	for (cnt = 0; cnt < entries; ++cnt) {
		pn = list[cnt];
		for (w = pn->whead; w != NULL; w = w->next) {
			(void)printf("%-*.*s %-*.*s ",
			    NAME_WIDTH, UT_NAMESIZE, pn->name,
			    MAXREALNAME, MAXREALNAME,
			    pn->realname ? pn->realname : "");
			if (!w->loginat) {
				(void)printf("  *     *  No logins   ");
				goto office;
			}
			(void)putchar(w->info == LOGGEDIN && !w->writable ?
			    '*' : ' ');
			if (*w->tty)
				(void)printf("%-2.2s ",
				    w->tty[0] != 't' || w->tty[1] != 't' ||
				    w->tty[2] != 'y' ? w->tty : w->tty + 3);
			else
				(void)printf("   ");
			if (w->info == LOGGEDIN) {
				stimeprint(w);
				(void)printf("  ");
			} else
				(void)printf("    *  ");
			p = ctime(&w->loginat);

			if (now - w->loginat < SECSPERDAY * 6)
				(void)printf("   %.3s", p);
			else
				(void)printf("%.6s", p + 4);
			if (now - w->loginat >= SIXMONTHS)
				(void)printf(" %.4s ", p + 20);
			else
				(void)printf(" %.5s", p + 11);
office:
			putchar(' ');
			if (oflag) {
				if (pn->office)
					(void)printf("%-10.10s", pn->office);
				else if (pn->officephone)
					(void)printf("%-10.10s", " ");
				if (pn->officephone)
					(void)printf(" %-.15s",
					    prphone(pn->officephone));
			} else
				(void)printf("%.*s", MAXHOSTNAME, w->host);
			putchar('\n');
		}
	}
}

PERSON **
sort(void)
{
	PERSON *pn, **lp;
	PERSON **list;

	if (!(list = (PERSON **)calloc((u_int)entries, sizeof(PERSON *))))
		err(1, "malloc");
	for (lp = list, pn = phead; pn != NULL; pn = pn->next)
		*lp++ = pn;
	(void)qsort(list, entries, sizeof(PERSON *), psort);
	return (list);
}

int
psort(const void *p, const void *t)
{
	return (strcmp((*(PERSON **)p)->name, (*(PERSON **)t)->name));
}

void
stimeprint(WHERE *w)
{
	struct tm *delta;

	delta = gmtime(&w->idletime);
	if (!delta->tm_yday) {
		if (!delta->tm_hour) {
			if (!delta->tm_min)
				(void)printf("    -");
			else
				(void)printf("%5d", delta->tm_min);
		 } else {
			(void)printf("%2d:%02d",
			    delta->tm_hour, delta->tm_min);
		 }
	} else
		(void)printf("%4dd", delta->tm_yday);
}
