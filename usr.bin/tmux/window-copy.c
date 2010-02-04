/* $OpenBSD: window-copy.c,v 1.41 2010/02/04 20:00:26 nicm Exp $ */

/*
 * Copyright (c) 2007 Nicholas Marriott <nicm@users.sourceforge.net>
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

#include <stdlib.h>
#include <string.h>

#include "tmux.h"

struct screen *window_copy_init(struct window_pane *);
void	window_copy_free(struct window_pane *);
void	window_copy_resize(struct window_pane *, u_int, u_int);
void	window_copy_key(struct window_pane *, struct client *, int);
int	window_copy_key_input(struct window_pane *, int);
void	window_copy_mouse(
	    struct window_pane *, struct client *, struct mouse_event *);

void	window_copy_redraw_lines(struct window_pane *, u_int, u_int);
void	window_copy_redraw_screen(struct window_pane *);
void	window_copy_write_line(
	    struct window_pane *, struct screen_write_ctx *, u_int);
void	window_copy_write_lines(
	    struct window_pane *, struct screen_write_ctx *, u_int, u_int);

void	window_copy_scroll_to(struct window_pane *, u_int, u_int);
int	window_copy_search_compare(
	    struct grid *, u_int, u_int, struct grid *, u_int);
int	window_copy_search_lr(
	    struct grid *, struct grid *, u_int *, u_int, u_int, u_int);
int	window_copy_search_rl(
	    struct grid *, struct grid *, u_int *, u_int, u_int, u_int);
void	window_copy_search_up(struct window_pane *, const char *);
void	window_copy_search_down(struct window_pane *, const char *);
void	window_copy_goto_line(struct window_pane *, const char *);
void	window_copy_update_cursor(struct window_pane *, u_int, u_int);
void	window_copy_start_selection(struct window_pane *);
int	window_copy_update_selection(struct window_pane *);
void	window_copy_copy_selection(struct window_pane *, struct client *);
void	window_copy_copy_line(
	    struct window_pane *, char **, size_t *, u_int, u_int, u_int);
int	window_copy_in_set(struct window_pane *, u_int, u_int, const char *);
u_int	window_copy_find_length(struct window_pane *, u_int);
void	window_copy_cursor_start_of_line(struct window_pane *);
void	window_copy_cursor_back_to_indentation(struct window_pane *);
void	window_copy_cursor_end_of_line(struct window_pane *);
void	window_copy_cursor_left(struct window_pane *);
void	window_copy_cursor_right(struct window_pane *);
void	window_copy_cursor_up(struct window_pane *, int);
void	window_copy_cursor_down(struct window_pane *, int);
void	window_copy_cursor_next_word(struct window_pane *, const char *);
void	window_copy_cursor_next_word_end(struct window_pane *, const char *);
void	window_copy_cursor_previous_word(struct window_pane *, const char *);
void	window_copy_scroll_up(struct window_pane *, u_int);
void	window_copy_scroll_down(struct window_pane *, u_int);

const struct window_mode window_copy_mode = {
	window_copy_init,
	window_copy_free,
	window_copy_resize,
	window_copy_key,
	window_copy_mouse,
	NULL,
};

enum window_copy_input_type {
	WINDOW_COPY_OFF,
	WINDOW_COPY_SEARCHUP,
	WINDOW_COPY_SEARCHDOWN,
	WINDOW_COPY_GOTOLINE,
};

struct window_copy_mode_data {
	struct screen	screen;

	struct mode_key_data mdata;

	u_int		oy;

	u_int		selx;
	u_int		sely;

	u_int		cx;
	u_int		cy;

	u_int		lastcx; /* position in last line with content */
	u_int		lastsx; /* size of last line with content */

	enum window_copy_input_type inputtype;
	const char     *inputprompt;
	char   	       *inputstr;

	enum window_copy_input_type searchtype;
	char	       *searchstr;
};

struct screen *
window_copy_init(struct window_pane *wp)
{
	struct window_copy_mode_data	*data;
	struct screen			*s;
	struct screen_write_ctx	 	 ctx;
	u_int				 i;
	int				 keys;

	wp->modedata = data = xmalloc(sizeof *data);
	data->oy = 0;
	data->cx = wp->base.cx;
	data->cy = wp->base.cy;

	data->lastcx = 0;
	data->lastsx = 0;

	data->inputtype = WINDOW_COPY_OFF;
	data->inputprompt = NULL;
	data->inputstr = xstrdup("");

	data->searchtype = WINDOW_COPY_OFF;
	data->searchstr = NULL;

	s = &data->screen;
	screen_init(s, screen_size_x(&wp->base), screen_size_y(&wp->base), 0);
	if (options_get_number(&wp->window->options, "mode-mouse"))
		s->mode |= MODE_MOUSE;

	keys = options_get_number(&wp->window->options, "mode-keys");
	if (keys == MODEKEY_EMACS)
		mode_key_init(&data->mdata, &mode_key_tree_emacs_copy);
	else
		mode_key_init(&data->mdata, &mode_key_tree_vi_copy);

	s->cx = data->cx;
	s->cy = data->cy;

	screen_write_start(&ctx, NULL, s);
	for (i = 0; i < screen_size_y(s); i++)
		window_copy_write_line(wp, &ctx, i);
	screen_write_cursormove(&ctx, data->cx, data->cy);
	screen_write_stop(&ctx);

	return (s);
}

