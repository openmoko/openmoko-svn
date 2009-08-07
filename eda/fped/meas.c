/*
 * meas.c - Measurements
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>

#include "util.h"
#include "coord.h"
#include "expr.h"
#include "obj.h"
#include "inst.h"
#include "meas.h"


struct num eval_unit(const struct expr *expr, const struct frame *frame);

struct sample {
	struct coord pos;
	struct sample *next;
};

struct meas *measurements = NULL;


static void reset_samples(struct sample **samples)
{
	struct sample *next;

	while (*samples) {
		next = (*samples)->next;
		free(*samples);
		*samples = next;
	}
}


void meas_start(void)
{
	struct frame *frame;
	struct vec *vec;

	for (frame = frames; frame; frame = frame->next)
		for (vec = frame->vecs; vec; vec = vec->next)
			reset_samples(&vec->samples);
}


void meas_post(struct vec *vec, struct coord pos)
{
	struct sample **walk, *new;

	for (walk = &vec->samples; *walk; walk = &(*walk)->next) {
		if (pos.y < (*walk)->pos.y)
			break;
		if (pos.y > (*walk)->pos.y)
			continue;
		if (pos.x < (*walk)->pos.x)
			break;
		if (pos.x == (*walk)->pos.x)
			return;
	}
	new = alloc_type(struct sample);
	new->pos = pos;
	new->next = *walk;
	*walk = new;
}


static int lt_x(struct coord a, struct coord b)
{
	return a.x < b.x;
}


static int lt_y(struct coord a, struct coord b)
{
	return a.y < b.y;
}


static int lt_xy(struct coord a, struct coord b)
{
	return a.y < b.y || (a.y == b.y && a.x < b.x);
}


static int (*lt_op[mt_n])(struct coord a, struct coord b) = {
	lt_xy,
	lt_x,
	lt_y,
	lt_xy,
	lt_x,
	lt_y
};


static int is_next[mt_n] = {
	1, 1, 1,
	0, 0, 0
};


static int better_next(int (*lt)(struct coord a, struct coord b),
    struct coord a0, struct coord b0, struct coord b)
{
	/* if we don't have any suitable point A0 < B0 yet, use this one */
	if (!lt(a0, b0))
		return 1;

	/* B must be strictly greater than A0 */
	if (!lt(a0, b))
		return 0;

	/* if we can get closer to A0, do so */
	if (lt(b, b0))
		return 1;

	/* reject B > B0 */
	if (lt(b0, b))
		return 0;

	/*
	 * B == B0 along the coordinate we measure. Now give the other
	 * coordinate a chance. This gives us a stable sort order and it
	 * makes meas/measx/measy usually select the same point.
	 */
	if (lt == lt_xy)
		return 0;
	if (lt == lt_x)
		return better_next(lt_y, a0, b0, b);
	if (lt == lt_y)
		return better_next(lt_x, a0, b0, b);
	abort();
}


int instantiate_meas(void)
{
	struct meas *meas;
	struct coord a0, b0;
	const struct sample *a, *b;
	int (*lt)(struct coord a, struct coord b);
	struct num offset;

	for (meas = measurements; meas; meas = meas->next) {
		if (!meas->low->samples || !meas->high->samples)
			continue;

		lt = lt_op[meas->type];

		/*
		 * In order to obtain a stable order, we sort points equal on
		 * the measured coordinate also by xy:
		 *
		 * if (*a < a0) use *a
		 * else if (*a == a0 && *a <xy a0) use *a
		 */
		a0 = meas->low->samples->pos;
		for (a = meas->low->samples; a; a = a->next)
			if (lt(a->pos, a0) ||
			    (!lt(a0, a->pos) && lt_xy(a->pos, a0)))
				a0 = a->pos;

		b0 = meas->high->samples->pos;
		for (b = meas->high->samples; b; b = b->next) {
			if (is_next[meas->type]) {
				if (better_next(lt, a0, b0, b->pos))
					b0 = b->pos;
			} else {
				if (lt(b0, b->pos) ||
				    (!lt(b->pos, b0) && lt_xy(b0, b->pos)))
					b0 = b->pos;
			}
		}

		offset = eval_unit(meas->offset, root_frame);
		if (is_undef(offset))
			return 0;
		inst_meas(NULL, meas,
		    meas->inverted ? b0 : a0, meas->inverted ? a0 : b0,
		    offset.n);
	}
	return 1;
}
