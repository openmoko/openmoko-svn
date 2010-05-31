/*
 * gui_meas.c - GUI, measurements
 *
 * Written 2009, 2010 by Werner Almesberger
* Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include "util.h"
#include "coord.h"
#include "meas.h"
#include "inst.h"
#include "gui_canvas.h"
#include "gui_tool.h"
#include "gui_meas.h"


static struct inst *meas_inst; /* point from which we're dragging */

static enum {
	min_to_next_or_max,
	max_to_min,
	next_to_min,
} mode;


/* ----- measurement type characteristics ---------------------------------- */


static struct meas_dsc {
	lt_op_type lt;
	enum meas_type type;
} *meas_dsc;


static struct meas_dsc meas_dsc_xy = {
	.lt	= lt_xy,
	.type	= mt_xy_next,
};


static struct meas_dsc meas_dsc_x = {
	.lt	= lt_x,
	.type	= mt_x_next,
};


static struct meas_dsc meas_dsc_y = {
	.lt	= lt_y,
	.type	= mt_y_next,
};


/* ----- min/next/max tester ----------------------------------------------- */


static int is_min(lt_op_type lt, const struct inst *inst)
{
	const struct sample *min;

	min = meas_find_min(lt, active_pkg->samples[inst->vec->n], NULL);
	return coord_eq(inst->u.vec.end, min->pos);
}


static int is_next(lt_op_type lt,
    const struct inst *inst, const struct inst *ref)
{
	const struct sample *next;

	next = meas_find_next(lt, active_pkg->samples[inst->vec->n],
	    ref->u.vec.end, NULL);
	return coord_eq(inst->u.vec.end, next->pos);
}


static int is_max(lt_op_type lt, const struct inst *inst)
{
	const struct sample *max;

	max = meas_find_max(lt, active_pkg->samples[inst->vec->n], NULL);
	return coord_eq(inst->u.vec.end, max->pos);
}


static int is_a_next(lt_op_type lt, struct inst *inst)
{
	struct inst *a;
	const struct sample *min, *next;

	for (a = insts_ip_vec(); a; a = a->next) {
		min = meas_find_min(lt, active_pkg->samples[a->vec->n], NULL);
		next = meas_find_next(lt, active_pkg->samples[inst->vec->n],
		    min->pos, NULL);
		if (coord_eq(next->pos, inst->u.vec.end))
			return 1;
	}
	return 0;
}


#if 0
static int is_min_of_next(lt_op_type lt,
    const struct inst *inst, const struct inst *ref)
{
	struct coord min, next;

	min = meas_find_min(lt, inst->vec->samples);
	next = meas_find_next(lt, ref->vec->samples, min);
	return coord_eq(next, ref->u.vec.end);
}
#endif


/* ----- picker functions -------------------------------------------------- */


static int meas_pick_vec_a(struct inst *inst, void *ctx)
{
	struct vec *vec = inst->vec;

	if (!active_pkg->samples[vec->n])
		return 0;
	if (is_min(meas_dsc->lt, inst)) {
		mode = min_to_next_or_max;
		return 1;
	}
	if (is_max(meas_dsc->lt, inst)) {
		mode = max_to_min;
		return 1;
	}
	if (is_a_next(meas_dsc->lt, inst)) {
		mode = next_to_min;
		return 1;
	}
	return 0;
}


static int meas_pick_vec_b(struct inst *inst, void *ctx)
{
	struct vec *vec = inst->vec;
	struct inst *a = ctx;

	if (!active_pkg->samples[vec->n])
		return 0;
	switch (mode) {
	case min_to_next_or_max:
		if (is_max(meas_dsc->lt, inst))
			return 1;
		if (is_next(meas_dsc->lt, inst, a))
			return 1;
		return 0;
	case max_to_min:
		return is_min(meas_dsc->lt, inst);
	case next_to_min:
		if (!is_min(meas_dsc->lt, inst))
			return 0;
		return is_next(meas_dsc->lt, a, inst);
//		return is_min_of_next(meas_dsc->lt, inst, a);
	default:
		abort();
	}
}


/* ----- highlighting ------------------------------------------------------ */


static void meas_highlight_a(void)
{
	inst_highlight_vecs(meas_pick_vec_a, NULL);
}


static void meas_highlight_b(void)
{
	inst_highlight_vecs(meas_pick_vec_b, meas_inst);
}


/* ----- meas -------------------------------------------------------------- */


struct pix_buf *draw_move_meas(struct inst *inst, struct coord pos, int i)
{
	return draw_move_line_common(inst, inst->u.meas.end, pos,
	    inst->obj->u.meas.inverted ? 1-i : i);
}


/* ----- tool selection ---------------------------------------------------- */


static void tool_selected_meas(void)
{
	highlight = meas_highlight_a;
	redraw();
}


static void tool_selected_meas_xy(void)
{
	meas_dsc = &meas_dsc_xy;
	tool_selected_meas();
}


static void tool_selected_meas_x(void)
{
	meas_dsc = &meas_dsc_x;
	tool_selected_meas();
}


static void tool_selected_meas_y(void)
{
	meas_dsc = &meas_dsc_y;
	tool_selected_meas();
}


static void tool_deselected_meas(void)
{
	highlight = NULL;
	redraw();
}


/* ----- find start point (new measurement) -------------------------------- */


static int is_highlighted(struct inst *inst, void *user)
{
	return inst->u.vec.highlighted;
}


static struct inst *find_point_meas_new(struct coord pos)
{
	return inst_find_vec(pos, is_highlighted, NULL);
}


/* ----- begin dragging new measurement ------------------------------------ */