void
window_copy_free(struct window_pane *wp)
{
	struct window_copy_mode_data	*data = wp->modedata;

	if (data->searchstr != NULL)
		xfree(data->searchstr);
	xfree(data->inputstr);

	screen_free(&data->screen);

	xfree(data);
}

void
window_copy_pageup(struct window_pane *wp)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;
	u_int				 n;

	n = 1;
	if (screen_size_y(s) > 2)
		n = screen_size_y(s) - 2;
	if (data->oy + n > screen_hsize(&wp->base))
		data->oy = screen_hsize(&wp->base);
	else
		data->oy += n;
	window_copy_update_selection(wp);
	window_copy_redraw_screen(wp);
}

void
window_copy_resize(struct window_pane *wp, u_int sx, u_int sy)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;
	struct screen_write_ctx	 	 ctx;

	screen_resize(s, sx, sy);

	if (data->cy > sy - 1)
		data->cy = sy - 1;
	if (data->cx > sx)
		data->cx = sx;

	screen_clear_selection(&data->screen);

	screen_write_start(&ctx, NULL, s);
	window_copy_write_lines(wp, &ctx, 0, screen_size_y(s) - 1);
	screen_write_stop(&ctx);

	window_copy_redraw_screen(wp);
}

void
window_copy_key(struct window_pane *wp, struct client *c, int key)
{
	const char			*word_separators = " -_@";
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;
	u_int				 n;
	int				 keys;

	if (data->inputtype != WINDOW_COPY_OFF) {
		if (window_copy_key_input(wp, key) != 0)
			goto input_off;
		return;
	}

	switch (mode_key_lookup(&data->mdata, key)) {
	case MODEKEYCOPY_CANCEL:
		window_pane_reset_mode(wp);
		break;
	case MODEKEYCOPY_LEFT:
		window_copy_cursor_left(wp);
		return;
	case MODEKEYCOPY_RIGHT:
		window_copy_cursor_right(wp);
		return;
	case MODEKEYCOPY_UP:
		window_copy_cursor_up(wp, 0);
		return;
	case MODEKEYCOPY_DOWN:
		window_copy_cursor_down(wp, 0);
		return;
	case MODEKEYCOPY_SCROLLUP:
		window_copy_cursor_up(wp, 1);
		break;
	case MODEKEYCOPY_SCROLLDOWN:
		window_copy_cursor_down(wp, 1);
		break;
	case MODEKEYCOPY_PREVIOUSPAGE:
		window_copy_pageup(wp);
		break;
	case MODEKEYCOPY_NEXTPAGE:
		n = 1;
		if (screen_size_y(s) > 2)
			n = screen_size_y(s) - 2;
		if (data->oy < n)
			data->oy = 0;
		else
			data->oy -= n;
		window_copy_update_selection(wp);
		window_copy_redraw_screen(wp);
		break;
	case MODEKEYCOPY_HALFPAGEUP:
		n = screen_size_y(s) / 2;
		if (data->oy + n > screen_hsize(&wp->base))
			data->oy = screen_hsize(&wp->base);
		else
			data->oy += n;
		window_copy_update_selection(wp);
		window_copy_redraw_screen(wp);
		break;
	case MODEKEYCOPY_HALFPAGEDOWN:
		n = screen_size_y(s) / 2;
		if (data->oy < n)
			data->oy = 0;
		else
			data->oy -= n;
		window_copy_update_selection(wp);
		window_copy_redraw_screen(wp);
		break;
	case MODEKEYCOPY_TOPLINE:
		data->cx = 0;
		data->cy = 0;
		window_copy_update_selection(wp);
		window_copy_redraw_screen(wp);
		break;
	case MODEKEYCOPY_MIDDLELINE:
		data->cx = 0;
		data->cy = (screen_size_y(s) - 1) / 2;
		window_copy_update_selection(wp);
		window_copy_redraw_screen(wp);
		break;
	case MODEKEYCOPY_BOTTOMLINE:
		data->cx = 0;
		data->cy = screen_size_y(s) - 1;
		window_copy_update_selection(wp);
		window_copy_redraw_screen(wp);
		break;
	case MODEKEYCOPY_HISTORYTOP:
		data->cx = 0;
		data->cy = 0;
		data->oy = screen_hsize(&wp->base);
		window_copy_update_selection(wp);
		window_copy_redraw_screen(wp);
		break;
	case MODEKEYCOPY_HISTORYBOTTOM:
		data->cx = 0;
		data->cy = screen_size_y(s) - 1;
		data->oy = 0;
		window_copy_update_selection(wp);
		window_copy_redraw_screen(wp);
		break;
	case MODEKEYCOPY_STARTSELECTION:
		window_copy_start_selection(wp);
		window_copy_redraw_screen(wp);
		break;
	case MODEKEYCOPY_CLEARSELECTION:
		screen_clear_selection(&data->screen);
		window_copy_redraw_screen(wp);
		break;
	case MODEKEYCOPY_COPYSELECTION:
		if (c != NULL && c->session != NULL) {
			window_copy_copy_selection(wp, c);
			window_pane_reset_mode(wp);
		}
		break;
	case MODEKEYCOPY_STARTOFLINE:
		window_copy_cursor_start_of_line(wp);
		break;
	case MODEKEYCOPY_BACKTOINDENTATION:
		window_copy_cursor_back_to_indentation(wp);
		break;
	case MODEKEYCOPY_ENDOFLINE:
		window_copy_cursor_end_of_line(wp);
		break;
	case MODEKEYCOPY_NEXTSPACE:
		window_copy_cursor_next_word(wp, " ");
		break;
	case MODEKEYCOPY_NEXTSPACEEND:
		window_copy_cursor_next_word_end(wp, " ");
		break;
	case MODEKEYCOPY_NEXTWORD:
		window_copy_cursor_next_word(wp, word_separators);
		break;
	case MODEKEYCOPY_NEXTWORDEND:
		window_copy_cursor_next_word_end(wp, word_separators);
		break;
	case MODEKEYCOPY_PREVIOUSSPACE:
		window_copy_cursor_previous_word(wp, " ");
		break;
	case MODEKEYCOPY_PREVIOUSWORD:
		window_copy_cursor_previous_word(wp, word_separators);
		break;
	case MODEKEYCOPY_SEARCHUP:
		data->inputtype = WINDOW_COPY_SEARCHUP;
		data->inputprompt = "Search Up";
		goto input_on;
	case MODEKEYCOPY_SEARCHDOWN:
		data->inputtype = WINDOW_COPY_SEARCHDOWN;
		data->inputprompt = "Search Down";
		goto input_on;
	case MODEKEYCOPY_SEARCHAGAIN:
		switch (data->searchtype) {
		case WINDOW_COPY_OFF:
		case WINDOW_COPY_GOTOLINE:
			break;
		case WINDOW_COPY_SEARCHUP:
			window_copy_search_up(wp, data->searchstr);
			break;
		case WINDOW_COPY_SEARCHDOWN:
			window_copy_search_down(wp, data->searchstr);
			break;
		}
		break;
	case MODEKEYCOPY_GOTOLINE:
		data->inputtype = WINDOW_COPY_GOTOLINE;
		data->inputprompt = "Goto Line";
		*data->inputstr = '\0';
		goto input_on;
	default:
		break;
	}

	return;

input_on:
	keys = options_get_number(&wp->window->options, "mode-keys");
	if (keys == MODEKEY_EMACS)
		mode_key_init(&data->mdata, &mode_key_tree_emacs_edit);
	else
		mode_key_init(&data->mdata, &mode_key_tree_vi_edit);

	window_copy_redraw_lines(wp, screen_size_y(s) - 1, 1);
	return;

input_off:
	keys = options_get_number(&wp->window->options, "mode-keys");
	if (keys == MODEKEY_EMACS)
		mode_key_init(&data->mdata, &mode_key_tree_emacs_copy);
	else
		mode_key_init(&data->mdata, &mode_key_tree_vi_copy);

	data->inputtype = WINDOW_COPY_OFF;
	data->inputprompt = NULL;

	window_copy_redraw_lines(wp, screen_size_y(s) - 1, 1);
}

