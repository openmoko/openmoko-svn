/*
 * gui_over.c - GUI, canvas overlays
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * This file is for the overlay state machine only. Given the heavy use of
 * global variables, adding other functionality would quickly render it
 * illegible.
 */


#include <stdlib.h>
#include <stdio.h>

#include "coord.h"
#include "gui_util.h"
#include "gui_over.h"


#if 0
#define DPRINTF(fmt, ...)	fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define DSAVE(pix_buf)		debug_save_pixbuf(pix_buf->buf)
#else
#define	DPRINTF(fmt, ...)
#define	DSAVE(buf)
#endif


static enum states {
	NOTHING,
	HOVER,
	DRAG,
	BOTH,
} state = NOTHING;


/*
 * We cache some externally provided state so that we can redraw without the
 * outside telling us what to redraw, etc.
 */

static struct pix_buf *buf_D, *buf_H;
static struct pix_buf *(*over_D_save_and_draw)(void *user, struct coord to);
static void *over_D_user;
static struct pix_buf *(*over_H_save_and_draw)(void *user);
static void *over_H_user;
static struct coord over_pos;


/* ----- actions ----------------------------------------------------------- */


static void draw_D(void)
{
	buf_D = over_D_save_and_draw(over_D_user, over_pos);
	DSAVE(buf_D);
}


static void draw_H(void)
{
	buf_H = over_H_save_and_draw(over_H_user);
	DSAVE(buf_H);
}


#define STATE(s)	DPRINTF("%s", #s); state = s; break;
#define	restore(x)	DPRINTF("  restore(%s)", #x); restore_pix_buf(buf_##x)
#define	drop(x)		DPRINTF("  drop(%s)", #x); free_pix_buf(buf_##x)
#define	save(x)		DPRINTF("  save(%s)", #x)
#define	draw(x)		DPRINTF("  draw(%s)", #x); draw_##x()
#define	update()	DPRINTF("  update"); over_pos = pos


/* ----- state machine ----------------------------------------------------- */


void over_enter(struct pix_buf *(*save_and_draw)(void *user), void *user)
{
	over_H_save_and_draw = save_and_draw;
	over_H_user = user;

	DPRINTF("enter");
	switch (state) {
	case NOTHING:
		save(H);
		draw(H);
		STATE(HOVER);
	case DRAG:
		restore(D);
		save(H);
		draw(H);
		save(D);
		draw(D);
		STATE(BOTH);
	default:
		abort();
	}
}


void over_leave(void)
{
	DPRINTF("leave");
	switch (state) {
	case HOVER:
		restore(H);
		STATE(NOTHING);
	case BOTH:
		restore(D);
		restore(H);
		save(D);
		draw(D);
		STATE(DRAG);
	default:
		abort();
	}
}


void over_begin(struct pix_buf *(*save_and_draw)(void *user, struct coord to),
    void *user, struct coord pos)
{
	over_pos = pos;
	over_D_save_and_draw = save_and_draw;
	over_D_user = user;

	DPRINTF("begin");
	switch (state) {
	case NOTHING:
		save(D);
		draw(D);
		STATE(DRAG);
	case HOVER:
		save(D);
		draw(D);
		STATE(BOTH);
	default:
		abort();
	}
}


void over_move(struct coord pos)
{
	over_pos = pos;

	DPRINTF("move");
	switch (state) {
	case NOTHING:
		break;
	case HOVER:
		break;
	case DRAG:
		restore(D);
		update();
		save(D);
		draw(D);
		STATE(DRAG);
	case BOTH:
		restore(D);
		update();
		save(D);
		draw(D);
		STATE(BOTH);
	default:
		abort();
	}
}


void over_end(void)
{
	DPRINTF("end");
	switch (state) {
	case DRAG:
		restore(D);
		STATE(NOTHING);
	case BOTH:
		restore(D);
		STATE(HOVER);
	default:
		abort();
	}
}


void over_reset(void)
{
	DPRINTF("reset");
	switch (state) {
	case NOTHING:
		break;
	case HOVER:
		drop(H);
		STATE(NOTHING);
	case DRAG:
		drop(D);
		STATE(NOTHING);
	case BOTH:
		drop(D);
		drop(H);
		STATE(NOTHING);
	default:
		abort();
	}
}
