/* $OpenBSD: cmd-find.c,v 1.1 2015/04/27 16:25:57 nicm Exp $ */

/*
 * Copyright (c) 2015 Nicholas Marriott <nicm@users.sourceforge.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#include <fnmatch.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <paths.h>

#include "tmux.h"

#define CMD_FIND_PREFER_UNATTACHED 0x1
#define CMD_FIND_QUIET 0x2
#define CMD_FIND_WINDOW_INDEX 0x4

enum cmd_find_type {
	CMD_FIND_PANE,
	CMD_FIND_WINDOW,
	CMD_FIND_SESSION,
};

struct cmd_find_state {
	struct cmd_q		*cmdq;
	int			 flags;
	struct cmd_find_state	*current;

	struct session          *s;
	struct winlink          *wl;
	struct window		*w;
	struct window_pane      *wp;
	int			 idx;
};

int		 cmd_find_client_better(struct client *, struct client *);
struct client	*cmd_find_best_client(struct client **, u_int);
int		 cmd_find_session_better(struct session *, struct session *,
		     int);
struct session	*cmd_find_best_session(struct session **, u_int, int);
int		 cmd_find_best_session_with_window(struct cmd_find_state *);
int		 cmd_find_best_winlink_with_window(struct cmd_find_state *);

int		 cmd_find_current_session_with_client(struct cmd_find_state *);
int		 cmd_find_current_session(struct cmd_find_state *);
struct client	*cmd_find_current_client(struct cmd_q *);

const char	*cmd_find_map_table(const char *[][2], const char *);

int	cmd_find_get_session(struct cmd_find_state *, const char *);
int	cmd_find_get_window(struct cmd_find_state *, const char *);
int	cmd_find_get_window_with_session(struct cmd_find_state *, const char *);
int	cmd_find_get_window_with_pane(struct cmd_find_state *);
int	cmd_find_get_pane(struct cmd_find_state *, const char *);
int	cmd_find_get_pane_with_session(struct cmd_find_state *, const char *);
int	cmd_find_get_pane_with_window(struct cmd_find_state *, const char *);

void	cmd_find_clear_state(struct cmd_find_state *, struct cmd_q *, int);
void	cmd_find_log_state(const char *, const char *, struct cmd_find_state *);

struct cmd_find_state	*cmd_find_target(struct cmd_q *, const char *,
	    enum cmd_find_type, int);

const char *cmd_find_session_table[][2] = {
	{ NULL, NULL }
};
const char *cmd_find_window_table[][2] = {
	{ "{start}", "^" },
	{ "{last}", "!" },
	{ "{end}", "$" },
	{ "{next}", "+" },
	{ "{previous}", "-" },
	{ NULL, NULL }
};
const char *cmd_find_pane_table[][2] = {
	{ "{last}", "!" },
	{ "{next}", "+" },
	{ "{previous}", "-" },
	{ "{top}", "top" },
	{ "{bottom}", "bottom" },
	{ "{left}", "left" },
	{ "{right}", "right" },
	{ "{top-left}", "top-left" },
	{ "{top-right}", "top-right" },
	{ "{bottom-left}", "bottom-left" },
	{ "{bottom-right}", "bottom-right" },
	{ "{up}", "{up}" },
	{ "{down}", "{down}" },
	{ "{left}", "{left}" },
	{ "{right}", "{right}" },
	{ NULL, NULL }
};

/* Is this client better? */
int
cmd_find_client_better(struct client *c, struct client *than)
{
	if (than == NULL)
		return (1);
	return (timercmp(&c->activity_time, &than->activity_time, >));
}

/* Find best client from a list, or all if list is NULL. */
struct client *
cmd_find_best_client(struct client **clist, u_int csize)
{
	struct client	*c_loop, *c;
	u_int		 i;

	c = NULL;
	if (clist != NULL) {
		for (i = 0; i < csize; i++) {
			if (cmd_find_client_better(clist[i], c))
				c = clist[i];
		}
	} else {
		TAILQ_FOREACH(c_loop, &clients, entry) {
			if (cmd_find_client_better(c_loop, c))
				c_loop = c;
		}
	}
	return (c);
}