int
window_copy_key_input(struct window_pane *wp, int key)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;
	size_t				 inputlen;

	switch (mode_key_lookup(&data->mdata, key)) {
	case MODEKEYEDIT_CANCEL:
		return (-1);
	case MODEKEYEDIT_BACKSPACE:
		inputlen = strlen(data->inputstr);
		if (inputlen > 0)
			data->inputstr[inputlen - 1] = '\0';
		break;
	case MODEKEYEDIT_DELETELINE:
		*data->inputstr = '\0';
		break;
	case MODEKEYEDIT_ENTER:
		switch (data->inputtype) {
		case WINDOW_COPY_OFF:
			break;
		case WINDOW_COPY_SEARCHUP:
			window_copy_search_up(wp, data->inputstr);
			data->searchtype = data->inputtype;
			data->searchstr = xstrdup(data->inputstr);
			break;
		case WINDOW_COPY_SEARCHDOWN:
			window_copy_search_down(wp, data->inputstr);
			data->searchtype = data->inputtype;
			data->searchstr = xstrdup(data->inputstr);
			break;
		case WINDOW_COPY_GOTOLINE:
			window_copy_goto_line(wp, data->inputstr);
			*data->inputstr = '\0';
			break;
		}
		return (1);
	case MODEKEY_OTHER:
		if (key < 32 || key > 126)
			break;
		inputlen = strlen(data->inputstr) + 2;

		data->inputstr = xrealloc(data->inputstr, 1, inputlen);
		data->inputstr[inputlen - 2] = key;
		data->inputstr[inputlen - 1] = '\0';
		break;
	default:
		break;
	}

	window_copy_redraw_lines(wp, screen_size_y(s) - 1, 1);
	return (0);
}

/* ARGSUSED */
void
window_copy_mouse(
    struct window_pane *wp, unused struct client *c, struct mouse_event *m)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;

	if ((m->b & 3) == 3)
		return;
	if (m->x >= screen_size_x(s))
		return;
	if (m->y >= screen_size_y(s))
		return;

	window_copy_update_cursor(wp, m->x, m->y);
	if (window_copy_update_selection(wp))
		window_copy_redraw_screen(wp);
}

