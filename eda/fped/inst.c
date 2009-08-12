/*
 * inst.c - Instance structures
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
#include <stdio.h>
#include <math.h>

#include "util.h"
#include "coord.h"
#include "expr.h"
#include "obj.h"
#include "delete.h"
#include "gui_util.h"
#include "gui_status.h"
#include "gui_tool.h"
#include "gui_meas.h"
#include "gui_inst.h"
#include "gui_frame.h"
#include "gui.h"
#include "inst.h"


struct inst *selected_inst = NULL;
struct inst *insts[ip_n];
struct bbox active_frame_bbox;

static struct inst *curr_frame = NULL;
static struct inst **next_inst[ip_n];
static struct inst *prev_insts[ip_n];

static unsigned long active_set = 0;


#define	IS_ACTIVE	((active_set & 1))


/* ----- selective visibility ---------------------------------------------- */


static int show(enum inst_prio prio)
{
	switch (prio) {
	case ip_vec:
	case ip_frame:
		return show_stuff;
	case ip_meas:
		return show_meas;
	default:
		return 1;
	}
}


/* ----- selection of items not on the canvas ------------------------------ */


static void *selected_outside = NULL;
static void (*outside_deselect)(void *item);


static void deselect_outside(void)
{
	if (selected_outside && outside_deselect)
		outside_deselect(selected_outside);
	selected_outside = NULL;
}


void inst_select_outside(void *item, void (*deselect)(void *item))
{
	if (item == selected_outside)
		return;
	deselect_outside();
	inst_deselect();
	selected_outside = item;
	outside_deselect = deselect;
}


/* ----- selection --------------------------------------------------------- */


static struct inst_ops vec_ops;
static struct inst_ops frame_ops;


static void set_path(int on)
{
	struct inst *inst;
return;
	if (inst->ops != &vec_ops && inst->ops != &frame_ops)
		return;
/* @@@ wrong */
	for (inst = selected_inst; inst; inst = inst->outer) {
		if (inst->ops != &vec_ops && inst->ops != &frame_ops)
			break;
		inst->in_path = on;
	}
}


static void inst_select_inst(struct inst *inst)
{
	selected_inst = inst;
	set_path(1);
	tool_selected_inst(inst);
	gui_frame_select_inst(inst);
	if (inst->ops->select)
		selected_inst->ops->select(inst);
}


int inst_select(struct coord pos)
{
	enum inst_prio prio;
	struct inst *inst;
	int best_dist = 0; /* keep gcc happy */
	int dist;

	deselect_outside();
	edit_nothing();
	if (selected_inst) {
		gui_frame_deselect_inst(selected_inst);
		tool_selected_inst(NULL);
	}
	selected_inst = NULL;
	FOR_INST_PRIOS_DOWN(prio) {
		if (!show(prio))
			continue;
		for (inst = insts[prio]; inst; inst = inst->next) {
			if (!inst->active || !inst->ops->distance)
				continue;
			dist = inst->ops->distance(inst, pos, draw_ctx.scale);
			if (dist >= 0 && (!selected_inst || best_dist > dist)) {
				selected_inst = inst;
				best_dist = dist;
			}
		}
		if (selected_inst)
			goto selected;
	}

	if (!show_stuff)
		return 0;

	/* give vectors a second chance */

	for (inst = insts[ip_vec]; inst; inst = inst->next) {
		if (!inst->active)
			continue;
		dist = gui_dist_vec_fallback(inst, pos, draw_ctx.scale);
		if (dist >= 0 && (!selected_inst || best_dist > dist)) {
			selected_inst = inst;
			best_dist = dist;
		}
	}
	
	if (!selected_inst)
		return 0;

selected:
	inst_select_inst(selected_inst);
	return 1;
}


struct inst *inst_find_point(struct coord pos)
{
	struct inst *inst, *found;
	int best_dist = 0; /* keep gcc happy */
	int dist;

	found = NULL;
	for (inst = insts[ip_frame]; inst; inst = inst->next) {
		if (!inst->u.frame.active)
			continue;
		dist = gui_dist_frame_eye(inst, pos, draw_ctx.scale);
		if (dist >= 0 && (!found || best_dist > dist)) {
			found = inst;
			best_dist = dist;
		}
	}
	if (found)
		return found;