/* Is this session better? */
int
cmd_find_session_better(struct session *s, struct session *than, int flags)
{
	int	attached;

	if (than == NULL)
		return (1);
	if (flags & CMD_FIND_PREFER_UNATTACHED) {
		attached = (~than->flags & SESSION_UNATTACHED);
		if (attached && (s->flags & SESSION_UNATTACHED))
			return (1);
		else if (!attached && (~s->flags & SESSION_UNATTACHED))
			return (0);
	}
	return (timercmp(&s->activity_time, &than->activity_time, >));
}

/* Find best session from a list, or all if list is NULL. */
struct session *
cmd_find_best_session(struct session **slist, u_int ssize, int flags)
{
	struct session	 *s_loop, *s;
	u_int		  i;

	s = NULL;
	if (slist != NULL) {
		for (i = 0; i < ssize; i++) {
			if (cmd_find_session_better(slist[i], s, flags))
				s = slist[i];
		}
	} else {
		RB_FOREACH(s_loop, sessions, &sessions) {
			if (cmd_find_session_better(s_loop, s, flags))
				s = s_loop;
		}
	}
	return (s);
}

/* Find best session and winlink for window. */
int
cmd_find_best_session_with_window(struct cmd_find_state *fs)
{
	struct session	**slist = NULL;
	u_int		  ssize;
	struct session	 *s;

	ssize = 0;
	RB_FOREACH(s, sessions, &sessions) {
		if (!session_has(s, fs->w))
			continue;
		slist = xreallocarray (slist, ssize + 1, sizeof *slist);
		slist[ssize++] = s;
	}
	if (ssize == 0)
		goto fail;
	fs->s = cmd_find_best_session(slist, ssize, fs->flags);
	if (fs->s == NULL)
		goto fail;
	free (slist);
	return (cmd_find_best_winlink_with_window(fs));

fail:
	free(slist);
	return (-1);
}

/*
 * Find the best winlink for a window (the current if it contains the pane,
 * otherwise the first).
 */
int
cmd_find_best_winlink_with_window(struct cmd_find_state *fs)
{
	struct winlink	 *wl, *wl_loop;

	wl = NULL;
	if (fs->s->curw->window == fs->w)
		wl = fs->s->curw;
	else {
		RB_FOREACH(wl_loop, winlinks, &fs->s->windows) {
			if (wl_loop->window == fs->w) {
				wl = wl_loop;
				break;
			}
		}
	}
	if (wl == NULL)
		return (-1);
	fs->wl = wl;
	fs->idx = fs->wl->idx;
	return (0);
}

/* Find current session when we have an unattached client. */
int
cmd_find_current_session_with_client(struct cmd_find_state *fs)
{
	struct window_pane	*wp;

	/* If this is running in a pane, that's great. */
	RB_FOREACH(wp, window_pane_tree, &all_window_panes) {
		if (strcmp(wp->tty, fs->cmdq->client->tty.path) == 0)
			break;
	}

	/* Not running in a pane. We know nothing. Find the best session. */
	if (wp == NULL) {
		fs->s = cmd_find_best_session(NULL, 0, fs->flags);
		if (fs->s == NULL)
			return (-1);
		fs->wl = fs->s->curw;
		fs->idx = fs->wl->idx;
		fs->w = fs->wl->window;
		fs->wp = fs->w->active;
		return (0);
	}

	/* We now know the window and pane. */
	fs->w = wp->window;
	fs->wp = wp;

	/* Find the best session and winlink. */
	if (cmd_find_best_session_with_window(fs) != 0)
		return (-1);
	return (0);
}

/*
 * Work out the best current state. If this function succeeds, the state is
 * guaranteed to be completely filled in.
 */