void
window_copy_scroll_to(struct window_pane *wp, u_int px, u_int py)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &wp->base;
	struct grid			*gd = s->grid;
	u_int				 offset, gap;

	data->cx = px;

	gap = gd->sy / 4;
	if (py < gd->sy) {
		offset = 0;
		data->cy = py;
	} else if (py > gd->hsize + gd->sy - gap) {
		offset = gd->hsize;
		data->cy = py - gd->hsize;
	} else {
		offset = py + gap - gd->sy;
		data->cy = py - offset;
	}
	data->oy = gd->hsize - offset;

	window_copy_update_selection(wp);
	window_copy_redraw_screen(wp);
}

int
window_copy_search_compare(
    struct grid *gd, u_int px, u_int py, struct grid *sgd, u_int spx)
{
	const struct grid_cell	*gc, *sgc;
	const struct grid_utf8	*gu, *sgu;

	gc = grid_peek_cell(gd, px, py);
	sgc = grid_peek_cell(sgd, spx, 0);

	if ((gc->flags & GRID_FLAG_UTF8) != (sgc->flags & GRID_FLAG_UTF8))
		return (0);

	if (gc->flags & GRID_FLAG_UTF8) {
		gu = grid_peek_utf8(gd, px, py);
		sgu = grid_peek_utf8(sgd, spx, 0);
		if (grid_utf8_compare(gu, sgu))
			return (1);
	} else {
		if (gc->data == sgc->data)
			return (1);
	}
	return (0);
}

int
window_copy_search_lr(struct grid *gd,
    struct grid *sgd, u_int *ppx, u_int py, u_int first, u_int last)
{
	u_int	ax, bx, px;

	for (ax = first; ax < last; ax++) {
		if (ax + sgd->sx >= gd->sx)
			break;
		for (bx = 0; bx < sgd->sx; bx++) {
			px = ax + bx;
			if (!window_copy_search_compare(gd, px, py, sgd, bx))
				break;
		}
		if (bx == sgd->sx) {
			*ppx = ax;
			return (1);
		}
	}
	return (0);
}

int
window_copy_search_rl(struct grid *gd,
    struct grid *sgd, u_int *ppx, u_int py, u_int first, u_int last)
{
	u_int	ax, bx, px;

	for (ax = last + 1; ax > first; ax--) {
		if (gd->sx - (ax - 1) < sgd->sx)
			continue;
		for (bx = 0; bx < sgd->sx; bx++) {
			px = ax - 1 + bx;
			if (!window_copy_search_compare(gd, px, py, sgd, bx))
				break;
		}
		if (bx == sgd->sx) {
			*ppx = ax - 1;
			return (1);
		}
	}
	return (0);
}

void
window_copy_search_up(struct window_pane *wp, const char *searchstr)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &wp->base, ss;
	struct screen_write_ctx		 ctx;
	struct grid			*gd = s->grid, *sgd;
	struct grid_cell	 	 gc;
	size_t				 searchlen;
	u_int				 i, last, fx, fy, px;
	int				 utf8flag, n, wrapped;

	if (*searchstr == '\0')
		return;
	utf8flag = options_get_number(&wp->window->options, "utf8");
	searchlen = screen_write_strlen(utf8flag, "%s", searchstr);

	screen_init(&ss, searchlen, 1, 0);
	screen_write_start(&ctx, NULL, &ss);
	memcpy(&gc, &grid_default_cell, sizeof gc);
	screen_write_nputs(&ctx, -1, &gc, utf8flag, "%s", searchstr);
	screen_write_stop(&ctx);

	fx = data->cx;
	fy = gd->hsize - data->oy + data->cy;

	if (fx == 0) {
		if (fy == 0)
			return;
		fx = gd->sx - 1;
		fy--;
	} else
		fx--;
	n = wrapped = 0;

retry:
	sgd = ss.grid;
	for (i = fy + 1; i > 0; i--) {
		last = screen_size_x(s);
		if (i == fy + 1)
			last = fx;
		n = window_copy_search_rl(gd, sgd, &px, i - 1, 0, last);
		if (n) {
			window_copy_scroll_to(wp, px, i - 1);
			break;
		}
	}
	if (!n && !wrapped) {
		fx = gd->sx - 1;
		fy = gd->hsize + gd->sy - 1;
		wrapped = 1;
		goto retry;
	}

	screen_free(&ss);
}

void
window_copy_search_down(struct window_pane *wp, const char *searchstr)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &wp->base, ss;
	struct screen_write_ctx		 ctx;
	struct grid			*gd = s->grid, *sgd;
	struct grid_cell	 	 gc;
	size_t				 searchlen;
	u_int				 i, first, fx, fy, px;
	int				 utf8flag, n, wrapped;

	if (*searchstr == '\0')
		return;
	utf8flag = options_get_number(&wp->window->options, "utf8");
	searchlen = screen_write_strlen(utf8flag, "%s", searchstr);

	screen_init(&ss, searchlen, 1, 0);
	screen_write_start(&ctx, NULL, &ss);
	memcpy(&gc, &grid_default_cell, sizeof gc);
	screen_write_nputs(&ctx, -1, &gc, utf8flag, "%s", searchstr);
	screen_write_stop(&ctx);

	fx = data->cx;
	fy = gd->hsize - data->oy + data->cy;

	if (fx == gd->sx - 1) {
		if (fy == gd->hsize + gd->sy)
			return;
		fx = 0;
		fy++;
	} else
		fx++;
	n = wrapped = 0;

