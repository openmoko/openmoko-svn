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
#include "gui_status.h"
#include "gui_status.h"
#include "gui_inst.h"
#include "inst.h"


struct inst_ops {
	void (*debug)(struct inst *self);
	void (*save)(FILE *file, struct inst *self);
	void (*draw)(struct inst *self, struct draw_ctx *ctx);
	void (*hover)(struct inst *self, struct draw_ctx *ctx);
	unit_type (*distance)(struct inst *self, struct coord pos, 
	    unit_type scale);
	void (*select)(struct inst *self);
	int (*anchors)(struct inst *self, struct vec ***anchors);
};

enum inst_prio {
	ip_frame,	/* frames have their own selection */
	ip_pad,		/* pads also accept clicks inside */
	ip_circ,	/* circles don't overlap easily */
	ip_arc,		/* arc are like circles, just shorter */
	ip_rect,	/* rectangles have plenty of sides */
	ip_meas,	/* mesurements are like lines but set a bit apart */
	ip_line,	/* lines are easly overlapped by other things */
	ip_vec,		/* vectors only have the end point */
	ip_n,		/* number of priorities */
};


#define FOR_INST_PRIOS_UP(prio)					\
	for (prio = 0; prio != ip_n; prio++)

#define FOR_INST_PRIOS_DOWN(prio)				\
	for (prio = ip_n-1; prio != (enum inst_prio) -1; prio--)

#define	FOR_INSTS_UP(prio, inst)				\
	FOR_INST_PRIOS_UP(prio)					\
		for (inst = insts[prio]; inst; inst = inst->next)

#define	FOR_INSTS_DOWN(prio, inst)				\
	FOR_INST_PRIOS_DOWN(prio)				\
		for (inst = insts[prio]; inst; inst = inst->next)


struct inst *selected_inst = NULL;
struct bbox active_frame_bbox;

static struct inst *curr_frame = NULL;
static struct inst *insts[ip_n], **next_inst[ip_n];
static struct inst *prev_insts[ip_n];

static unsigned long active_set = 0;


#define	IS_ACTIVE	((active_set & 1))


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


int inst_select(const struct draw_ctx *ctx, struct coord pos)
{
	enum inst_prio prio;
	struct inst *inst;
	int best_dist = 0; /* keep gcc happy */
	int dist;

	deselect_outside();
	edit_nothing();
	selected_inst = NULL;
	FOR_INST_PRIOS_DOWN(prio) {
		for (inst = insts[prio]; inst; inst = inst->next) {
			if (!inst->active || !inst->ops->distance)
				continue;
			dist = inst->ops->distance(inst, pos, ctx->scale);
			if (dist >= 0 && (!selected_inst || best_dist > dist)) {
				selected_inst = inst;
				best_dist = dist;
			}
		}
		if (selected_inst)
			goto selected;
	}

	/* give vectors a second chance */

	for (inst = insts[ip_vec]; inst; inst = inst->next) {
		if (!inst->active)
			continue;
		dist = gui_dist_vec_fallback(inst, pos, ctx->scale);
		if (dist >= 0 && (!selected_inst || best_dist > dist)) {
			selected_inst = inst;
			best_dist = dist;
		}
	}
	
	if (!selected_inst)
		return 0;

selected:
	set_path(1);
	if (selected_inst->ops->select)
		selected_inst->ops->select(selected_inst);
	return 1;
}


struct inst *inst_find_point(const struct draw_ctx *ctx, struct coord pos)
{
	struct inst *inst, *found;
	int best_dist = 0; /* keep gcc happy */
	int dist;

	found = NULL;
	for (inst = insts[ip_frame]; inst; inst = inst->next) {
		if (!inst->active)
			continue;
		dist = gui_dist_frame_eye(inst, pos, ctx->scale);
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
		dist = inst->ops->distance(inst, pos, ctx->scale);
		if (dist >= 0 && (!found || best_dist > dist)) {
			found = inst;
			best_dist = dist;
		}
	}
	return found;
}


struct coord inst_get_point(const struct inst *inst)
{
	if (inst->ops == &vec_ops)
		return inst->u.rect.end;
	if (inst->ops == &frame_ops)
		return inst->base;
	abort();
}