int
cmd_find_current_session(struct cmd_find_state *fs)
{
	/* If we know the current client, use it. */
	if (fs->cmdq->client != NULL) {
		if (fs->cmdq->client->session == NULL)
			return (cmd_find_current_session_with_client(fs));
		fs->s = fs->cmdq->client->session;
		fs->wl = fs->s->curw;
		fs->idx = fs->wl->idx;
		fs->w = fs->wl->window;
		fs->wp = fs->w->active;
		return (0);
	}

	/* We know nothing, find the best session and client. */
	fs->s = cmd_find_best_session(NULL, 0, fs->flags);
	if (fs->s == NULL)
		return (-1);
	fs->wl = fs->s->curw;
	fs->idx = fs->wl->idx;
	fs->w = fs->wl->window;
	fs->wp = fs->w->active;

	return (0);
}

/* Work out the best current client. */
struct client *
cmd_find_current_client(struct cmd_q *cmdq)
{
	struct cmd_find_state	 current;
	struct session		*s;
	struct client		*c, **clist = NULL;
	u_int		 	 csize;

	/* If the queue client has a session, use it. */
	if (cmdq->client != NULL && cmdq->client->session != NULL)
		return (cmdq->client);

	/* Otherwise find the current session. */
	cmd_find_clear_state(&current, cmdq, 0);
	if (cmd_find_current_session(&current) != 0)
		return (NULL);

	/* If it is attached, find the best of it's clients. */
	s = current.s;
	if (~s->flags & SESSION_UNATTACHED) {
		csize = 0;
		TAILQ_FOREACH(c, &clients, entry) {
			if (c->session != s)
				continue;
			clist = xreallocarray (clist, csize + 1, sizeof *clist);
			clist[csize++] = c;
		}
		if (csize != 0) {
			c = cmd_find_best_client(clist, csize);
			if (c != NULL) {
				free(clist);
				return (c);
			}
		}
		free(clist);
	}

	/* Otherwise pick best of all clients. */
	return (cmd_find_best_client(NULL, 0));
}

/* Maps string in table. */
const char *
cmd_find_map_table(const char *table[][2], const char *s)
{
	u_int	i;

	for (i = 0; table[i][0] != NULL; i++) {
		if (strcmp(s, table[i][0]) == 0)
			return (table[i][1]);
	}
	return (s);
}

/* Find session from string. Fills in s. */
int
cmd_find_get_session(struct cmd_find_state *fs, const char *session)
{
	struct session	*s, *s_loop;

	log_debug("%s: %s", __func__, session);

	/* Check for session ids starting with $. */
	if (*session == '$') {
		fs->s = session_find_by_id_str(session);
		if (fs->s == NULL)
			return (-1);
		return (0);
	}

	/* Look for exactly this session. */
	fs->s = session_find(session);
	if (fs->s != NULL)
		return (0);

	/* Otherwise look for prefix. */
	s = NULL;
	RB_FOREACH(s_loop, sessions, &sessions) {
		if (strncmp(session, s_loop->name, strlen(session)) == 0) {
			if (s != NULL)
				return (-1);
			s = s_loop;
		}
	}
	if (s != NULL) {
		fs->s = s;
		return (0);
	}

	/* Then as a pattern. */
	s = NULL;
	RB_FOREACH(s_loop, sessions, &sessions) {
		if (fnmatch(session, s_loop->name, 0) == 0) {
			if (s != NULL)
				return (-1);
			s = s_loop;
		}
	}
	if (s != NULL) {
		fs->s = s;
		return (0);
	}

	return (-1);
}

/* Find window from string. Fills in s, wl, w. */
int
cmd_find_get_window(struct cmd_find_state *fs, const char *window)
{
	log_debug("%s: %s", __func__, window);

	/* Check for window ids starting with @. */
	if (*window == '@') {
		fs->w = window_find_by_id_str(window);
		if (fs->w == NULL)
			return (-1);
		return (cmd_find_best_session_with_window(fs));
	}

	/* Not a window id, so use the current session. */
	fs->s = fs->current->s;

	/* We now only need to find the winlink in this session. */
	return (cmd_find_get_window_with_session(fs, window));
}

/*
 * Find window from string, assuming it is in given session. Needs s, fills in
 * wl and w.
 */