retry:
	sgd = ss.grid;
	for (i = fy + 1; i < gd->hsize + gd->sy; i++) {
		first = 0;
		if (i == fy + 1)
			first = fx;
		n = window_copy_search_lr(gd, sgd, &px, i - 1, first, gd->sx);
		if (n) {
			window_copy_scroll_to(wp, px, i - 1);
			break;
		}
	}
	if (!n && !wrapped) {
		fx = 0;
		fy = 0;
		wrapped = 1;
		goto retry;
	}

	screen_free(&ss);
}

void
window_copy_goto_line(struct window_pane *wp, const char *linestr)
{
	struct window_copy_mode_data	*data = wp->modedata;
	const char			*errstr;
	u_int				 lineno;

	lineno = strtonum(linestr, 0, screen_hsize(&wp->base), &errstr);
	if (errstr != NULL)
		return;

	data->oy = lineno;
	window_copy_update_selection(wp);
	window_copy_redraw_screen(wp);
}

void
window_copy_write_line(
    struct window_pane *wp, struct screen_write_ctx *ctx, u_int py)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;
	struct options			*oo = &wp->window->options;
	struct grid_cell		 gc;
	char				 hdr[32];
	size_t	 			 last, xoff = 0, size = 0;

	memcpy(&gc, &grid_default_cell, sizeof gc);
	colour_set_fg(&gc, options_get_number(oo, "mode-fg"));
	colour_set_bg(&gc, options_get_number(oo, "mode-bg"));
	gc.attr |= options_get_number(oo, "mode-attr");

	last = screen_size_y(s) - 1;
	if (py == 0) {
		size = xsnprintf(hdr, sizeof hdr,
		    "[%u/%u]", data->oy, screen_hsize(&wp->base));
		screen_write_cursormove(ctx, screen_size_x(s) - size, 0);
		screen_write_puts(ctx, &gc, "%s", hdr);
	} else if (py == last && data->inputtype != WINDOW_COPY_OFF) {
		xoff = size = xsnprintf(hdr, sizeof hdr,
		    "%s: %s", data->inputprompt, data->inputstr);
		screen_write_cursormove(ctx, 0, last);
		screen_write_puts(ctx, &gc, "%s", hdr);
	} else
		size = 0;

	screen_write_cursormove(ctx, xoff, py);
	screen_write_copy(ctx, &wp->base, xoff, (screen_hsize(&wp->base) -
	    data->oy) + py, screen_size_x(s) - size, 1);

	if (py == data->cy && data->cx == screen_size_x(s)) {
		memcpy(&gc, &grid_default_cell, sizeof gc);
		screen_write_cursormove(ctx, screen_size_x(s) - 1, py);
		screen_write_putc(ctx, &gc, '$');
	}
}

void
window_copy_write_lines(
    struct window_pane *wp, struct screen_write_ctx *ctx, u_int py, u_int ny)
{
	u_int	yy;

	for (yy = py; yy < py + ny; yy++)
		window_copy_write_line(wp, ctx, py);
}

void
window_copy_redraw_lines(struct window_pane *wp, u_int py, u_int ny)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen_write_ctx	 	 ctx;
	u_int				 i;

	screen_write_start(&ctx, wp, NULL);
	for (i = py; i < py + ny; i++)
		window_copy_write_line(wp, &ctx, i);
	screen_write_cursormove(&ctx, data->cx, data->cy);
	screen_write_stop(&ctx);
}

void
window_copy_redraw_screen(struct window_pane *wp)
{
	struct window_copy_mode_data	*data = wp->modedata;

	window_copy_redraw_lines(wp, 0, screen_size_y(&data->screen));
}

void
window_copy_update_cursor(struct window_pane *wp, u_int cx, u_int cy)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;
	struct screen_write_ctx		 ctx;
	u_int				 old_cx, old_cy;

	old_cx = data->cx; old_cy = data->cy;
	data->cx = cx; data->cy = cy;
	if (old_cx == screen_size_x(s))
		window_copy_redraw_lines(wp, old_cy, 1);
	if (data->cx == screen_size_x(s))
		window_copy_redraw_lines(wp, data->cy, 1);
	else {
		screen_write_start(&ctx, wp, NULL);
		screen_write_cursormove(&ctx, data->cx, data->cy);
		screen_write_stop(&ctx);
	}
}

void
window_copy_start_selection(struct window_pane *wp)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;

	data->selx = data->cx;
	data->sely = screen_hsize(&wp->base) + data->cy - data->oy;

	s->sel.flag = 1;
	window_copy_update_selection(wp);
}

