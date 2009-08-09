/*
 * gui_meas.c - GUI, canvas overlays
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include "util.h"
#include "meas.h"
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
	struct coord min;

	min = meas_find_min(lt, inst->vec->samples);
	return coord_eq(inst->u.rect.end, min);
}


static int is_next(lt_op_type lt,
    const struct inst *inst, const struct inst *ref)
{
	struct coord next;

	next = meas_find_next(lt, inst->vec->samples, ref->u.rect.end);
	return coord_eq(inst->u.rect.end, next);
}


static int is_max(lt_op_type lt, const struct inst *inst)
{
	struct coord max;

	max = meas_find_max(lt, inst->vec->samples);
	return coord_eq(inst->u.rect.end, max);
}


static int is_a_next(lt_op_type lt, struct inst *inst)
{
	struct inst *a;
	struct coord min, next;

	for (a = insts_ip_vec(); a; a = a->next) {
		min = meas_find_min(lt, a->vec->samples);
		next = meas_find_next(lt, inst->vec->samples, min);
		if (coord_eq(next, inst->u.rect.end))
			return 1;
	}
	return 0;
}


static int is_min_of_next(lt_op_type lt,
    const struct inst *inst, const struct inst *ref)
{
	struct coord min, next;

	min = meas_find_min(lt, inst->vec->samples);
	next = meas_find_next(lt, ref->vec->samples, min);
	return coord_eq(next, ref->u.rect.end);
}


/* ----- picker functions -------------------------------------------------- */


static int meas_pick_vec_a(struct inst *inst, void *ctx)
{
	struct vec *vec = inst->vec;

	if (!vec->samples)
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

	if (!vec->samples)
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
		return is_min_of_next(meas_dsc->lt, inst, a);
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
	return draw_move_line_common(inst, inst->u.meas.end, pos, i);
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


/* ----- find point ------------------------------------------------------- */


static struct inst *find_point_meas(struct coord pos)
{
	if (meas_inst)
		return inst_find_vec(pos, meas_pick_vec_b, meas_inst);
	else
		return inst_find_vec(pos, meas_pick_vec_a, NULL);
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
	if (from == to)
		return 0;
	/* it's safe to pass "from" here, but we may change it later */
	obj = new_obj(ot_meas, from);
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
	  meas_dsc->lt(from->u.rect.end, to->u.rect.end) !=
	  (mode == min_to_next_or_max);
{
char *sm[] = { "min_to", "max_to", "next_to" };
char *st[] = { "nxy", "nx", "ny", "mxy", "mx", "my" };
fprintf(stderr, "mode %s type %s, inverted %d\n",
sm[mode], st[meas->type], meas->inverted);
}
	meas->offset = NULL;
	meas_dsc = NULL;
	return 1;
}


/* ----- begin dragging existing measurement ------------------------------- */


/* ----- operations ------------------------------------------------------- */


struct tool_ops meas_ops = {
	.tool_selected	= tool_selected_meas_xy,
	.tool_deselected= tool_deselected_meas,
	.find_point	= find_point_meas,
	.begin_drag_new	= begin_drag_new_meas,
	.drag_new	= drag_new_line,
	.end_new	= end_new_meas,
};

struct tool_ops meas_ops_x = {
	.tool_selected	= tool_selected_meas_x,
	.tool_deselected= tool_deselected_meas,
	.find_point	= find_point_meas,
	.begin_drag_new	= begin_drag_new_meas,
	.drag_new	= drag_new_line,
	.end_new	= end_new_meas,
};


struct tool_ops meas_ops_y = {
	.tool_selected	= tool_selected_meas_y,
	.tool_deselected= tool_deselected_meas,
	.find_point	= find_point_meas,
	.begin_drag_new	= begin_drag_new_meas,
	.drag_new	= drag_new_line,
	.end_new	= end_new_meas,
};
