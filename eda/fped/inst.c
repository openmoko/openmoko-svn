/*
 * inst.c - Instance structures
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
#include <stdio.h>
#include <math.h>

#include "util.h"
#include "coord.h"
#include "expr.h"
#include "layer.h"
#include "obj.h"
#include "delete.h"
#include "gui_util.h"
#include "gui_status.h"
#include "gui_canvas.h"
#include "gui_tool.h"
#include "gui_meas.h"
#include "gui_inst.h"
#include "gui_frame.h"
#include "gui.h"
#include "inst.h"


struct inst *selected_inst = NULL;
struct bbox active_frame_bbox;
struct pkg *pkgs, *active_pkg, *curr_pkg;
struct inst *curr_frame = NULL;

static struct pkg *prev_pkgs;

static unsigned long active_set = 0;

static struct inst_ops vec_ops;
static struct inst_ops frame_ops;
static struct inst_ops meas_ops;


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


int bright(const struct inst *inst)
{
	if (!show_bright)
		return 0;
	return inst->ops != &vec_ops && inst->ops != &frame_ops &&
	    inst->ops != &meas_ops;
}


static int show_this(const struct inst *inst)
{
	if (show_all)
		return 1;
	if (inst->ops == &frame_ops && inst->u.frame.ref == active_frame)
		return 1;
	if (!inst->outer)
		return active_frame == root_frame;
	return inst->outer->u.frame.ref == active_frame;
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


/* ----- check connectedness ----------------------------------------------- */


/*
 * After an instantiation failure, the instances can get out of sync with the
 * object tree, and attempts to select an item on the canvas can cause accesses
 * to objects that aren't there anymore. So we need to check if we can still
 * reach the corresponding object.
 *
 * Note: even this isn't bullet-proof. Theoretically, we may get a new object
 * in the old place. However, this probably doesn't do any serious damage.
 */


static int inst_connected(const struct inst *inst)
{
	const struct frame *frame;
	const struct vec *vec;
	const struct obj *obj;

	for (frame = frames; frame; frame = frame->next) {
		if (inst->ops == &vec_ops) {
			for (vec = frame->vecs; vec; vec = vec->next)
				if (vec == inst->vec)
					return 1;
		} else {
			for (obj = frame->objs; obj; obj = obj->next)
				if (obj == inst->obj)
					return 1;
		}
	}
	return 0;
}


/* ----- selection --------------------------------------------------------- */


static void inst_select_inst(struct inst *inst)
{
	selected_inst = inst;
	tool_selected_inst(inst);
	gui_frame_select_inst(inst);
	if (inst->ops->select)
		selected_inst->ops->select(inst);
	status_set_icon(get_icon_by_inst(inst));
}


static int activate_item(struct inst *inst)
{
	if (!inst->outer)
		return 0;
	if (inst->outer->u.frame.ref->active_ref == inst->outer->obj)
		return activate_item(inst->outer);
	inst->outer->u.frame.ref->active_ref = inst->outer->obj;
	activate_item(inst->outer);
	return 1;
}