int
window_copy_update_selection(struct window_pane *wp)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;
	struct options			*oo = &wp->window->options;
	struct grid_cell		 gc;
	u_int				 sx, sy, ty;

	if (!s->sel.flag)
		return (0);

	/* Set colours. */
	memcpy(&gc, &grid_default_cell, sizeof gc);
	colour_set_fg(&gc, options_get_number(oo, "mode-fg"));
	colour_set_bg(&gc, options_get_number(oo, "mode-bg"));
	gc.attr |= options_get_number(oo, "mode-attr");

	/* Find top of screen. */
	ty = screen_hsize(&wp->base) - data->oy;

	/* Adjust the selection. */
	sx = data->selx;
	sy = data->sely;
	if (sy < ty) {					/* above screen */
		sx = 0;
		sy = 0;
	} else if (sy > ty + screen_size_y(s) - 1) {	/* below screen */
		sx = screen_size_x(s) - 1;
		sy = screen_size_y(s) - 1;
	} else
		sy -= ty;
	sy = screen_hsize(s) + sy;

	screen_set_selection(
	    s, sx, sy, data->cx, screen_hsize(s) + data->cy, &gc);
	return (1);
}

void
window_copy_copy_selection(struct window_pane *wp, struct client *c)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;
	char				*buf;
	size_t	 			 off;
	u_int	 			 i, xx, yy, sx, sy, ex, ey, limit;

	if (!s->sel.flag)
		return;

	buf = xmalloc(1);
	off = 0;

	*buf = '\0';

	/*
	 * The selection extends from selx,sely to (adjusted) cx,cy on
	 * the base screen.
	 */

	/* Find start and end. */
	xx = data->cx;
	yy = screen_hsize(&wp->base) + data->cy - data->oy;
	if (yy < data->sely || (yy == data->sely && xx < data->selx)) {
		sx = xx; sy = yy;
		ex = data->selx; ey = data->sely;
	} else {
		sx = data->selx; sy = data->sely;
		ex = xx; ey = yy;
	}

	/* Trim ex to end of line. */
	xx = window_copy_find_length(wp, ey);
	if (ex > xx)
		ex = xx;

	/* Copy the lines. */
	if (sy == ey)
		window_copy_copy_line(wp, &buf, &off, sy, sx, ex);
	else {
		xx = screen_size_x(s);
		window_copy_copy_line(wp, &buf, &off, sy, sx, xx);
		if (ey - sy > 1) {
			for (i = sy + 1; i < ey; i++)
				window_copy_copy_line(wp, &buf, &off, i, 0, xx);
		}
		window_copy_copy_line(wp, &buf, &off, ey, 0, ex);
	}

	/* Don't bother if no data. */
	if (off == 0) {
		xfree(buf);
		return;
	}
	off--;	/* remove final \n */

	/* Add the buffer to the stack. */
	limit = options_get_number(&c->session->options, "buffer-limit");
	paste_add(&c->session->buffers, buf, off, limit);
}

void
window_copy_copy_line(struct window_pane *wp,
    char **buf, size_t *off, u_int sy, u_int sx, u_int ex)
{
	struct grid		*gd = wp->base.grid;
	const struct grid_cell	*gc;
	const struct grid_utf8	*gu;
	struct grid_line	*gl;
	u_int			 i, xx, wrapped = 0;
	size_t			 size;

	if (sx > ex)
		return;

	/*
	 * Work out if the line was wrapped at the screen edge and all of it is
	 * on screen.
	 */
	gl = &gd->linedata[sy];
	if (gl->flags & GRID_LINE_WRAPPED && gl->cellsize <= gd->sx)
		wrapped = 1;

	/* If the line was wrapped, don't strip spaces (use the full length). */
	if (wrapped)
		xx = gl->cellsize;
	else
		xx = window_copy_find_length(wp, sy);
	if (ex > xx)
		ex = xx;
	if (sx > xx)
		sx = xx;

	if (sx < ex) {
		for (i = sx; i < ex; i++) {
			gc = grid_peek_cell(gd, i, sy);
			if (gc->flags & GRID_FLAG_PADDING)
				continue;
			if (!(gc->flags & GRID_FLAG_UTF8)) {
				*buf = xrealloc(*buf, 1, (*off) + 1);
				(*buf)[(*off)++] = gc->data;
			} else {
				gu = grid_peek_utf8(gd, i, sy);
				size = grid_utf8_size(gu);
				*buf = xrealloc(*buf, 1, (*off) + size);
				*off += grid_utf8_copy(gu, *buf + *off, size);
			}
		}
	}

	/* Only add a newline if the line wasn't wrapped. */
	if (!wrapped) {
		*buf = xrealloc(*buf, 1, (*off) + 1);
		(*buf)[(*off)++] = '\n';
	}
}

int
window_copy_in_set(struct window_pane *wp, u_int px, u_int py, const char *set)
{
	const struct grid_cell	*gc;

	gc = grid_peek_cell(wp->base.grid, px, py);
	if (gc->flags & (GRID_FLAG_PADDING|GRID_FLAG_UTF8))
		return (0);
	if (gc->data == 0x00 || gc->data == 0x7f)
		return (0);
	return (strchr(set, gc->data) != NULL);
}

u_int
window_copy_find_length(struct window_pane *wp, u_int py)
{
	const struct grid_cell	*gc;
	u_int			 px;

	/*
	 * If the pane has been resized, its grid can contain old overlong
	 * lines. grid_peek_cell does not allow accessing cells beyond the
	 * width of the grid, and screen_write_copy treats them as spaces, so
	 * ignore them here too.
	 */
	px = wp->base.grid->linedata[py].cellsize;
	if (px > screen_size_x(&wp->base))
		px = screen_size_x(&wp->base);
	while (px > 0) {
		gc = grid_peek_cell(wp->base.grid, px - 1, py);
		if (gc->flags & GRID_FLAG_UTF8)
			break;
		if (gc->data != ' ')
			break;
		px--;
	}
	return (px);
}

