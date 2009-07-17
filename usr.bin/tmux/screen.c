/* $OpenBSD: screen.c,v 1.9 2009/07/17 18:35:11 nicm Exp $ */

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
#include <vis.h>

#include "tmux.h"

void	screen_resize_x(struct screen *, u_int);
void	screen_resize_y(struct screen *, u_int);

/* Create a new screen. */
void
screen_init(struct screen *s, u_int sx, u_int sy, u_int hlimit)
{
	s->grid = grid_create(sx, sy, hlimit);

	s->title = xstrdup("");

	s->tabs = NULL;

	screen_reinit(s);
}

/* Reinitialise screen. */
void
screen_reinit(struct screen *s)
{
	s->cx = 0;
	s->cy = 0;

	s->rupper = 0;
	s->rlower = screen_size_y(s) - 1;

	s->mode = MODE_CURSOR;
	
	screen_reset_tabs(s);

	grid_clear_lines(s->grid, s->grid->hsize, s->grid->sy);

	screen_clear_selection(s);
}

/* Destroy a screen. */
void
screen_free(struct screen *s)
{
	xfree(s->title);
	grid_destroy(s->grid);
}

/* Reset tabs to default, eight spaces apart. */
void
screen_reset_tabs(struct screen *s)
{
	u_int	i;

	if (s->tabs != NULL)
		xfree(s->tabs);

	if ((s->tabs = bit_alloc(screen_size_x(s))) == NULL)
		fatal("bit_alloc failed");
	for (i = 8; i < screen_size_x(s); i += 8)
		bit_set(s->tabs, i);
}

/* Set screen title. */
void
screen_set_title(struct screen *s, const char *title)
{
	char	tmp[BUFSIZ];

	strnvis(tmp, title, sizeof tmp, VIS_OCTAL|VIS_TAB|VIS_NL);

	xfree(s->title);
	s->title = xstrdup(tmp);
}

/* Resize screen. */
void
screen_resize(struct screen *s, u_int sx, u_int sy)
{
	if (sx < 1)
		sx = 1;
	if (sy < 1)
		sy = 1;

	if (sx != screen_size_x(s)) {
		screen_resize_x(s, sx);

		/*
		 * It is unclear what should happen to tabs on resize. xterm
		 * seems to try and maintain them, rxvt resets them. Resetting
		 * is simpler and more reliable so let's do that.
		 */
		screen_reset_tabs(s);
	}

	if (sy != screen_size_y(s))
		screen_resize_y(s, sy);
}

void
screen_resize_x(struct screen *s, u_int sx)
{
	struct grid		*gd = s->grid;

	if (sx == 0)
		fatalx("zero size");

	/*
	 * Treat resizing horizontally simply: just ensure the cursor is
	 * on-screen and change the size. Don't bother to truncate any lines -
	 * then the data should be accessible if the size is then incrased.
	 *
	 * The only potential wrinkle is if UTF-8 double-width characters are
	 * left in the last column, but UTF-8 terminals should deal with this
	 * sanely.
	 */
	if (s->cx >= sx)
		s->cx = sx - 1;
	gd->sx = sx;
}

void
screen_resize_y(struct screen *s, u_int sy)
{
	struct grid	*gd = s->grid;
	u_int		 needed, available, oldy, i;

	if (sy == 0)
		fatalx("zero size");
	oldy = screen_size_y(s);

	/* 
	 * When resizing:
	 *
	 * If the height is decreasing, delete lines from the bottom until
	 * hitting the cursor, then push lines from the top into the history.
	 * 
	 * When increasing, pull as many lines as possible from the history to
	 * the top, then fill the remaining with blanks at the bottom.
	 */

	/* Size decreasing. */
	if (sy < oldy) {
		needed = oldy - sy;

		/* Delete as many lines as possible from the bottom. */
		available = oldy - 1 - s->cy;
		if (available > 0) {
			if (available > needed)
				available = needed;
			grid_view_delete_lines(gd, oldy - available, available);
		}
		needed -= available;

		/*
		 * Now just increase the history size, if possible, to take
		 * over the lines which are left. If history is off, delete
		 * lines from the top.
		 *
		 * XXX Should apply history limit?
		 */
		available = s->cy;
		if (gd->flags & GRID_HISTORY)
			gd->hsize += needed;
		else if (needed > 0 && available > 0) {
			if (available > needed)
				available = needed;
			grid_view_delete_lines(gd, 0, available);
		}
		s->cy -= needed;
 	}

	/* Resize line arrays. */
	gd->size = xrealloc(gd->size, gd->hsize + sy, sizeof *gd->size);
	gd->data = xrealloc(gd->data, gd->hsize + sy, sizeof *gd->data);
	gd->usize = xrealloc(gd->usize, gd->hsize + sy, sizeof *gd->usize);
	gd->udata = xrealloc(gd->udata, gd->hsize + sy, sizeof *gd->udata);

	/* Size increasing. */
	if (sy > oldy) {
		needed = sy - oldy;

		/*
		 * Try to pull as much as possible out of the history, if is
		 * is enabled.
		 */
		available = gd->hsize;
		if (gd->flags & GRID_HISTORY && available > 0) {
			if (available > needed)
				available = needed;
			gd->hsize -= available;
			s->cy += available;
		} else
			available = 0;
		needed -= available;

		/* Then fill the rest in with blanks. */
		for (i = gd->hsize + sy - needed; i < gd->hsize + sy; i++) {
			gd->size[i] = 0;
			gd->data[i] = NULL;
			gd->usize[i] = 0;
			gd->udata[i] = NULL;
		}
	}

	/* Set the new size, and reset the scroll region. */
	gd->sy = sy;
	s->rupper = 0;
	s->rlower = screen_size_y(s) - 1;
}

/* Set selection. */
void
screen_set_selection(struct screen *s,
    u_int sx, u_int sy, u_int ex, u_int ey, struct grid_cell *gc)
{
	struct screen_sel	*sel = &s->sel;

	memcpy(&sel->cell, gc, sizeof sel->cell);

	sel->flag = 1;
	if (ey < sy || (sy == ey && ex < sx)) {
		sel->sx = ex; sel->sy = ey;
		sel->ex = sx; sel->ey = sy;
	} else {
		sel->sx = sx; sel->sy = sy;
		sel->ex = ex; sel->ey = ey;
	}
}

/* Clear selection. */
void
screen_clear_selection(struct screen *s)
{
	struct screen_sel	*sel = &s->sel;

	sel->flag = 0;
}

/* Check if cell in selection. */
int
screen_check_selection(struct screen *s, u_int px, u_int py)
{
	struct screen_sel	*sel = &s->sel;

	if (!sel->flag || py < sel->sy || py > sel->ey)
		return (0);

	if (py == sel->sy && py == sel->ey) {
		if (px < sel->sx || px > sel->ex)
			return (0);
		return (1);
	}

	if ((py == sel->sy && px < sel->sx) || (py == sel->ey && px > sel->ex))
		return (0);
	return (1);
}
