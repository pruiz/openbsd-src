/*	$OpenBSD: manpath.c,v 1.15 2015/05/07 12:07:29 schwarze Exp $	*/
/*
 * Copyright (c) 2011, 2014, 2015 Ingo Schwarze <schwarze@openbsd.org>
 * Copyright (c) 2011 Kristaps Dzonsons <kristaps@bsd.lv>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/types.h>
#include <sys/stat.h>

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mandoc_aux.h"
#include "manconf.h"

#define MAN_CONF_FILE	"/etc/man.conf"
#define MANPATH_DEFAULT	"/usr/share/man:/usr/X11R6/man:/usr/local/man"

static	void	 manconf_file(struct manconf *, const char *);
static	void	 manpath_add(struct manpaths *, const char *, int);
static	void	 manpath_parseline(struct manpaths *, char *, int);


void
manconf_parse(struct manconf *conf, const char *file,
		char *defp, char *auxp)
{
	char		*insert;

	/* Always prepend -m. */
	manpath_parseline(&conf->manpath, auxp, 1);

	/* If -M is given, it overrides everything else. */
	if (NULL != defp) {
		manpath_parseline(&conf->manpath, defp, 1);
		return;
	}

	/* MANPATH and man.conf(5) cooperate. */
	defp = getenv("MANPATH");
	if (NULL == file)
		file = MAN_CONF_FILE;

	/* No MANPATH; use man.conf(5) only. */
	if (NULL == defp || '\0' == defp[0]) {
		manconf_file(conf, file);
		return;
	}

	/* Prepend man.conf(5) to MANPATH. */
	if (':' == defp[0]) {
		manconf_file(conf, file);
		manpath_parseline(&conf->manpath, defp, 0);
		return;
	}

	/* Append man.conf(5) to MANPATH. */
	if (':' == defp[strlen(defp) - 1]) {
		manpath_parseline(&conf->manpath, defp, 0);
		manconf_file(conf, file);
		return;
	}

	/* Insert man.conf(5) into MANPATH. */
	insert = strstr(defp, "::");
	if (NULL != insert) {
		*insert++ = '\0';
		manpath_parseline(&conf->manpath, defp, 0);
		manconf_file(conf, file);
		manpath_parseline(&conf->manpath, insert + 1, 0);
		return;
	}

	/* MANPATH overrides man.conf(5) completely. */
	manpath_parseline(&conf->manpath, defp, 0);
}

/*
 * Parse a FULL pathname from a colon-separated list of arrays.
 */
static void
manpath_parseline(struct manpaths *dirs, char *path, int complain)
{
	char	*dir;

	if (NULL == path)
		return;

	for (dir = strtok(path, ":"); dir; dir = strtok(NULL, ":"))
		manpath_add(dirs, dir, complain);
}

/*
 * Add a directory to the array, ignoring bad directories.
 * Grow the array one-by-one for simplicity's sake.
 */
static void
manpath_add(struct manpaths *dirs, const char *dir, int complain)
{
	char		 buf[PATH_MAX];
	struct stat	 sb;
	char		*cp;
	size_t		 i;

	if (NULL == (cp = realpath(dir, buf))) {
		if (complain) {
			fputs("manpath: ", stderr);
			perror(dir);
		}
		return;
	}

	for (i = 0; i < dirs->sz; i++)
		if (0 == strcmp(dirs->paths[i], dir))
			return;

	if (stat(cp, &sb) == -1) {
		if (complain) {
			fputs("manpath: ", stderr);
			perror(dir);
		}
		return;
	}

	dirs->paths = mandoc_reallocarray(dirs->paths,
	    dirs->sz + 1, sizeof(char *));

	dirs->paths[dirs->sz++] = mandoc_strdup(cp);
}

void
manconf_free(struct manconf *conf)
{
	size_t		 i;

	for (i = 0; i < conf->manpath.sz; i++)
		free(conf->manpath.paths[i]);

	free(conf->manpath.paths);
	free(conf->output.includes);
	free(conf->output.man);
	free(conf->output.paper);
	free(conf->output.style);
}

static void
manconf_file(struct manconf *conf, const char *file)
{
	const char *const toks[] = { "manpath", "output", "_whatdb" };
	char manpath_default[] = MANPATH_DEFAULT;

	FILE		*stream;
	char		*cp, *ep;
	size_t		 len, tok;

	if ((stream = fopen(file, "r")) == NULL)
		goto out;

	while ((cp = fgetln(stream, &len)) != NULL) {
		ep = cp + len;
		if (ep[-1] != '\n')
			break;
		*--ep = '\0';
		while (isspace((unsigned char)*cp))
			cp++;
		if (*cp == '#')
			continue;

		for (tok = 0; tok < sizeof(toks)/sizeof(toks[0]); tok++) {
			len = strlen(toks[tok]);
			if (cp + len < ep &&
			    isspace((unsigned char)cp[len]) &&
			    !strncmp(cp, toks[tok], len)) {
				cp += len;
				while (isspace((unsigned char)*cp))
					cp++;
				break;
			}
		}

		switch (tok) {
		case 2:  /* _whatdb */
			while (ep > cp && ep[-1] != '/')
				ep--;
			if (ep == cp)
				continue;
			*ep = '\0';
			/* FALLTHROUGH */
		case 0:  /* manpath */
			manpath_add(&conf->manpath, cp, 0);
			*manpath_default = '\0';
			break;
		case 1:  /* output */
			manconf_output(&conf->output, cp);
			break;
		default:
			break;
		}
	}
	fclose(stream);

out:
	if (*manpath_default != '\0')
		manpath_parseline(&conf->manpath, manpath_default, 0);
}

void
manconf_output(struct manoutput *conf, const char *cp)
{
	const char *const toks[] = {
	    "includes", "man", "paper", "style",
	    "indent", "width", "fragment", "mdoc"
	};

	size_t	 len, tok;

	for (tok = 0; tok < sizeof(toks)/sizeof(toks[0]); tok++) {
		len = strlen(toks[tok]);
		if ( ! strncmp(cp, toks[tok], len) &&
		    strchr(" =	", cp[len]) != NULL) {
			cp += len;
			if (*cp == '=')
				cp++;
			while (isspace((unsigned char)*cp))
				cp++;
			break;
		}
	}

	if (tok < 6 && *cp == '\0')
		return;

	switch (tok) {
	case 0:
		if (conf->includes == NULL)
			conf->includes = mandoc_strdup(cp);
		break;
	case 1:
		if (conf->man == NULL)
			conf->man = mandoc_strdup(cp);
		break;
	case 2:
		if (conf->paper == NULL)
			conf->paper = mandoc_strdup(cp);
		break;
	case 3:
		if (conf->style == NULL)
			conf->style = mandoc_strdup(cp);
		break;
	case 4:
		if (conf->indent == 0)
			conf->indent = strtonum(cp, 0, 1000, NULL);
		break;
	case 5:
		if (conf->width == 0)
			conf->width = strtonum(cp, 58, 1000, NULL);
		break;
	case 6:
		conf->fragment = 1;
		break;
	case 7:
		conf->mdoc = 1;
		break;
	default:
		break;
	}
}