void
window_copy_cursor_start_of_line(struct window_pane *wp)
{
	struct window_copy_mode_data	*data = wp->modedata;

	window_copy_update_cursor(wp, 0, data->cy);
	if (window_copy_update_selection(wp))
		window_copy_redraw_lines(wp, data->cy, 1);
}

void
window_copy_cursor_back_to_indentation(struct window_pane *wp)
{
	struct window_copy_mode_data	*data = wp->modedata;
	u_int				 px, py, xx;
	const struct grid_cell		*gc;

	px = 0;
	py = screen_hsize(&wp->base) + data->cy - data->oy;
	xx = window_copy_find_length(wp, py);

	while (px < xx) {
		gc = grid_peek_cell(wp->base.grid, px, py);
		if (gc->flags & GRID_FLAG_UTF8)
			break;
		if (gc->data != ' ')
			break;
		px++;
	}

	window_copy_update_cursor(wp, px, data->cy);
	if (window_copy_update_selection(wp))
		window_copy_redraw_lines(wp, data->cy, 1);
}

void
window_copy_cursor_end_of_line(struct window_pane *wp)
{
	struct window_copy_mode_data	*data = wp->modedata;
	u_int				 px, py;

	py = screen_hsize(&wp->base) + data->cy - data->oy;
	px = window_copy_find_length(wp, py);

	window_copy_update_cursor(wp, px, data->cy);
	if (window_copy_update_selection(wp))
		window_copy_redraw_lines(wp, data->cy, 1);
}

void
window_copy_cursor_left(struct window_pane *wp)
{
	struct window_copy_mode_data	*data = wp->modedata;

	if (data->cx == 0) {
		window_copy_cursor_up(wp, 0);
		window_copy_cursor_end_of_line(wp);
	} else {
		window_copy_update_cursor(wp, data->cx - 1, data->cy);
		if (window_copy_update_selection(wp))
			window_copy_redraw_lines(wp, data->cy, 1);
	}
}

void
window_copy_cursor_right(struct window_pane *wp)
{
	struct window_copy_mode_data	*data = wp->modedata;
	u_int				 px, py;

	py = screen_hsize(&wp->base) + data->cy - data->oy;
	px = window_copy_find_length(wp, py);

	if (data->cx >= px) {
		window_copy_cursor_start_of_line(wp);
		window_copy_cursor_down(wp, 0);
	} else {
		window_copy_update_cursor(wp, data->cx + 1, data->cy);
		if (window_copy_update_selection(wp))
			window_copy_redraw_lines(wp, data->cy, 1);
	}
}

void
window_copy_cursor_up(struct window_pane *wp, int scroll_only)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;
	u_int				 ox, oy, px, py;

	oy = screen_hsize(&wp->base) + data->cy - data->oy;
	ox = window_copy_find_length(wp, oy);
	if (ox != 0) {
		data->lastcx = data->cx;
		data->lastsx = ox;
	}

	data->cx = data->lastcx;
	if (scroll_only || data->cy == 0) {
		window_copy_scroll_down(wp, 1);
		if (scroll_only) {
			if (data->cy == screen_size_y(s) - 1)
				window_copy_redraw_lines(wp, data->cy, 1);
			else
				window_copy_redraw_lines(wp, data->cy, 2);
		}
	} else {
		window_copy_update_cursor(wp, data->cx, data->cy - 1);
		if (window_copy_update_selection(wp)) {
			if (data->cy == screen_size_y(s) - 1)
				window_copy_redraw_lines(wp, data->cy, 1);
			else
				window_copy_redraw_lines(wp, data->cy, 2);
		}
	}

	py = screen_hsize(&wp->base) + data->cy - data->oy;
	px = window_copy_find_length(wp, py);
	if (data->cx >= data->lastsx || data->cx > px)
		window_copy_cursor_end_of_line(wp);
}

void
window_copy_cursor_down(struct window_pane *wp, int scroll_only)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;
	u_int				 ox, oy, px, py;

	oy = screen_hsize(&wp->base) + data->cy - data->oy;
	ox = window_copy_find_length(wp, oy);
	if (ox != 0) {
		data->lastcx = data->cx;
		data->lastsx = ox;
	}

	data->cx = data->lastcx;
	if (scroll_only || data->cy == screen_size_y(s) - 1) {
		window_copy_scroll_up(wp, 1);
		if (scroll_only && data->cy > 0)
			window_copy_redraw_lines(wp, data->cy - 1, 2);
	} else {
		window_copy_update_cursor(wp, data->cx, data->cy + 1);
		if (window_copy_update_selection(wp))
			window_copy_redraw_lines(wp, data->cy - 1, 2);
	}

	py = screen_hsize(&wp->base) + data->cy - data->oy;
	px = window_copy_find_length(wp, py);
	if (data->cx >= data->lastsx || data->cx > px)
		window_copy_cursor_end_of_line(wp);
}