struct vec *inst_get_ref(const struct inst *inst)
{
	if (inst->ops == &vec_ops) {
		inst->vec->n_refs++;
		return inst->vec;
	}
	if (inst->ops == &frame_ops)
		return NULL;
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
	if (selected_inst)
		set_path(0);
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


static void rect_status(struct coord a, struct coord b, unit_type width)
{
	struct coord d = sub_vec(b, a);
	double angle;
	
	status_set_xy(d);
	if (!d.x && !d.y)
		status_set_angle("a = 0 deg");
	else {
		angle = atan2(d.y, d.x)/M_PI*180.0;
		if (angle < 0)
			angle += 360;
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
	update_bbox(&curr_frame->bbox, inst->bbox.min);
	update_bbox(&curr_frame->bbox, inst->bbox.max);
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
	edit_unique_null(&self->vec->name, validate_vec_name, self->vec);
	edit_x(&self->vec->x);
	edit_y(&self->vec->y);
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
	.anchors	= vec_op_anchors,
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
	    units_to_mm(self->bbox.min.x), units_to_mm(self->bbox.min.y),
	    units_to_mm(self->bbox.max.x), units_to_mm(self->bbox.max.y));
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
	status_set_name("%s", self->u.name);
	rect_status(self->bbox.min, self->bbox.max, -1);
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
};


int inst_pad(struct obj *obj, const char *name, struct coord a, struct coord b)
{
	struct inst *inst;

	inst = add_inst(&pad_ops, ip_pad, a);
	inst->obj = obj;
	inst->u.name = stralloc(name);
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
};


int inst_arc(struct obj *obj, struct coord center, struct coord start,
    struct coord end, unit_type width)
{
	struct inst *inst;
	double r, a1, a2;

	inst = add_inst(&arc_ops, ip_arc, center);
	inst->obj = obj;
	r = hypot(start.x-center.x, start.y-center.y);
	a1 = atan2(start.y-center.y, start.x-center.x)/M_PI*180.0;
	a2 = atan2(end.y-center.y, end.x-center.x)/M_PI*180.0;
	if (a1 < 0)
		a1 += 360.0;
	if (a2 < 0)
		a2 += 360.0;
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
	status_set_type_entry("width =");
	status_set_name("%5.2f mm", units_to_mm(self->u.meas.offset));
	edit_expr(&self->obj->u.meas.offset);
}


static int meas_op_anchors(struct inst *inst, struct vec ***anchors)
{
	struct obj *obj = inst->obj;

	anchors[0] = &obj->base;
	anchors[1] = &obj->u.meas.other;
	return 2;
}


static struct inst_ops meas_ops = {
	.debug		= meas_op_debug,
	.draw		= gui_draw_meas,
	.distance	= gui_dist_meas,
	.select		= meas_op_select,
	.anchors	= meas_op_anchors,
};


int inst_meas(struct obj *obj, struct coord from, struct coord to,
    unit_type offset)
{
	struct inst *inst;

	inst = add_inst(&meas_ops, ip_meas, from);
	inst->obj = obj;
	inst->u.meas.end = to;
	inst->u.meas.offset = offset;
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
	anchors[0] = &inst->vec->base;
	return 1;
}


static struct inst_ops frame_ops = {
	.debug		= frame_op_debug,
	.draw		= gui_draw_frame,
	.hover		= gui_hover_frame,
	.distance	= gui_dist_frame,
	.select		= frame_op_select,
	.anchors	= frame_op_anchors,
};


void inst_begin_frame(const struct frame *frame, struct coord base,
    int active, int is_active_frame)
{
	struct inst *inst;

	inst = add_inst(&frame_ops, ip_frame, base);
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
	if (inst->active && frame == active_frame)
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


void inst_draw(struct draw_ctx *ctx)
{
	enum inst_prio prio;
	struct inst *inst;

	FOR_INSTS_UP(prio, inst)
		if (!inst->active && inst->ops->draw)
			inst->ops->draw(inst, ctx);
	FOR_INSTS_UP(prio, inst)
		if (prio != ip_frame && inst->active &&
		    inst != selected_inst && inst->ops->draw)
			inst->ops->draw(inst, ctx);
	for (inst = insts[ip_frame]; inst; inst = inst->next)
		if (inst->active && inst != selected_inst && inst->ops->draw)
			inst->ops->draw(inst, ctx);
	if (selected_inst && selected_inst->ops->draw)
		selected_inst->ops->draw(selected_inst, ctx);
}


void inst_hover(struct inst *inst, struct draw_ctx *ctx, int on)
{
	if (!inst->ops->hover)
		return;
	if (on)
		inst->ops->hover(inst, ctx);
	else
		inst->ops->draw(inst, ctx);
}


void inst_debug(void)
{
	enum inst_prio prio;
	struct inst *inst;

	FOR_INSTS_UP(prio, inst)
		inst->ops->debug(inst);
}