int
cmd_find_get_window_with_session(struct cmd_find_state *fs, const char *window)
{
	struct winlink	*wl;
	const char	*errstr;
	int		 idx, n;
	struct session	*s;

	log_debug("%s: %s", __func__, window);

	/* Check for window ids starting with @. */
	if (*window == '@') {
		fs->w = window_find_by_id_str(window);
		if (fs->w == NULL || !session_has(fs->s, fs->w))
			return (-1);
		return (cmd_find_best_winlink_with_window(fs));
	}

	/* Try as an offset. */
	if (window[0] == '+' || window[0] == '-') {
		if (window[1] != '\0')
			n = strtonum(window + 1, 1, INT_MAX, NULL);
		else
			n = 1;
		s = fs->s;
		if (fs->flags & CMD_FIND_WINDOW_INDEX) {
			if (window[0] == '+') {
				if (INT_MAX - s->curw->idx < n)
					return (-1);
				fs->idx = s->curw->idx + n;
			} else {
				if (n < s->curw->idx)
					return (-1);
				fs->idx = s->curw->idx - n;
			}
			return (0);
		}
		if (window[0] == '+')
			fs->wl = winlink_next_by_number(s->curw, s, n);
		else
			fs->wl = winlink_previous_by_number(s->curw, s, n);
		if (fs->wl != NULL) {
			fs->idx = fs->wl->idx;
			fs->w = fs->wl->window;
			return (0);
		}
	}

	/* Try special characters. */
	if (strcmp(window, "!") == 0) {
		fs->wl = TAILQ_FIRST(&fs->s->lastw);
		if (fs->wl == NULL)
			return (-1);
		fs->idx = fs->wl->idx;
		fs->w = fs->wl->window;
		return (0);
	} else if (strcmp(window, "^") == 0) {
		fs->wl = RB_MIN(winlinks, &fs->s->windows);
		if (fs->wl == NULL)
			return (-1);
		fs->idx = fs->wl->idx;
		fs->w = fs->wl->window;
		return (0);
	} else if (strcmp(window, "$") == 0) {
		fs->wl = RB_MAX(winlinks, &fs->s->windows);
		if (fs->wl == NULL)
			return (-1);
		fs->idx = fs->wl->idx;
		fs->w = fs->wl->window;
		return (0);
	}

	/* First see if this is a valid window index in this session. */
	idx = strtonum(window, 0, INT_MAX, &errstr);
	if (errstr == NULL) {
		if (fs->flags & CMD_FIND_WINDOW_INDEX) {
			fs->idx = idx;
			return (0);
		}
		fs->wl = winlink_find_by_index(&fs->s->windows, idx);
		if (fs->wl != NULL) {
			fs->w = fs->wl->window;
			return (0);
		}
	}

	/* Look for exact matches, error if more than one. */
	fs->wl = NULL;
	RB_FOREACH(wl, winlinks, &fs->s->windows) {
		if (strcmp(window, wl->window->name) == 0) {
			if (fs->wl != NULL)
				return (-1);
			fs->wl = wl;
		}
	}
	if (fs->wl != NULL) {
		fs->idx = fs->wl->idx;
		fs->w = fs->wl->window;
		return (0);
	}

	/* Try as the start of a window name, error if multiple. */
	fs->wl = NULL;
	RB_FOREACH(wl, winlinks, &fs->s->windows) {
		if (strncmp(window, wl->window->name, strlen(window)) == 0) {
			if (fs->wl != NULL)
				return (-1);
			fs->wl = wl;
		}
	}
	if (fs->wl != NULL) {
		fs->idx = fs->wl->idx;
		fs->w = fs->wl->window;
		return (0);
	}

	/* Now look for pattern matches, again error if multiple. */
	fs->wl = NULL;
	RB_FOREACH(wl, winlinks, &fs->s->windows) {
		if (fnmatch(window, wl->window->name, 0) == 0) {
			if (fs->wl != NULL)
				return (-1);
			fs->wl = wl;
		}
	}
	if (fs->wl != NULL) {
		fs->idx = fs->wl->idx;
		fs->w = fs->wl->window;
		return (0);
	}

	return (-1);
}

/* Find window from given pane. Needs wp, fills in s and wl and w. */
int
cmd_find_get_window_with_pane(struct cmd_find_state *fs)
{
	log_debug("%s", __func__);

	fs->w = fs->wp->window;
	return (cmd_find_best_session_with_window(fs));
}

