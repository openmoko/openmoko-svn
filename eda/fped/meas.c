/*
 * meas.c - Measurements
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
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
			bitset_free(samples[i]->frame_set);
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


void meas_post(const struct vec *vec, struct coord pos,
    const struct bitset *frame_set)
{
	struct sample **walk, *new;

	for (walk = &curr_pkg->samples[vec->n]; *walk; walk = &(*walk)->next) {
		if (pos.y < (*walk)->pos.y)
			break;
		if (pos.y > (*walk)->pos.y)
			continue;
		if (pos.x < (*walk)->pos.x)
			break;
		if (pos.x != (*walk)->pos.x)
			continue;
		if (bitset_ge((*walk)->frame_set, frame_set))
			return;
		if (bitset_ge(frame_set, (*walk)->frame_set)) {
			bitset_or((*walk)->frame_set, frame_set);
			return;
		}
	}
	new = alloc_type(struct sample);
	new->pos = pos;
	new->frame_set = bitset_clone(frame_set);
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

const struct sample *meas_find_min(lt_op_type lt, const struct sample *s,
    const struct bitset *qual)
{
	const struct sample *min = NULL;

	while (s) {
		if (!qual || bitset_ge(s->frame_set, qual))
			if (!min || lt(s->pos, min->pos) ||
			    (!lt(min->pos, s->pos) && lt_xy(s->pos, min->pos)))
				min = s;
		s = s->next;
	}
	return min;
}


const struct sample *meas_find_next(lt_op_type lt, const struct sample *s,
    struct coord ref, const struct bitset *qual)
{
	const struct sample *next = NULL;

	while (s) {
		if (!qual || bitset_ge(s->frame_set, qual))
			if (!next || better_next(lt, ref, next->pos, s->pos))
				next = s;
		s = s->next;
	}
	return next;
}


const struct sample *meas_find_max(lt_op_type lt, const struct sample *s,
    const struct bitset *qual)
{
	const struct sample *max = NULL;

	while (s) {
		if (!qual || bitset_ge(s->frame_set, qual))
			if (!max || lt(max->pos, s->pos) ||
			    (!lt(s->pos, max->pos) && lt_xy(max->pos, s->pos)))
				max = s;
		s = s->next;
	}
	return max;
}


/* ----- instantiation ----------------------------------------------------- */


static struct bitset *make_frame_set(struct frame_qual *qual, int n_frames)
{
	struct bitset *set;

	set = bitset_new(n_frames);
	while (qual) {
		bitset_set(set, qual->frame->n);
		qual = qual->next;
	}
	return set;
}


static int instantiate_meas_pkg(int n_frames)
{
	struct obj *obj;
	const struct meas *meas;
	struct bitset *set;
	const struct sample *a0, *b0;
	lt_op_type lt;

	for (obj = frames->objs; obj; obj = obj->next) {
		if (obj->type != ot_meas)
			continue;
		meas = &obj->u.meas;

		/* optimization. not really needed anymore. */
		if (!curr_pkg->samples[obj->base->n] ||
		    !curr_pkg->samples[meas->high->n])
			continue;

		lt = lt_op[meas->type];

		set = make_frame_set(meas->low_qual, n_frames);
		a0 = meas_find_min(lt, curr_pkg->samples[obj->base->n], set);
		bitset_free(set);
		if (!a0)
			continue;

		set = make_frame_set(meas->high_qual, n_frames);
		if (is_next[meas->type])
			b0 = meas_find_next(lt,
			    curr_pkg->samples[meas->high->n], a0->pos, set);
		else
			b0 = meas_find_max(lt,
			    curr_pkg->samples[meas->high->n], set);
		bitset_free(set);
		if (!b0)
			continue;

		inst_meas(obj,
		    meas->inverted ? b0->pos : a0->pos,
		    meas->inverted ? a0->pos : b0->pos);
	}
	return 1;
}


int instantiate_meas(int n_frames)
{
	struct pkg *pkg;

	frame_instantiating = pkgs->insts[ip_frame];
	for (pkg = pkgs; pkg; pkg = pkg->next)
		if (pkg->name) {
			inst_select_pkg(pkg->name);
			if (!instantiate_meas_pkg(n_frames))
				return 0;
		}
	return 1;
}