int inst_select(struct coord pos)
{
	enum inst_prio prio;
	const struct inst *prev;
	struct inst *inst;
	struct inst *first = NULL;	/* first active item */
	struct inst *next = NULL;	/* active item after currently sel. */
	struct inst *any_first = NULL;	/* first item, active or inactive */
	struct inst *any_same_frame = NULL; /* first item on active frame */
	struct frame *frame;
	int best_dist = 0; /* keep gcc happy */
	int select_next;
	int dist, i;

	prev = selected_inst;
	deselect_outside();
	edit_nothing();
	if (selected_inst) {
		gui_frame_deselect_inst(selected_inst);
		tool_selected_inst(NULL);
	}
	inst_deselect();
	select_next = 0;
	FOR_INST_PRIOS_DOWN(prio) {
		if (!show(prio))
			continue;
		FOR_ALL_INSTS(i, prio, inst) {
			if (!show_this(inst))
				continue;
			if (!inst->ops->distance)
				continue;
			if (!inst_connected(inst))
				continue;
			dist = inst->ops->distance(inst, pos, draw_ctx.scale);
			if (dist >= 0) {
				if (!any_first)
					any_first = inst;
				if (!any_same_frame && inst->outer &&
				    inst->outer->u.frame.ref == active_frame)
					any_same_frame = inst;
				if (!inst->active)
					continue;
				if (!first)
					first = inst;
				if (!next && select_next)
					next = inst;
				if (inst == prev)
					select_next = 1;
				if (!selected_inst || best_dist > dist) {
					selected_inst = inst;
					best_dist = dist;
				}
			}
		}
	}
	if (select_next) {
		selected_inst = next ? next : first;
		goto selected;
	}
	if (selected_inst)
		goto selected;

	/* give vectors a second chance */

	if (show_stuff) {
		FOR_ALL_INSTS(i, ip_vec, inst) {
			if (!inst->active)
				continue;
			if (!inst_connected(inst))
				continue;
			dist = gui_dist_vec_fallback(inst, pos, draw_ctx.scale);
			if (dist >= 0 && (!selected_inst || best_dist > dist)) {
				selected_inst = inst;
				best_dist = dist;
			}
		}
	
		if (selected_inst)
			goto selected;
	}

	if (!show_all)
		return 0;

	if (any_same_frame) {
		if (activate_item(any_same_frame))
			return inst_select(pos);
	}
	if (any_first) {
		frame = any_first->outer ? any_first->outer->u.frame.ref : NULL;
		if (frame != active_frame) {
			select_frame(frame);
			return inst_select(pos);
		}
	}

	return 0;

selected:
	inst_select_inst(selected_inst);
	return 1;
}


struct inst *inst_find_point(struct coord pos)
{
	struct inst *inst, *found;
	int best_dist = 0; /* keep gcc happy */
	int dist, i;