	for (inst = insts[ip_vec]; inst; inst = inst->next) {
		if (!inst->active || !inst->ops->distance)
			continue;
		dist = inst->ops->distance(inst, pos, draw_ctx.scale);
		if (dist >= 0 && (!found || best_dist > dist)) {
			found = inst;
			best_dist = dist;
		}
	}
	return found;
}


int inst_find_point_selected(struct coord pos, struct inst **res)
{
	struct vec **anchors[3];
	int n, best_i, i;
	struct inst *best = NULL;
	struct inst *inst;
	int d_min, d;

	assert(selected_inst);
	n = inst_anchors(selected_inst, anchors);
	for (i = 0; i != n; i++) {
		if (*anchors[i]) {
			for (inst = insts[ip_vec]; inst; inst = inst->next) {
				if (inst->vec != *anchors[i])
					continue;
				d = gui_dist_vec(inst, pos, draw_ctx.scale);
				if (d != -1 && (!best || d < d_min)) {
					best = inst;
					best_i = i;
					d_min = d;
				}
			}
		} else {
			for (inst = insts[ip_frame]; inst; inst = inst->next) {
				if (inst != selected_inst->outer)
					continue;
				d = gui_dist_frame(inst, pos, draw_ctx.scale);
				if (d != -1 && (!best || d < d_min)) {
					best = inst;
					best_i = i;
					d_min = d;
				}
			}
		}
	}
	if (!best)
		return -1;
	if (res)
		*res = best;
	return best_i;
}


struct coord inst_get_point(const struct inst *inst)
{
	if (inst->ops == &vec_ops)
		return inst->u.rect.end;
	if (inst->ops == &frame_ops)
		return inst->base;
	abort();
}


struct vec *inst_get_vec(const struct inst *inst)
{
	if (inst->ops == &vec_ops)
		return inst->vec;
	if (inst->ops == &frame_ops)
		return NULL;
	abort();
}


int inst_anchors(struct inst *inst, struct vec ***anchors)
{
	return inst->ops->anchors ? inst->ops->anchors(inst, anchors) : 0;
}


void inst_deselect(void)
{
	if (selected_inst) {
		set_path(0);
		tool_selected_inst(NULL);
		gui_frame_deselect_inst(selected_inst);
	}
	deselect_outside();
	status_set_type_x("");
	status_set_type_y("");
	status_set_type_entry("");
	status_set_name("");
	status_set_x("");
	status_set_y("");
	status_set_r("");
	status_set_angle("");
	selected_inst = NULL;
	edit_nothing();
}


/* ----- select instance by vector/object ---------------------------------- */


void inst_select_vec(const struct vec *vec)
{
	struct inst *inst;

	for (inst = insts[ip_vec]; inst; inst = inst->next)
		if (inst->vec == vec && inst->active) {
			inst_deselect();
			inst_select_inst(inst);
			return;
		}
	
}


void inst_select_obj(const struct obj *obj)
{
	enum inst_prio prio;
	struct inst *inst;

	FOR_INSTS_DOWN(prio, inst)
		if (inst->obj && inst->obj == obj && inst->active) {
			inst_deselect();
			inst_select_inst(inst);
			return;
		}
}


/* ----- common status reporting ------------------------------------------- */


static void rect_status(struct coord a, struct coord b, unit_type width)
{
	struct coord d = sub_vec(b, a);
	double angle;
	
	status_set_xy(d);
	if (!d.x && !d.y)
		status_set_angle("a = 0 deg");
	else {
		angle = theta(a, b);
		status_set_angle("a = %3.1f deg", angle);
	}
	status_set_r("r = %5.2f mm", hypot(units_to_mm(d.x), units_to_mm(d.y)));
	if (width != -1) {
		status_set_type_entry("width =");
		status_set_name("%5.2f mm", units_to_mm(width));
	}
}


/* ----- helper functions for instance creation ---------------------------- */


static void update_bbox(struct bbox *bbox, struct coord coord)
{
	if (bbox->min.x > coord.x)
		bbox->min.x = coord.x;
	if (bbox->max.x < coord.x)
		bbox->max.x = coord.x;
	if (bbox->min.y > coord.y)
		bbox->min.y = coord.y;
	if (bbox->max.y < coord.y)
		bbox->max.y = coord.y;
}


static void propagate_bbox(const struct inst *inst)
{
	struct inst *frame = curr_frame ? curr_frame : insts[ip_frame];

	update_bbox(&frame->bbox, inst->bbox.min);
	update_bbox(&frame->bbox, inst->bbox.max);
}