void
window_copy_cursor_next_word(struct window_pane *wp, const char *separators)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*base_s = &wp->base;
	u_int				 px, py, xx, yy;

	px = data->cx;
	py = screen_hsize(base_s) + data->cy - data->oy;
	xx = window_copy_find_length(wp, py);
	yy = screen_hsize(base_s) + screen_size_y(base_s) - 1;

	/* Are we in a word? Skip it! */
	while (!window_copy_in_set(wp, px, py, separators))
		px++;

	/* Find the start of a word. */
	while (px > xx || window_copy_in_set(wp, px, py, separators)) {
		/* Past the end of the line? Nothing but spaces. */
		if (px > xx) {
			if (py == yy)
				return;
			window_copy_cursor_down(wp, 0);
			px = 0;

			py = screen_hsize(base_s) + data->cy - data->oy;
			xx = window_copy_find_length(wp, py);
		}
		px++;
	}

	window_copy_update_cursor(wp, px, data->cy);
	if (window_copy_update_selection(wp))
		window_copy_redraw_lines(wp, data->cy, 1);
}

void
window_copy_cursor_next_word_end(struct window_pane *wp, const char *separators)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*base_s = &wp->base;
	u_int				 px, py, xx, yy;

	px = data->cx;
	py = screen_hsize(base_s) + data->cy - data->oy;
	xx = window_copy_find_length(wp, py);
	yy = screen_hsize(base_s) + screen_size_y(base_s) - 1;

	/* Are we on spaces? Skip 'em! */
	while (px > xx || window_copy_in_set(wp, px, py, separators)) {
		/* Nothing but spaces past the end of the line, so move down. */
		if (px > xx) {
			if (py == yy)
				return;
			window_copy_cursor_down(wp, 0);
			px = 0;

			py = screen_hsize(base_s) + data->cy - data->oy;
			xx = window_copy_find_length(wp, py);
		}
		px++;
	}

	/* Find the end of this word. */
	while (!window_copy_in_set(wp, px, py, separators))
		px++;

	window_copy_update_cursor(wp, px, data->cy);
	if (window_copy_update_selection(wp))
		window_copy_redraw_lines(wp, data->cy, 1);
}

/* Move to the previous place where a word begins. */
void
window_copy_cursor_previous_word(struct window_pane *wp, const char *separators)
{
	struct window_copy_mode_data	*data = wp->modedata;
	u_int				 px, py;

	px = data->cx;
	py = screen_hsize(&wp->base) + data->cy - data->oy;

	/* Move back to the previous word character. */
	for (;;) {
		if (px > 0) {
			px--;
			if (!window_copy_in_set(wp, px, py, separators))
				break;
		} else {
			if (data->cy == 0 &&
			    (screen_hsize(&wp->base) == 0 ||
			    data->oy >= screen_hsize(&wp->base) - 1))
				goto out;
			window_copy_cursor_up(wp, 0);

			py = screen_hsize(&wp->base) + data->cy - data->oy;
			px = window_copy_find_length(wp, py);
		}
	}

	/* Move back to the beginning of this word. */
	while (px > 0 && !window_copy_in_set(wp, px - 1, py, separators))
		px--;

out:
	window_copy_update_cursor(wp, px, data->cy);
	if (window_copy_update_selection(wp))
		window_copy_redraw_lines(wp, data->cy, 1);
}

void
window_copy_scroll_up(struct window_pane *wp, u_int ny)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;
	struct screen_write_ctx		 ctx;

	if (data->oy < ny)
		ny = data->oy;
	if (ny == 0)
		return;
	data->oy -= ny;
	window_copy_update_selection(wp);

	screen_write_start(&ctx, wp, NULL);
	screen_write_cursormove(&ctx, 0, 0);
	screen_write_deleteline(&ctx, ny);
	window_copy_write_lines(wp, &ctx, screen_size_y(s) - ny, ny);
	window_copy_write_line(wp, &ctx, 0);
	if (screen_size_y(s) > 1)
		window_copy_write_line(wp, &ctx, 1);
	if (screen_size_y(s) > 3)
		window_copy_write_line(wp, &ctx, screen_size_y(s) - 2);
	if (s->sel.flag && screen_size_y(s) > ny)
		window_copy_write_line(wp, &ctx, screen_size_y(s) - ny - 1);
	screen_write_cursormove(&ctx, data->cx, data->cy);
	screen_write_stop(&ctx);
}

void
window_copy_scroll_down(struct window_pane *wp, u_int ny)
{
	struct window_copy_mode_data	*data = wp->modedata;
	struct screen			*s = &data->screen;
	struct screen_write_ctx		 ctx;

	if (ny > screen_hsize(&wp->base))
		return;

	if (data->oy > screen_hsize(&wp->base) - ny)
		ny = screen_hsize(&wp->base) - data->oy;
	if (ny == 0)
		return;
	data->oy += ny;
	window_copy_update_selection(wp);

	screen_write_start(&ctx, wp, NULL);
	screen_write_cursormove(&ctx, 0, 0);
	screen_write_insertline(&ctx, ny);
	window_copy_write_lines(wp, &ctx, 0, ny);
	if (s->sel.flag && screen_size_y(s) > ny)
		window_copy_write_line(wp, &ctx, ny);
	else if (ny == 1) /* nuke position */
		window_copy_write_line(wp, &ctx, 1);
	screen_write_cursormove(&ctx, data->cx, data->cy);
	screen_write_stop(&ctx);
}