static void begin_drag_new_meas(struct inst *inst)
{
	highlight = meas_highlight_b;
	meas_inst = inst;
	if (is_min(meas_dsc->lt, inst))
		mode = min_to_next_or_max;
	else if (is_max(meas_dsc->lt, inst))
		mode = max_to_min;
	else
		mode = next_to_min;
	redraw();
}


/* ----- end dragging new measurement -------------------------------------- */


static int end_new_meas(struct inst *from, struct inst *to)
{
	struct obj *obj;
	struct meas *meas;

	meas_inst = NULL;
	highlight = NULL;
	if (from == to)
		return 0;
	/* it's safe to pass "from" here, but we may change it later */
	obj = new_obj_unconnected(ot_meas, from);
	connect_obj(frames, obj);
	meas = &obj->u.meas;
	meas->label = NULL;
	switch (mode) {
	case min_to_next_or_max:
		if (!is_max(meas_dsc->lt, to)) {
			meas->type = meas_dsc->type;
		} else {
			meas->type = meas_dsc->type+3;
		}
		obj->base = from->vec;
		meas->high = to->vec;
		break;
	case next_to_min:
		meas->type = meas_dsc->type;
		obj->base = to->vec;
		meas->high = from->vec;
		break;
	case max_to_min:
		meas->type = meas_dsc->type+3;
		obj->base = to->vec;
		meas->high = from->vec;
		break;
	default:
		abort();
	}
	meas->inverted =
	    mode == min_to_next_or_max && is_min(meas_dsc->lt, to) ? 0 :
	    meas_dsc->lt(from->u.vec.end, to->u.vec.end) !=
	    (mode == min_to_next_or_max);
	meas->offset = NULL;
	meas_dsc = NULL;
	/* we don't support qualifiers through the GUI yet */
	meas->low_qual = NULL;
	meas->high_qual = NULL;
	return 1;
}


static void cancel_drag_new_meas(void)
{
	meas_inst = NULL;
	highlight = NULL;
	redraw();
}


/* ----- begin dragging existing measurement ------------------------------- */


/*
 * We didn't record which instance provided the vector we're using here, so we
 * have to search for it now.
 */

static struct inst *vec_at(const struct vec *vec, struct coord pos)
{
	struct inst *inst;
	const struct sample *s;

	for (inst = insts_ip_vec(); inst; inst = inst->next)
		if (inst->vec == vec)
			for (s = active_pkg->samples[vec->n]; s; s = s->next)
				if (coord_eq(s->pos, pos))
					return inst;
	abort();
}


void begin_drag_move_meas(struct inst *inst, int i)
{
	const struct meas *meas = &inst->obj->u.meas;
	struct coord a, b;

	switch (meas->type) {
	case mt_xy_next:
	case mt_xy_max:
		meas_dsc = &meas_dsc_xy;
		break;
	case mt_x_next:
	case mt_x_max:
		meas_dsc = &meas_dsc_x;
		break;
	case mt_y_next:
	case mt_y_max:
		meas_dsc = &meas_dsc_y;
		break;
	default:
		abort();
	}
	highlight = meas_highlight_b;

	/*
	 * We're setting up the same conditions as after picking the first
	 * point when making a new measurement. Thus, we set meas_inst to the
	 * vector to the endpoint we're not moving.
	 */
	a = inst->base;
	b = inst->u.meas.end;
	if (inst->obj->u.meas.inverted)
		SWAP(a, b);
	switch (i) {
	case 0:
		mode = meas->type < 3 ? next_to_min : max_to_min;
		meas_inst = vec_at(inst->obj->u.meas.high, b);
		break;
	case 1:
		mode = min_to_next_or_max;
		meas_inst = vec_at(inst->obj->base, a);
		break;
	default:
		abort();
	}
//	redraw();
}


/* ----- find end point (existing measurement) ----------------------------- */


struct inst *find_point_meas_move(struct inst *inst, struct coord pos)
{
	return inst_find_vec(pos, is_highlighted, NULL);
}


/* ----- end dragging existing measurements -------------------------------- */


void end_drag_move_meas(void)
{
	highlight = NULL;
	redraw();
}


void do_move_to_meas(struct inst *inst, struct inst *to, int i)
{
	struct meas *meas = &inst->obj->u.meas;

	switch (i) {
	case 0:
		inst->obj->base = inst_get_vec(to);
		break;
	case 1:
		meas->high = inst_get_vec(to);
		if (is_max(meas_dsc->lt, to))
			meas->type = (meas->type % 3)+3;
		else
			meas->type = (meas->type % 3);
		break;
	default:
		abort();
	}
}


/* ----- operations -------------------------------------------------------- */


struct tool_ops tool_meas_ops = {
	.tool_selected	= tool_selected_meas_xy,
	.tool_deselected= tool_deselected_meas,
	.find_point	= find_point_meas_new,
	.begin_drag_new	= begin_drag_new_meas,
	.drag_new	= drag_new_line,
	.end_new	= end_new_meas,
	.cancel_drag_new= cancel_drag_new_meas,
};

struct tool_ops tool_meas_ops_x = {
	.tool_selected	= tool_selected_meas_x,
	.tool_deselected= tool_deselected_meas,
	.find_point	= find_point_meas_new,
	.begin_drag_new	= begin_drag_new_meas,
	.drag_new	= drag_new_line,
	.end_new	= end_new_meas,
	.cancel_drag_new= cancel_drag_new_meas,
};


struct tool_ops tool_meas_ops_y = {
	.tool_selected	= tool_selected_meas_y,
	.tool_deselected= tool_deselected_meas,
	.find_point	= find_point_meas_new,
	.begin_drag_new	= begin_drag_new_meas,
	.drag_new	= drag_new_line,
	.end_new	= end_new_meas,
	.cancel_drag_new= cancel_drag_new_meas,
};