static void grow_bbox_by_width(struct bbox *bbox, unit_type width)
{
	bbox->min.x -= width/2;
	bbox->min.y -= width/2;
	bbox->max.x += width/2;
	bbox->max.y += width/2;
}


static struct inst *add_inst(const struct inst_ops *ops, enum inst_prio prio,
    struct coord base)
{
	struct inst *inst;

	inst = alloc_type(struct inst);
	inst->ops = ops;
	inst->vec = NULL;
	inst->obj = NULL;
	inst->base = inst->bbox.min = inst->bbox.max = base;
	inst->outer = curr_frame;
	inst->active = IS_ACTIVE;
	inst->in_path = 0;
	inst->next = NULL;
	*next_inst[prio] = inst;
	next_inst[prio] = &inst->next;
	return inst;
}


/* ----- vec --------------------------------------------------------------- */


static void vec_op_debug(struct inst *self)
{
	printf("vec %lg, %lg -> %lg, %lg\n",
	    units_to_mm(self->base.x), units_to_mm(self->base.y),
	    units_to_mm(self->u.rect.end.x), units_to_mm(self->u.rect.end.y));
}


static int validate_vec_name(const char *s, void *ctx)
{
	struct vec *vec = ctx;
	const struct vec *walk;

	if (!is_id(s))
		return 0;
	for (walk = vec->frame->vecs; walk; walk = walk->next)
		if (walk->name && !strcmp(walk->name, s))
			return 0;
	return 1;
}


static void vec_op_select(struct inst *self)
{
	status_set_type_entry("ref =");
	status_set_name("%s", self->vec->name ? self->vec->name : "");
	rect_status(self->base, self->u.rect.end, -1);
	edit_x(&self->vec->x);
	edit_y(&self->vec->y);
	edit_unique_null(&self->vec->name, validate_vec_name, self->vec);
}


/*
 * @@@ The logic of gui_find_point_vec isn't great. Instead of selecting a
 * point and then filtering, we should filter the candidates, so that a point
 * that's close end eligible can win against one that's closer but not
 * eligible.
 */

static struct inst *find_point_vec(struct inst *self, struct coord pos)
{
	struct inst *inst;
	const struct vec *vec;

	inst = inst_find_point(pos);
	if (!inst)
		return NULL;
	if (inst->ops == &frame_ops)
		return inst;
	for (vec = inst->vec; vec; vec = vec->base)
		if (vec == self->vec)
		return NULL;
	return inst;
}


static int vec_op_anchors(struct inst *inst, struct vec ***anchors)
{
	anchors[0] = &inst->vec->base;
	return 1;
}


static struct inst_ops vec_ops = {
	.debug		= vec_op_debug,
	.draw		= gui_draw_vec,
	.hover		= gui_hover_vec,
	.distance	= gui_dist_vec,
	.select		= vec_op_select,
	.find_point	= find_point_vec,
	.anchors	= vec_op_anchors,
	.draw_move	= draw_move_vec,
};


int inst_vec(struct vec *vec, struct coord base)
{
	struct inst *inst;

	inst = add_inst(&vec_ops, ip_vec, base);
	inst->vec = vec;
	inst->u.rect.end = vec->pos;
	update_bbox(&inst->bbox, vec->pos);
	propagate_bbox(inst);
	return 1;
}


/* ----- line -------------------------------------------------------------- */


static void line_op_debug(struct inst *self)
{
	printf("line %lg, %lg / %lg, %lg\n",
	    units_to_mm(self->base.x), units_to_mm(self->base.y),
	    units_to_mm(self->u.rect.end.x), units_to_mm(self->u.rect.end.y));
}


static void line_op_select(struct inst *self)
{
	rect_status(self->bbox.min, self->bbox.max, self->u.rect.width);
	edit_expr(&self->obj->u.line.width);
}


static int line_op_anchors(struct inst *inst, struct vec ***anchors)
{
	struct obj *obj = inst->obj;

	anchors[0] = &obj->base;
	anchors[1] = &obj->u.rect.other;
	return 2;
}


static struct inst_ops line_ops = {
	.debug		= line_op_debug,
	.draw		= gui_draw_line,
	.distance	= gui_dist_line,
	.select		= line_op_select,
	.anchors	= line_op_anchors,
	.draw_move	= draw_move_line,
};


