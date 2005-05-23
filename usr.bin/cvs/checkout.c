/*	$OpenBSD: checkout.c,v 1.21 2005/05/23 17:30:35 xsa Exp $	*/
/*
 * Copyright (c) 2004 Jean-Francois Brousseau <jfb@openbsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cvs.h"
#include "log.h"
#include "file.h"
#include "proto.h"


#define CVS_LISTMOD    1
#define CVS_STATMOD    2

int cvs_checkout_options(char *, int, char **, int *);
int cvs_checkout_sendflags(struct cvsroot *);

struct cvs_cmd_info cvs_checkout = {
	cvs_checkout_options,
	cvs_checkout_sendflags,
	NULL, NULL, NULL,
	0,
	CVS_REQ_CO,
	CVS_CMD_SENDDIR | CVS_CMD_SENDARGS1 | CVS_CMD_SENDARGS2
};

static char *date, *rev, *koptstr, *tgtdir, *rcsid;
static int statmod = 0;
static int kflag = RCS_KWEXP_DEFAULT;
static int usehead;

int
cvs_checkout_options(char *opt, int argc, char **argv, int *arg)
{
	int ch;

	date = rev = koptstr = tgtdir = rcsid = NULL;
	kflag = RCS_KWEXP_DEFAULT;
	usehead = 0;

	while ((ch = getopt(argc, argv, opt)) != -1) {
		switch (ch) {
		case 'A':
			break;
		case 'c':
			statmod = CVS_LISTMOD;
			break;
		case 'D':
			date = optarg;
			break;
		case 'd':
			tgtdir = optarg;
			break;
		case 'f':
			usehead = 1;
			break;
		case 'j':
			break;
		case 'k':
			koptstr = optarg;
			kflag = rcs_kflag_get(koptstr);
			if (RCS_KWEXP_INVAL(kflag)) {
				cvs_log(LP_ERR,
				    "invalid RCS keyword expansion mode");
				rcs_kflag_usage();
				return (CVS_EX_USAGE);
			}
			break;
		case 'p':
			cvs_noexec = 1;	/* no locks will be created */
			break;
		case 'r':
			rev = optarg;
			break;
		case 's':
			statmod = CVS_STATMOD;
			break;
		case 't':
			rcsid = optarg;
			break;
		default:
			return (CVS_EX_USAGE);
		}
	}

	argc -= optind;
	argv += optind;

	if (!statmod && (argc == 0)) {
		cvs_log(LP_ERR,
		    "must specify at least one module or directory");
		return (CVS_EX_USAGE);
	}

	if (statmod && (argc > 0)) {
		cvs_log(LP_ERR,  "-c and -s must not get any arguments");
		return (CVS_EX_USAGE);
	}

	*arg = optind;
	return (0);
}

int
cvs_checkout_sendflags(struct cvsroot *root)
{
	if (cvs_sendreq(root, CVS_REQ_DIRECTORY, ".") < 0)
		return (CVS_EX_PROTO);
	if (cvs_sendraw(root, root->cr_dir, strlen(root->cr_dir)) < 0)
		return (CVS_EX_PROTO);
	if (cvs_sendraw(root, "\n", 1) < 0)
		return (CVS_EX_PROTO);

	if (cvs_sendreq(root, CVS_REQ_XPANDMOD, NULL) < 0)
		cvs_log(LP_ERR, "failed to expand module");

	if (usehead && (cvs_sendarg(root, "-f", 0) < 0))
		 return (CVS_EX_PROTO);

	/* XXX not too sure why we have to send this arg */
	if (cvs_sendarg(root, "-N", 0) < 0)
		return (CVS_EX_PROTO);

	if ((statmod == CVS_LISTMOD) &&
	    (cvs_sendarg(root, "-c", 0) < 0))
		return (CVS_EX_PROTO);

	if ((statmod == CVS_STATMOD) &&
	    (cvs_sendarg(root, "-s", 0) < 0))
		return (CVS_EX_PROTO);

	return (0);
}
