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


int n_samples;


struct num eval_unit(const struct expr *expr, const struct frame *frame);


void reset_samples(struct sample **samples, int n)
{
	struct sample *next;
	int i;

	for (i = 0; i != n; i++)
		while (samples[i]) {
			next = samples[i]->next;
			free(samples[i]);
			samples[i] = next;
		}
}


void meas_start(void)
{
	const struct frame *frame;
	struct vec *vec;

	n_samples = 0;
	for (frame = frames; frame; frame = frame->next)
		for (vec = frame->vecs; vec; vec = vec->next)
			vec->n = n_samples++;
}


void meas_post(const struct vec *vec, struct coord pos)
{
	struct sample **walk, *new;

	for (walk = &curr_pkg->samples[vec->n]; *walk; walk = &(*walk)->next) {
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


/* ----- lt operators ------------------------------------------------------ */


int lt_x(struct coord a, struct coord b)
{
	return a.x < b.x;
}


int lt_y(struct coord a, struct coord b)
{
	return a.y < b.y;
}


int lt_xy(struct coord a, struct coord b)
{
	return a.y < b.y || (a.y == b.y && a.x < b.x);
}


/* ----- measurement type map ---------------------------------------------- */


static lt_op_type lt_op[mt_n] = {
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


/* ----- search functions -------------------------------------------------- */


static int closer(int da, int db)
{
	int abs_a, abs_b;

	abs_a = da < 0 ? -da : da;
	abs_b = db < 0 ? -db : db;
	if (abs_a < abs_b)
		return 1;
	if (abs_a > abs_b)
		return 0;
	/*
	 * Really *all* other things being equal, pick the one that protrudes
	 * in the positive direction.
	 */
	return da > db;
}


static int better_next(lt_op_type lt,
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
		return closer(b.y-a0.y, b0.y-a0.y);
	if (lt == lt_y)
		return closer(b.x-a0.x, b0.x-a0.x);
	abort();
}


/*
 * In order to obtain a stable order, we sort points equal on the measured
 * coordinate also by xy:
 *
 * if (*a < a0) use *a
 * else if (*a == a0 && *a <xy a0) use *a
 */

struct coord meas_find_min(lt_op_type lt, const struct sample *s)
{
	struct coord min;

	min = s->pos;
	while (s) {
		if (lt(s->pos, min) ||
		    (!lt(min, s->pos) && lt_xy(s->pos, min)))
			min = s->pos;
		s = s->next;
	}
	return min;
}


struct coord meas_find_next(lt_op_type lt, const struct sample *s,
    struct coord ref)
{
	struct coord next;

	next = s->pos;
	while (s) {
		if (better_next(lt, ref, next, s->pos))
			next = s->pos;
		s = s->next;
	}
	return next;
}


struct coord meas_find_max(lt_op_type lt, const struct sample *s)
{
	struct coord max;

	max = s->pos;
	while (s) {
		if (lt(max, s->pos) ||
		    (!lt(s->pos, max) && lt_xy(max, s->pos)))
			max = s->pos;
		s = s->next;
	}
	return max;
}


/* ----- instantiation ----------------------------------------------------- */


static int instantiate_meas_pkg(void)
{
	struct obj *obj;
	const struct meas *meas;
	struct coord a0, b0;
	lt_op_type lt;

	for (obj = frames->objs; obj; obj = obj->next) {
		if (obj->type != ot_meas)
			continue;
		meas = &obj->u.meas;
		if (!curr_pkg->samples[obj->base->n] ||
		    !curr_pkg->samples[meas->high->n])
			continue;

		lt = lt_op[meas->type];
		a0 = meas_find_min(lt, curr_pkg->samples[obj->base->n]);
		if (is_next[meas->type])
			b0 = meas_find_next(lt,
			    curr_pkg->samples[meas->high->n], a0);
		else
			b0 = meas_find_max(lt,
			    curr_pkg->samples[meas->high->n]);

		inst_meas(obj,
		    meas->inverted ? b0 : a0, meas->inverted ? a0 : b0);
	}
	return 1;
}


int instantiate_meas(void)
{
	struct pkg *pkg;

	curr_frame = pkgs->insts[ip_frame];
	for (pkg = pkgs; pkg; pkg = pkg->next)
		if (pkg->name) {
			inst_select_pkg(pkg->name);
			if (!instantiate_meas_pkg())
				return 0;
		}
	return 1;
}