/* Find pane from string. Fills in s, wl, w, wp. */
int
cmd_find_get_pane(struct cmd_find_state *fs, const char *pane)
{
	log_debug("%s: %s", __func__, pane);

	/* Check for pane ids starting with %. */
	if (*pane == '%') {
		fs->wp = window_pane_find_by_id_str(pane);
		if (fs->wp == NULL)
			return (-1);
		fs->w = fs->wp->window;
		return (cmd_find_best_session_with_window(fs));
	}

	/* Not a pane id, so use the current session and window. */
	fs->s = fs->current->s;
	fs->wl = fs->current->wl;
	fs->idx = fs->current->idx;
	fs->w = fs->current->w;

	/* We now only need to find the pane in this window. */
	return (cmd_find_get_pane_with_window(fs, pane));
}

/*
 * Find pane from string, assuming it is in given session. Needs s, fills in wl
 * and w and wp.
 */
int
cmd_find_get_pane_with_session(struct cmd_find_state *fs, const char *pane)
{
	log_debug("%s: %s", __func__, pane);

	/* Check for pane ids starting with %. */
	if (*pane == '%') {
		fs->wp = window_pane_find_by_id_str(pane);
		if (fs->wp == NULL)
			return (-1);
		fs->w = fs->wp->window;
		return (cmd_find_best_winlink_with_window(fs));
	}

	/* Otherwise use the current window. */
	fs->wl = fs->s->curw;
	fs->idx = fs->wl->idx;
	fs->w = fs->wl->window;

	/* Now we just need to look up the pane. */
	return (cmd_find_get_pane_with_window(fs, pane));
}

/*
 * Find pane from string, assuming it is in the given window. Needs w, fills in
 * wp.
 */
int
cmd_find_get_pane_with_window(struct cmd_find_state *fs, const char *pane)
{
	const char		*errstr;
	int			 idx;
	struct window_pane	*wp;
	u_int			 n;

	log_debug("%s: %s", __func__, pane);

	/* Check for pane ids starting with %. */
	if (*pane == '%') {
		fs->wp = window_pane_find_by_id_str(pane);
		if (fs->wp == NULL || fs->wp->window != fs->w)
			return (-1);
		return (0);
	}

	/* Try special characters. */
	if (strcmp(pane, "!") == 0) {
		if (fs->w->last == NULL)
			return (-1);
		fs->wp = fs->w->last;
		return (0);
	} else if (strcmp(pane, "{up}") == 0) {
		fs->wp = window_pane_find_up(fs->w->active);
		if (fs->wp == NULL)
			return (-1);
		return (0);
	} else if (strcmp(pane, "{down}") == 0) {
		fs->wp = window_pane_find_down(fs->w->active);
		if (fs->wp == NULL)
			return (-1);
		return (0);
	} else if (strcmp(pane, "{left}") == 0) {
		fs->wp = window_pane_find_left(fs->w->active);
		if (fs->wp == NULL)
			return (-1);
		return (0);
	} else if (strcmp(pane, "{right}") == 0) {
		fs->wp = window_pane_find_right(fs->w->active);
		if (fs->wp == NULL)
			return (-1);
		return (0);
	}

	/* Try as an offset. */
	if (pane[0] == '+' || pane[0] == '-') {
		if (pane[1] != '\0')
			n = strtonum(pane + 1, 1, INT_MAX, NULL);
		else
			n = 1;
		wp = fs->w->active;
		if (pane[0] == '+')
			fs->wp = window_pane_next_by_number(fs->w, wp, n);
		else
			fs->wp = window_pane_previous_by_number(fs->w, wp, n);
		if (fs->wp != NULL)
			return (0);
	}

	/* Get pane by index. */
	idx = strtonum(pane, 0, INT_MAX, &errstr);
	if (errstr == NULL) {
		fs->wp = window_pane_at_index(fs->w, idx);
		if (fs->wp != NULL)
			return (0);
	}

	/* Try as a description. */
	fs->wp = window_find_string(fs->w, pane);
	if (fs->wp != NULL)
		return (0);

	return (-1);
}