int inst_line(struct obj *obj, struct coord a, struct coord b, unit_type width)
{
	struct inst *inst;

	inst = add_inst(&line_ops, ip_line, a);
	inst->obj = obj;
	inst->u.rect.end = b;
	inst->u.rect.width = width;
	update_bbox(&inst->bbox, b);
	grow_bbox_by_width(&inst->bbox, width);
	propagate_bbox(inst);
	return 1;
}


/* ----- rect -------------------------------------------------------------- */


static void rect_op_debug(struct inst *self)
{
	printf("rect %lg, %lg / %lg, %lg\n",
	    units_to_mm(self->base.x), units_to_mm(self->base.y),
	    units_to_mm(self->u.rect.end.x), units_to_mm(self->u.rect.end.y));
}


static void rect_op_select(struct inst *self)
{
	rect_status(self->bbox.min, self->bbox.max, self->u.rect.width);
	edit_expr(&self->obj->u.rect.width);
}


static struct inst_ops rect_ops = {
	.debug		= rect_op_debug,
	.draw		= gui_draw_rect,
	.distance	= gui_dist_rect,
	.select		= rect_op_select,
	.anchors	= line_op_anchors,
	.draw_move	= draw_move_rect,
};


int inst_rect(struct obj *obj, struct coord a, struct coord b, unit_type width)
{
	struct inst *inst;

	inst = add_inst(&rect_ops, ip_rect, a);
	inst->obj = obj;
	inst->u.rect.end = b;
	inst->u.rect.width = width;
	update_bbox(&inst->bbox, b);
	grow_bbox_by_width(&inst->bbox, width);
	propagate_bbox(inst);
	return 1;
}


/* ----- pad --------------------------------------------------------------- */


static void pad_op_debug(struct inst *self)
{
	printf("pad \"%s\" %lg, %lg / %lg, %lg\n", self->u.name,
	    units_to_mm(self->base.x), units_to_mm(self->base.y),
	    units_to_mm(self->u.pad.other.x), units_to_mm(self->u.pad.other.y));
}


static int validate_pad_name(const char *s, void *ctx)
{
	char *tmp;

	tmp = expand(s, NULL);
	if (!tmp)
		return 0;
	free(tmp);
	return 1;
}


static void pad_op_select(struct inst *self)
{
	status_set_type_entry("label =");
	status_set_name("%s", self->u.pad.name);
	rect_status(self->base, self->u.pad.other, -1);
	edit_name(&self->obj->u.pad.name, validate_pad_name, NULL);
}


static int pad_op_anchors(struct inst *inst, struct vec ***anchors)
{
	struct obj *obj = inst->obj;

	anchors[0] = &obj->base;
	anchors[1] = &obj->u.pad.other;
	return 2;
}


static struct inst_ops pad_ops = {
	.debug		= pad_op_debug,
	.draw		= gui_draw_pad,
	.distance	= gui_dist_pad,
	.select		= pad_op_select,
	.anchors	= pad_op_anchors,
	.draw_move	= draw_move_pad,
};


int inst_pad(struct obj *obj, const char *name, struct coord a, struct coord b)
{
	struct inst *inst;

	inst = add_inst(&pad_ops, ip_pad, a);
	inst->obj = obj;
	inst->u.pad.name = stralloc(name);
	inst->u.pad.other = b;
	update_bbox(&inst->bbox, b);
	propagate_bbox(inst);
	return 1;
}


/* ----- arc --------------------------------------------------------------- */


static void arc_op_debug(struct inst *self)
{
	printf("arc %lg, %lg radius %lg %lg ... %lg\n",
	    units_to_mm(self->base.x), units_to_mm(self->base.y),
	    units_to_mm(self->u.arc.r), self->u.arc.a1, self->u.arc.a2);
}


static void arc_op_select(struct inst *self)
{
	status_set_xy(self->base);
	status_set_angle("a = %3.1f deg",
	    self->u.arc.a1 == self->u.arc.a2 ? 360 :
	    self->u.arc.a2-self->u.arc.a1);
	status_set_r("r = %5.2f mm", units_to_mm(self->u.arc.r));
	status_set_type_entry("width =");
	status_set_name("%5.2f mm", units_to_mm(self->u.arc.width));
	edit_expr(&self->obj->u.arc.width);
}


static int arc_op_anchors(struct inst *inst, struct vec ***anchors)
{
	struct obj *obj = inst->obj;

	anchors[0] = &obj->base;
	anchors[1] = &obj->u.arc.start;
	anchors[2] = &obj->u.arc.end;
	return 3;
}