	found = NULL;
	FOR_ALL_INSTS(i, ip_frame, inst) {
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

	FOR_ALL_INSTS(i, ip_vec, inst) {
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
	int d_min, d, j;

	assert(selected_inst);
	n = inst_anchors(selected_inst, anchors);
	for (i = 0; i != n; i++) {
		if (*anchors[i]) {
			FOR_ALL_INSTS(j, ip_vec, inst) {
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
			FOR_ALL_INSTS(j, ip_frame, inst) {
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
		return inst->u.vec.end;
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
		tool_selected_inst(NULL);
		gui_frame_deselect_inst(selected_inst);
	}
	deselect_outside();
	status_set_type_x(NULL, "");
	status_set_type_y(NULL, "");
	status_set_type_entry(NULL, "");
	status_set_name(NULL, "");
	status_set_x(NULL, "");
	status_set_y(NULL, "");
	status_set_r(NULL, "");
	status_set_angle(NULL, "");
	selected_inst = NULL;
	edit_nothing();
	refresh_pos();
	status_set_icon(NULL);
}


/* ----- select instance by vector/object ---------------------------------- */


static void vec_edit(struct vec *vec);
static void obj_edit(struct obj *obj);


void inst_select_vec(struct vec *vec)
{
	struct inst *inst;
	int i;

	if (vec->frame != active_frame)
		select_frame(vec->frame);
	FOR_ALL_INSTS(i, ip_vec, inst)
		if (inst->vec == vec && inst->active) {
			inst_deselect();
			inst_select_inst(inst);
			return;
		}
	vec_edit(vec);
}


void inst_select_obj(struct obj *obj)
{
	enum inst_prio prio;
	struct inst *inst;
	int i;

	if (obj->frame != active_frame)
		select_frame(obj->frame);
	FOR_INST_PRIOS_DOWN(prio)
		FOR_ALL_INSTS(i, prio, inst)
			if (inst->obj && inst->obj == obj && inst->active)
				goto found;
	obj_edit(obj);
	return;

found:
	inst_deselect();
	inst_select_inst(inst);
}


/* ----- common status reporting ------------------------------------------- */


static void rect_status(struct coord a, struct coord b, unit_type width,
    int rounded)
{
	const char *tip;
	struct coord d = sub_vec(b, a);
	double r;
	unit_type diag;

	status_set_xy(d);
	tip = "Angle of diagonal";
	if (!d.x && !d.y)
		status_set_angle(tip, "a = 0 deg");
	else {
		status_set_angle(tip, "a = %3.1f deg", theta(a, b));
	}
	if (d.x < 0)
		d.x = -d.x;
	if (d.y < 0)
		d.y = -d.y;
	diag = hypot(d.x, d.y);
	if (rounded) {
		/*
		 * Only consider the part of the diagonal that is on the pad
		 * surface.
		 *
		 * The circle: (x-r)^2+(y-r)^2 = r^2
		 * The diagonal: x = t*cos(theta), y = t*sin(theta)
		 *
		 * t is the distance from the corner of the surrounding
		 * rectangle to the half-circle:
		 *
		 * t = 2*r*(s+c-sqrt(2*s*c))
		 *
		 * With s = sin(theta) and c = cos(theta).
		 *
		 * Since d.x = diag*cos(theta), we don't need to calculate the
		 * sinus and cosinus but can use d.x and d.y directly.
		 */
		r = (d.x > d.y ? d.y : d.x)/2;
		diag -= 2*r*(d.x+d.y-sqrt(2*d.x*d.y))/diag;
	}
	set_with_units(status_set_r, "d = ", diag, "Length of diagonal");
	if (width != -1) {
		status_set_type_entry(NULL, "width =");
		set_with_units(status_set_name, "", width, "Line width");
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
	struct inst *frame =
	    curr_frame ? curr_frame : curr_pkg->insts[ip_frame];

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
	inst->prio = prio;
	inst->vec = NULL;
	inst->obj = NULL;
	inst->base = inst->bbox.min = inst->bbox.max = base;
	inst->outer = curr_frame;
	inst->active = IS_ACTIVE;
	inst->next = NULL;
	*curr_pkg->next_inst[prio] = inst;
	curr_pkg->next_inst[prio] = &inst->next;
	return inst;
}


/* ----- vec --------------------------------------------------------------- */


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


static void vec_edit(struct vec *vec)
{
	edit_x(&vec->x, "X distance");
	edit_y(&vec->y, "Y distance");
	edit_unique_null(&vec->name, validate_vec_name, vec, "Vector name");
}


static void vec_op_select(struct inst *self)
{
	status_set_type_entry(NULL, "ref =");
	status_set_name("Vector reference (name)",
	    "%s", self->vec->name ? self->vec->name : "");
	rect_status(self->base, self->u.vec.end, -1, 0);
	vec_edit(self->vec);
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


/*
 * When instantiating and when dumping, we assume that bases appear in the
 * frame->vecs list before vectors using them. A move may change this order.
 * We therefore have to sort the list after the move.
 *
 * Since the list is already ordered, cleaning it up is just O(n).
 */


static void do_move_to_vec(struct inst *inst, struct inst *to, int i)
{
	struct vec *to_vec = inst_get_vec(to);
	struct vec *vec = inst->vec;
	struct frame *frame = vec->frame;
	struct vec *v, **anchor, **walk;

	assert(!i);
	vec->base = to_vec;

	/*
	 * Mark the vector that's being rebased and all vectors that
	 * (recursively) depend on it.
	 *
	 * We're mainly interested in the range between the vector being moved
	 * and the new base. If the vector follows the base, the list is
	 * already in the correct order and nothing needs moving.
	 */
	for (v = frame->vecs; v != vec; v = v->next)
		v->mark = 0;
	vec->mark = 1;
	for (v = vec->next; v && v != to_vec; v = v->next)
		v->mark = v->base ? v->base->mark : 0;
	if (!v)
		return;

	/*
	 * All the marked vectors appearing on the list before the new base
	 * are moved after the new base, preserving their order.
	 *
	 * Start at frame->vecs, not "vec", so that we move the the vector
	 * being rebased as well.
	 */
	anchor = &to_vec->next;
	walk = &frame->vecs;
	while (*walk != to_vec) {
		v = *walk;
		if (!v->mark)
			walk = &v->next;
		else {
			*walk = v->next;
			v->next = *anchor;
			*anchor = v;
			anchor = &v->next;
		}
	}
}


static struct inst_ops vec_ops = {
	.draw		= gui_draw_vec,
	.hover		= gui_hover_vec,
	.distance	= gui_dist_vec,
	.select		= vec_op_select,
	.find_point	= find_point_vec,
	.anchors	= vec_op_anchors,
	.draw_move	= draw_move_vec,
	.do_move_to	= do_move_to_vec,
};


int inst_vec(struct vec *vec, struct coord base)
{
	struct inst *inst;

	inst = add_inst(&vec_ops, ip_vec, base);
	inst->vec = vec;
	inst->u.vec.end = vec->pos;
	update_bbox(&inst->bbox, vec->pos);
	propagate_bbox(inst);
	return 1;
}


/* ----- line -------------------------------------------------------------- */


static void obj_line_edit(struct obj *obj)
{
	edit_expr(&obj->u.line.width, "Line width");
}


static void line_op_select(struct inst *self)
{
	rect_status(self->bbox.min, self->bbox.max, self->u.rect.width, 0);
	obj_line_edit(self->obj);
}


static int line_op_anchors(struct inst *inst, struct vec ***anchors)
{
	struct obj *obj = inst->obj;

	anchors[0] = &obj->base;
	anchors[1] = &obj->u.rect.other;
	return 2;
}


static struct inst_ops line_ops = {
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


static void obj_rect_edit(struct obj *obj)
{
	edit_expr(&obj->u.rect.width, "Line width");
}


static void rect_op_select(struct inst *self)
{
	rect_status(self->bbox.min, self->bbox.max, self->u.rect.width, 0);
	obj_rect_edit(self->obj);
}


static struct inst_ops rect_ops = {
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


/* ----- pad / rpad -------------------------------------------------------- */


static int validate_pad_name(const char *s, void *ctx)
{
	char *tmp;

	status_begin_reporting();
	tmp = expand(s, NULL);
	if (!tmp)
		return 0;
	free(tmp);
	return 1;
}


static void obj_pad_edit(struct obj *obj)
{
	edit_pad_type(&obj->u.pad.type);
	edit_name(&obj->u.pad.name, validate_pad_name, NULL,
	    "Pad name (template)");
}


static void pad_op_select(struct inst *self)
{
	status_set_type_entry(NULL, "label =");
	status_set_name("Pad name (actual)", "%s", self->u.pad.name);
	rect_status(self->base, self->u.pad.other, -1, 0);
	obj_pad_edit(self->obj);
}


static int pad_op_anchors(struct inst *inst, struct vec ***anchors)
{
	struct obj *obj = inst->obj;

	anchors[0] = &obj->base;
	anchors[1] = &obj->u.pad.other;
	return 2;
}


static struct inst_ops pad_ops = {
	.draw		= gui_draw_pad,
	.distance	= gui_dist_pad,
	.select		= pad_op_select,
	.anchors	= pad_op_anchors,
	.draw_move	= draw_move_pad,
};


static void rpad_op_select(struct inst *self)
{
	status_set_type_entry(NULL, "label =");
	status_set_name("Pad name (actual)", "%s", self->u.pad.name);
	rect_status(self->base, self->u.pad.other, -1, 1);
	obj_pad_edit(self->obj);
}


static struct inst_ops rpad_ops = {
	.draw		= gui_draw_rpad,
	.distance	= gui_dist_pad, /* @@@ */
	.select		= rpad_op_select,
	.anchors	= pad_op_anchors,
	.draw_move	= draw_move_rpad,
};


int inst_pad(struct obj *obj, const char *name, struct coord a, struct coord b)
{
	struct inst *inst;

	inst = add_inst(obj->u.pad.rounded ? &rpad_ops : &pad_ops,
	    obj->u.pad.type == pt_normal || obj->u.pad.type == pt_bare ?
	    ip_pad_copper : ip_pad_special, a);
	inst->obj = obj;
	inst->u.pad.name = stralloc(name);
	inst->u.pad.other = b;
	inst->u.pad.layers = pad_type_to_layers(obj->u.pad.type);
	update_bbox(&inst->bbox, b);
	propagate_bbox(inst);
	return 1;
}


/* ----- arc --------------------------------------------------------------- */


static void obj_arc_edit(struct obj *obj)
{
	edit_expr(&obj->u.arc.width, "Line width");
}


static void arc_op_select(struct inst *self)
{
	status_set_xy(self->base);
	status_set_angle("Angle", "a = %3.1f deg",
	    self->u.arc.a1 == self->u.arc.a2 ? 360 :
	    self->u.arc.a2-self->u.arc.a1);
	set_with_units(status_set_r, "r = ", self->u.arc.r, "Radius");
	status_set_type_entry(NULL, "width =");
	set_with_units(status_set_name, "", self->u.arc.width, "Line width");
	obj_arc_edit(self->obj);
}


static int arc_op_anchors(struct inst *inst, struct vec ***anchors)
{
	struct obj *obj = inst->obj;

	/*
	 * Put end point first so that this is what we grab if dragging a
	 * circle (thereby turning it into an arc).
	 */
	anchors[0] = &obj->base;
	anchors[1] = &obj->u.arc.end;
	anchors[2] = &obj->u.arc.start;
	return 3;
}


static struct inst_ops arc_ops = {
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

	a1 = theta(center, start);
	a2 = theta(center, end);
	inst = add_inst(&arc_ops,
	    fmod(a1, 360) == fmod(a2, 360) ? ip_circ : ip_arc, center);
	inst->obj = obj;
	r = hypot(start.x-center.x, start.y-center.y);
	inst->u.arc.r = r;
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


static void obj_meas_edit(struct obj *obj)
{
	edit_expr(&obj->u.meas.offset, "Measurement line offset");
}


static void meas_op_select(struct inst *self)
{
	rect_status(self->bbox.min, self->bbox.max, -1, 0);
	status_set_type_entry(NULL, "offset =");
	set_with_units(status_set_name, "", self->u.meas.offset,
	    "Measurement line offset");
	obj_meas_edit(self->obj);
}


static int meas_op_anchors(struct inst *inst, struct vec ***anchors)
{
	struct obj *obj = inst->obj;

	anchors[0] = &obj->base;
	anchors[1] = &obj->u.meas.high;
	return 2;
}


static struct inst_ops meas_ops = {
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


static struct inst *find_meas_hint(const struct obj *obj)
{
	struct inst *inst;

	for (inst = curr_pkg->insts[ip_meas]; inst; inst = inst->next)
		if (inst->obj == obj)
			break;
	return inst;
}


int inst_meas(struct obj *obj, struct coord from, struct coord to)
{
	struct inst *inst;
	struct coord a1, b1;

	inst = find_meas_hint(obj);
	assert(inst);
	inst->base = from;
	inst->u.meas.end = to;
	/* @@@ we still need to consider the text size as well */
	update_bbox(&inst->bbox, from);
	update_bbox(&inst->bbox, to);
	project_meas(inst, &a1, &b1);
	update_bbox(&inst->bbox, a1);
	update_bbox(&inst->bbox, b1);
	propagate_bbox(inst);
	return 1;
}


void inst_meas_hint(struct obj *obj, unit_type offset)
{
	static const struct coord zero = { 0, 0 };
	struct inst *inst;

	inst = find_meas_hint(obj);
	if (inst)
		return;
	inst = add_inst(&meas_ops, ip_meas, zero);
	inst->obj = obj;
	inst->u.meas.offset = offset;
	inst->active = 1; /* measurements are always active */
}


/* ----- direct editing of objects ----------------------------------------- */


static void obj_edit(struct obj *obj)
{
	switch (obj->type) {
	case ot_frame:
		break;
	case ot_line:
		obj_line_edit(obj);
		break;
	case ot_rect:
		obj_rect_edit(obj);
		break;
	case ot_arc:
		obj_arc_edit(obj);
		break;
	case ot_pad:
		obj_pad_edit(obj);
		break;
	case ot_meas:
		obj_meas_edit(obj);
		break;
	default:
		abort();
	}
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


static void frame_op_select(struct inst *self)
{
	rect_status(self->bbox.min, self->bbox.max, -1, 0);
	status_set_type_entry(NULL, "name =");
	status_set_name("Frame name", "%s", self->u.frame.ref->name);
}


static int frame_op_anchors(struct inst *inst, struct vec ***anchors)
{
	anchors[0] = &inst->obj->base;
	return 1;
}


static struct inst_ops frame_ops = {
	.draw		= gui_draw_frame,
	.hover		= gui_hover_frame,
	.distance	= gui_dist_frame,
	.select		= frame_op_select,
	.anchors	= frame_op_anchors,
	.draw_move	= draw_move_frame,
};


void inst_begin_frame(struct obj *obj, struct frame *frame,
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


/* ----- package ----------------------------------------------------------- */


void inst_select_pkg(const char *name)
{
	struct pkg **pkg;
	enum inst_prio prio;

	name = name ? unique(name) : NULL;
	for (pkg = &pkgs; *pkg; pkg = &(*pkg)->next)
		if ((*pkg)->name == name)
			break;
	if (!*pkg) {
		*pkg = zalloc_type(struct pkg);
		(*pkg)->name = name;
		FOR_INST_PRIOS_UP(prio)
			(*pkg)->next_inst[prio] = &(*pkg)->insts[prio];
		(*pkg)->samples =
		    zalloc_size(sizeof(struct sample *)*n_samples);
		(*pkg)->n_samples = n_samples;
	}
	curr_pkg = *pkg;
}


/* ----- misc. ------------------------------------------------------------- */


struct bbox inst_get_bbox(void)
{
	return pkgs->insts[ip_frame]->bbox;
}


static void cleanup_inst(enum inst_prio prio, const struct inst *inst)
{
	switch (prio) {
	case ip_pad_copper:
	case ip_pad_special:
		free(inst->u.pad.name);
		break;
	default:
		break;
	}
}


static void free_pkgs(struct pkg *pkg)
{
	enum inst_prio prio;
	struct pkg *next_pkg;
	struct inst *inst, *next;

	while (pkg) {
		next_pkg = pkg->next;
		FOR_INST_PRIOS_UP(prio)
			for (inst = pkg->insts[prio]; inst; inst = next) {
				next = inst->next;
				cleanup_inst(prio, inst);
				free(inst);
			}
		reset_samples(pkg->samples, pkg->n_samples);
		free(pkg->samples);
		free(pkg);
		pkg = next_pkg;
	}
}


void inst_start(void)
{
	static struct bbox bbox_zero = { { 0, 0 }, { 0, 0 }};

	active_frame_bbox = bbox_zero;
	prev_pkgs = pkgs;
	pkgs = NULL;
	inst_select_pkg(NULL);
	curr_pkg = pkgs;
	curr_frame = NULL;
}


void inst_commit(void)
{
	struct pkg *pkg;

	if (active_pkg) {
		for (pkg = pkgs; pkg && pkg->name != active_pkg->name;
		    pkg = pkg->next);
		active_pkg = pkg;
	}
	if (!active_pkg)
		active_pkg = pkgs->next;
	free_pkgs(prev_pkgs);
}


void inst_revert(void)
{
	free_pkgs(pkgs);
	pkgs = prev_pkgs;
}


void inst_draw(void)
{
	enum inst_prio prio;
	struct inst *inst;
	int i;

	FOR_INST_PRIOS_UP(prio)
		FOR_ALL_INSTS(i, prio, inst)
			if (show_this(inst))
				if (show(prio) && !inst->active &&
				    inst->ops->draw)
					inst->ops->draw(inst);
	FOR_INST_PRIOS_UP(prio)
		FOR_ALL_INSTS(i, prio, inst)
			if (show(prio) && prio != ip_frame && inst->active &&
			    inst != selected_inst && inst->ops->draw)
				inst->ops->draw(inst);
	if (show_stuff)
		FOR_ALL_INSTS(i, ip_frame, inst)
			if (inst->active && inst != selected_inst &&
			    inst->ops->draw)
				inst->ops->draw(inst);
	if (selected_inst && selected_inst->ops->draw)
		selected_inst->ops->draw(selected_inst);
}


void inst_highlight_vecs(int (*pick)(struct inst *inst, void *user), void *user)
{
	struct inst *inst;
	int i;

	FOR_ALL_INSTS(i, ip_vec, inst) {
		inst->u.vec.highlighted = pick(inst, user);
		if (inst->u.vec.highlighted)
			gui_highlight_vec(inst);
	}
}


struct inst *inst_find_vec(struct coord pos,
    int (*pick)(struct inst *inst, void *user), void *user)
{
	struct inst *inst, *found;
	int best_dist = 0; /* keep gcc happy */
	int dist, i;

	found = NULL;
	FOR_ALL_INSTS(i, ip_vec, inst) {
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
	return active_pkg->insts[ip_vec];
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