/* Clear state. */
void
cmd_find_clear_state(struct cmd_find_state *fs, struct cmd_q *cmdq, int flags)
{
	memset (fs, 0, sizeof *fs);

	fs->cmdq = cmdq;
	fs->flags = flags;

	fs->idx = -1;
}

/* Split target into pieces and resolve for the given type. */
struct cmd_find_state *
cmd_find_target(struct cmd_q *cmdq, const char *target, enum cmd_find_type type,
    int flags)
{
	static struct cmd_find_state	 fs, current;
	struct mouse_event		*m;
	char				*colon, *period, *copy = NULL;
	const char			*session, *window, *pane;

	/* Find current state. */
	cmd_find_clear_state(&current, cmdq, flags);
	if (cmd_find_current_session(&current) != 0) {
		if (~flags & CMD_FIND_QUIET)
			cmdq_error(cmdq, "no current session");
		goto error;
	}

	/* Clear new state. */
	cmd_find_clear_state(&fs, cmdq, flags);
	fs.current = &current;

	/* An empty or NULL target is the current. */
	if (target == NULL || *target == '\0')
		goto current;

	/* Mouse target is a plain = or {mouse}. */
	if (strcmp(target, "=") == 0 || strcmp(target, "{mouse}") == 0) {
		m = &cmdq->item->mouse;
		switch (type) {
		case CMD_FIND_PANE:
			fs.wp = cmd_mouse_pane(m, &fs.s, &fs.wl);
			if (fs.wp != NULL)
				fs.w = fs.wl->window;
			break;
		case CMD_FIND_WINDOW:
		case CMD_FIND_SESSION:
			fs.wl = cmd_mouse_window(m, &fs.s);
			if (fs.wl != NULL) {
				fs.w = fs.wl->window;
				fs.wp = fs.w->active;
			}
			break;
		}
		if (fs.wp == NULL) {
			if (~flags & CMD_FIND_QUIET)
				cmdq_error(cmdq, "no mouse target");
			goto error;
		}
		return (&fs);
	}
	copy = xstrdup(target);

	/* Find separators if they exist. */
	colon = strchr(copy, ':');
	if (colon != NULL)
		*colon++ = '\0';
	if (colon == NULL)
		period = strchr(copy, '.');
	else
		period = strchr(colon, '.');
	if (period != NULL)
		*period++ = '\0';

	/* Set session, window and pane parts. */
	session = window = pane = NULL;
	if (colon != NULL && period != NULL) {
		session = copy;
		window = colon;
		pane = period;
	} else if (colon != NULL && period == NULL) {
		session = copy;
		window = colon;
	} else if (colon == NULL && period != NULL) {
		window = copy;
		pane = period;
	} else {
		if (*copy == '$')
			session = copy;
		else if (*copy == '@')
			window = copy;
		else if (*copy == '%')
			pane = copy;
		else {
			switch (type) {
			case CMD_FIND_SESSION:
				session = copy;
				break;
			case CMD_FIND_WINDOW:
				window = copy;
				break;
			case CMD_FIND_PANE:
				pane = copy;
				break;
			}
		}
	}

	/* Empty is the same as NULL. */
	if (session != NULL && *session == '\0')
		session = NULL;
	if (window != NULL && *window == '\0')
		window = NULL;
	if (pane != NULL && *pane == '\0')
		pane = NULL;

	/* Map though conversion table. */
	if (session != NULL)
		session = cmd_find_map_table(cmd_find_session_table, session);
	if (window != NULL)
		window = cmd_find_map_table(cmd_find_window_table, window);
	if (pane != NULL)
		pane = cmd_find_map_table(cmd_find_pane_table, pane);

	log_debug("target %s (flags %#x): session=%s, window=%s, pane=%s",
	    target, flags, session == NULL ? "none" : session,
	    window == NULL ? "none" : window, pane == NULL ? "none" : pane);

	/* No pane is allowed if want an index. */
	if (pane != NULL && (flags & CMD_FIND_WINDOW_INDEX)) {
		if (~flags & CMD_FIND_QUIET)
			cmdq_error(cmdq, "can't specify pane here");
		goto error;
	}

	/* If the session isn't NULL, look it up. */
	if (session != NULL) {
		/* This will fill in session. */
		if (cmd_find_get_session(&fs, session) != 0)
			goto no_session;

		/* If window and pane are NULL, use that session's current. */
		if (window == NULL && pane == NULL) {
			fs.wl = fs.s->curw;
			fs.idx = -1;
			fs.w = fs.wl->window;
			fs.wp = fs.w->active;
			goto found;
		}

		/* If window is present but pane not, find window in session. */
		if (window != NULL && pane == NULL) {
			/* This will fill in winlink and window. */
			if (cmd_find_get_window_with_session(&fs, window) != 0)
				goto no_window;
			if (~flags & CMD_FIND_WINDOW_INDEX)
				fs.wp = fs.wl->window->active;
			goto found;
		}

		/* If pane is present but window not, find pane. */
		if (window == NULL && pane != NULL) {
			/* This will fill in winlink and window and pane. */
			if (cmd_find_get_pane_with_session(&fs, pane) != 0)
				goto no_pane;
			goto found;
		}

		/*
		 * If window and pane are present, find both in session. This
		 * will fill in winlink and window.
		 */
		if (cmd_find_get_window_with_session(&fs, window) != 0)
			goto no_window;
		/* This will fill in pane. */
		if (cmd_find_get_pane_with_window(&fs, pane) != 0)
			goto no_pane;
		goto found;
	}

	/* No session. If window and pane, try them. */
	if (window != NULL && pane != NULL) {
		/* This will fill in session, winlink and window. */
		if (cmd_find_get_window(&fs, window) != 0)
			goto no_window;
		/* This will fill in pane. */
		if (cmd_find_get_pane_with_window(&fs, pane) != 0)
			goto no_pane;
		goto found;
	}

	/* If just window is present, try it. */
	if (window != NULL && pane == NULL) {
		/* This will fill in session, winlink and window. */
		if (cmd_find_get_window(&fs, window) != 0)
			goto no_window;
		if (~flags & CMD_FIND_WINDOW_INDEX)
			fs.wp = fs.wl->window->active;
		goto found;
	}

	/* If just pane is present, try it. */
	if (window == NULL && pane != NULL) {
		/* This will fill in session, winlink, window and pane. */
		if (cmd_find_get_pane(&fs, pane) != 0)
			goto no_pane;
		goto found;
	}

current:
	/* None is the current session. */
	free(copy);
	if (flags & CMD_FIND_WINDOW_INDEX)
		current.idx = -1;
	return (&current);

error:
	free(copy);
	return (NULL);

found:
	free(copy);
	return (&fs);

no_session:
	if (~flags & CMD_FIND_QUIET)
		cmdq_error(cmdq, "can't find session %s", session);
	goto error;

no_window:
	if (~flags & CMD_FIND_QUIET)
		cmdq_error(cmdq, "can't find window %s", window);
	goto error;

no_pane:
	if (~flags & CMD_FIND_QUIET)
		cmdq_error(cmdq, "can't find pane %s", pane);
	goto error;
}