static struct inst_ops arc_ops = {
	.debug		= arc_op_debug,
	.draw		= gui_draw_arc,
	.distance	= gui_dist_arc,
	.select		= arc_op_select,
	.anchors	= arc_op_anchors,
	.draw_move	= draw_move_arc,
	.do_move_to	= do_move_to_arc,
};


int inst_arc(struct obj *obj, struct coord center, struct coord start,
    struct coord end, unit_type width)
{
	struct inst *inst;
	double r, a1, a2;

	inst = add_inst(&arc_ops, ip_arc, center);
	inst->obj = obj;
	r = hypot(start.x-center.x, start.y-center.y);
	inst->u.arc.r = r;
	a1 = theta(center, start);
	a2 = theta(center, end);
	inst->u.arc.a1 = a1;
	inst->u.arc.a2 = a2;
	inst->u.arc.width = width;
	inst->bbox.min.x = center.x-r;
	inst->bbox.max.x = center.x+r;
	inst->bbox.min.y = center.y-r;
	inst->bbox.max.y = center.y+r;
	grow_bbox_by_width(&inst->bbox, width);
	propagate_bbox(inst);
	return 1;
}


/* ----- measurement ------------------------------------------------------- */


static void meas_op_debug(struct inst *self)
{
	printf("meas %lg, %lg / %lg, %lg offset %lg\n",
	    units_to_mm(self->base.x), units_to_mm(self->base.y),
	    units_to_mm(self->u.meas.end.x), units_to_mm(self->u.meas.end.y),
	    units_to_mm(self->u.meas.offset));
}


static void meas_op_select(struct inst *self)
{
	rect_status(self->bbox.min, self->bbox.max, -1);
	status_set_type_entry("offset =");
	status_set_name("%5.2f mm", units_to_mm(self->u.meas.offset));
	edit_expr(&self->obj->u.meas.offset);
}


static int meas_op_anchors(struct inst *inst, struct vec ***anchors)
{
	struct obj *obj = inst->obj;

	anchors[0] = &obj->base;
	anchors[1] = &obj->u.meas.high;
	return 2;
}


static struct inst_ops meas_ops = {
	.debug		= meas_op_debug,
	.draw		= gui_draw_meas,
	.distance	= gui_dist_meas,
	.select		= meas_op_select,
	.anchors	= meas_op_anchors,
	.begin_drag_move= begin_drag_move_meas,
	.find_point	= find_point_meas_move,
	.draw_move	= draw_move_meas,
	.end_drag_move	= end_drag_move_meas,
	.do_move_to	= do_move_to_meas,
};


int inst_meas(struct obj *obj,
    struct coord from, struct coord to, unit_type offset)
{
	struct inst *inst;

	inst = add_inst(&meas_ops, ip_meas, from);
	inst->obj = obj;
	inst->u.meas.end = to;
	inst->u.meas.offset = offset;
	inst->active = 1; /* measurements are always active */
	/* @@@ our bbox is actually a bit more complex than this */
	update_bbox(&inst->bbox, to);
	propagate_bbox(inst);
	return 1;
}


/* ----- active instance --------------------------------------------------- */


void inst_begin_active(int active)
{
	active_set = (active_set << 1) | active;
}


void inst_end_active(void)
{
	active_set >>= 1;
}


/* ----- frame ------------------------------------------------------------- */


static void frame_op_debug(struct inst *self)
{
	printf("frame %s @ %lg, %lg\n",
	    self->u.frame.ref->name ? self->u.frame.ref->name : "(root)",
	    units_to_mm(self->base.x), units_to_mm(self->base.y));
}


static void frame_op_select(struct inst *self)
{
	rect_status(self->bbox.min, self->bbox.max, -1);
	status_set_type_entry("name =");
	status_set_name("%s", self->u.frame.ref->name);
}


static int frame_op_anchors(struct inst *inst, struct vec ***anchors)
{
	anchors[0] = &inst->obj->base;
	return 1;
}


static struct inst_ops frame_ops = {
	.debug		= frame_op_debug,
	.draw		= gui_draw_frame,
	.hover		= gui_hover_frame,
	.distance	= gui_dist_frame,
	.select		= frame_op_select,
	.anchors	= frame_op_anchors,
	.draw_move	= draw_move_frame,
};


void inst_begin_frame(struct obj *obj, const struct frame *frame,
    struct coord base, int active, int is_active_frame)
{
	struct inst *inst;