/* Log the result. */
void
cmd_find_log_state(const char *f, const char *target, struct cmd_find_state *fs)
{
	log_debug("%s: target %s%s", f, target == NULL ? "none" : target,
	    fs != NULL ? "" : " (failed)");
	if (fs == NULL)
		return;
	if (fs->s != NULL)
		log_debug("\ts=$%u", fs->s->id);
	else
		log_debug("\ts=none");
	if (fs->wl != NULL) {
		log_debug("\twl=%u %d w=@%u %s", fs->wl->idx,
		    fs->wl->window == fs->w, fs->w->id, fs->w->name);
	} else
		log_debug("\twl=none");
	if (fs->wp != NULL)
		log_debug("\twp=%%%u", fs->wp->id);
	else
		log_debug("\twp=none");
	if (fs->idx != -1)
		log_debug("\tidx=%d", fs->idx);
	else
		log_debug("\tidx=none");
}

/* Find the current session. */
struct session *
cmd_find_current(struct cmd_q *cmdq)
{
	struct cmd_find_state	*fs;
	int			 flags = CMD_FIND_QUIET;

	fs = cmd_find_target(cmdq, NULL, CMD_FIND_SESSION, flags);
	cmd_find_log_state(__func__, NULL, fs);
	if (fs == NULL)
		return (NULL);

	return (fs->s);
}

/* Find the target session or report an error and return NULL. */
struct session *
cmd_find_session(struct cmd_q *cmdq, const char *target, int prefer_unattached)
{
	struct cmd_find_state	*fs;
	int			 flags = 0;

	if (prefer_unattached)
		flags |= CMD_FIND_PREFER_UNATTACHED;

	fs = cmd_find_target(cmdq, target, CMD_FIND_SESSION, flags);
	cmd_find_log_state(__func__, target, fs);
	if (fs == NULL)
		return (NULL);

	return (fs->s);
}

/* Find the target window or report an error and return NULL. */
struct winlink *
cmd_find_window(struct cmd_q *cmdq, const char *target, struct session **sp)
{
	struct cmd_find_state	*fs;

	fs = cmd_find_target(cmdq, target, CMD_FIND_WINDOW, 0);
	cmd_find_log_state(__func__, target, fs);
	if (fs == NULL)
		return (NULL);

	if (sp != NULL)
		*sp = fs->s;
	return (fs->wl);
}

/* Find the target pane and report an error and return NULL. */
struct winlink *
cmd_find_pane(struct cmd_q *cmdq, const char *target, struct session **sp,
    struct window_pane **wpp)
{
	struct cmd_find_state	*fs;

	fs = cmd_find_target(cmdq, target, CMD_FIND_PANE, 0);
	cmd_find_log_state(__func__, target, fs);
	if (fs == NULL)
		return (NULL);

	if (sp != NULL)
		*sp = fs->s;
	if (wpp != NULL)
		*wpp = fs->wp;
	return (fs->wl);
}

/* Find the target client or report an error and return NULL. */
struct client *
cmd_find_client(struct cmd_q *cmdq, const char *target, int quiet)
{
	struct client	*c;
	char		*copy;
	size_t		 size;
	const char	*path;

	/* A NULL argument means the current client. */
	if (target == NULL) {
		c = cmd_find_current_client(cmdq);
		if (c == NULL && !quiet)
			cmdq_error(cmdq, "no current client");
		return (c);
	}
	copy = xstrdup(target);

	/* Trim a single trailing colon if any. */
	size = strlen(copy);
	if (size != 0 && copy[size - 1] == ':')
		copy[size - 1] = '\0';

	/* Check path of each client. */
	TAILQ_FOREACH(c, &clients, entry) {
		if (c->session == NULL || c->tty.path == NULL)
			continue;
		path = c->tty.path;

		/* Try for exact match. */
		if (strcmp(copy, path) == 0)
			break;

		/* Try without leading /dev. */
		if (strncmp(path, _PATH_DEV, (sizeof _PATH_DEV) - 1) != 0)
			continue;
		if (strcmp(copy, path + (sizeof _PATH_DEV) - 1) == 0)
			break;
	}

	/* If no client found, report an error. */
	if (c == NULL && !quiet)
		cmdq_error(cmdq, "can't find client %s", copy);

	free(copy);
	return (c);
}

/*
 * Find the target session and window index, whether or not it exists in the
 * session. Return -2 on error or -1 if no window index is specified. This is
 * used when parsing an argument for a window target that may not exist (for
 * example if it is going to be created).
 */
int
cmd_find_index(struct cmd_q *cmdq, const char *target, struct session **sp)
{
	struct cmd_find_state	*fs;
	int			 flags = CMD_FIND_WINDOW_INDEX;

	fs = cmd_find_target(cmdq, target, CMD_FIND_WINDOW, flags);
	cmd_find_log_state(__func__, target, fs);
	if (fs == NULL)
		return (-2);

	if (sp != NULL)
		*sp = fs->s;
	return (fs->idx);
}