	inst = add_inst(&frame_ops, ip_frame, base);
	inst->obj = obj;
	inst->u.frame.ref = frame;
	inst->u.frame.active = is_active_frame;
	inst->active = active;
	curr_frame = inst;
}


void inst_end_frame(const struct frame *frame)
{
	struct inst *inst = curr_frame;

	curr_frame = curr_frame->outer;
	if (curr_frame)
		propagate_bbox(inst);
	if (inst->u.frame.active && frame == active_frame)
		active_frame_bbox = inst->bbox;
}


/* ----- misc. ------------------------------------------------------------- */


struct bbox inst_get_bbox(void)
{
	return insts[ip_frame]->bbox;
}


static void inst_free(struct inst *list[ip_n])
{
	enum inst_prio prio;
	struct inst *next;

	FOR_INST_PRIOS_UP(prio)
		while (list[prio]) {
			next = list[prio]->next;
			free(list[prio]);
			list[prio] = next;
		}
}


void inst_start(void)
{
	static struct bbox bbox_zero = { { 0, 0 }, { 0, 0 }};
	enum inst_prio prio;

	active_frame_bbox = bbox_zero;
	FOR_INST_PRIOS_UP(prio) {
		prev_insts[prio] = insts[prio];
		insts[prio] = NULL;
		next_inst[prio] = &insts[prio];
	}
}


void inst_commit(void)
{
	inst_free(prev_insts);
}


void inst_revert(void)
{
	enum inst_prio prio;

	inst_free(insts);
	FOR_INST_PRIOS_UP(prio)
		insts[prio] = prev_insts[prio];
}


void inst_draw(void)
{
	enum inst_prio prio;
	struct inst *inst;

	FOR_INSTS_UP(prio, inst)
		if (show(prio) && !inst->active && inst->ops->draw)
			inst->ops->draw(inst);
	FOR_INSTS_UP(prio, inst)
		if (show(prio) && prio != ip_frame && inst->active &&
		    inst != selected_inst && inst->ops->draw)
			inst->ops->draw(inst);
	if (show_stuff)
		for (inst = insts[ip_frame]; inst; inst = inst->next)
			if (inst->active && inst != selected_inst &&
			    inst->ops->draw)
				inst->ops->draw(inst);
	if (selected_inst && selected_inst->ops->draw)
		selected_inst->ops->draw(selected_inst);
}


void inst_highlight_vecs(int (*pick)(struct inst *inst, void *user), void *user)
{
	struct inst *inst;

	for (inst = insts[ip_vec]; inst; inst = inst->next)
		if (pick(inst, user))
			gui_highlight_vec(inst);
}


struct inst *inst_find_vec(struct coord pos,
    int (*pick)(struct inst *inst, void *user), void *user)
{
	struct inst *inst, *found;
	int best_dist = 0; /* keep gcc happy */
	int dist;

	found = NULL;
	for (inst = insts[ip_vec]; inst; inst = inst->next) {
		if (!inst->ops->distance)
			continue;
		dist = inst->ops->distance(inst, pos, draw_ctx.scale);
		if (dist < 0 || (found && best_dist <= dist))
			continue;
		if (!pick(inst, user))
			continue;
		found = inst;
		best_dist = dist;
	}
	return found;
}


struct inst *insts_ip_vec(void)
{
	return insts[ip_vec];
}


struct pix_buf *inst_draw_move(struct inst *inst, struct coord pos, int i)
{
	return inst->ops->draw_move(inst, pos, i);
}


int inst_do_move_to(struct inst *inst, struct inst *to, int i)
{
	if (!inst->ops->do_move_to)
		return 0;
	inst->ops->do_move_to(inst, to, i);
	return 1;
}


struct pix_buf *inst_hover(struct inst *inst)
{
	if (!inst->ops->hover)
		return NULL;
	return inst->ops->hover(inst);
}


void inst_begin_drag_move(struct inst *inst, int i)
{
	if (inst->ops->begin_drag_move)
		inst->ops->begin_drag_move(inst, i);
}


void inst_delete(struct inst *inst)
{
	if (inst->ops == &vec_ops)
		delete_vec(inst->vec);
	else
		delete_obj(inst->obj);
}


void inst_debug(void)
{
	enum inst_prio prio;
	struct inst *inst;

	FOR_INSTS_UP(prio, inst)
		inst->ops->debug(inst);
}
