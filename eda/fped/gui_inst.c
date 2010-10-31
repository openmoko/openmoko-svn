/*
 * gui_inst.c - GUI, instance functions
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
#include <math.h>
#include <gtk/gtk.h>

#include "util.h"
#include "coord.h"
#include "inst.h"
#include "gui.h"
#include "gui_util.h"
#include "gui_style.h"
#include "gui_status.h"
#include "gui_inst.h"


/* ----- coordinate translation -------------------------------------------- */


struct coord translate(struct coord pos)
{
	pos.x -= draw_ctx.center.x;
	pos.y -= draw_ctx.center.y;
	pos.x /= draw_ctx.scale;
	pos.y /= draw_ctx.scale;
	pos.y = -pos.y;
	pos.x += draw_ctx.widget->allocation.width/2;
	pos.y += draw_ctx.widget->allocation.height/2;
	return pos;
}


struct coord canvas_to_coord(int x, int y)
{
	struct coord pos;

	x -= draw_ctx.widget->allocation.width/2;
	y -= draw_ctx.widget->allocation.height/2;
	y = -y;
	pos.x = x*draw_ctx.scale+draw_ctx.center.x;
	pos.y = y*draw_ctx.scale+draw_ctx.center.y;
	return pos;
}


/* ----- drawing primitives ------------------------------------------------ */


static void draw_eye(GdkGC *gc, struct coord center, int r1, int r2)
{
	draw_circle(DA, gc, TRUE, center.x, center.y, r1);
	draw_circle(DA, gc, FALSE, center.x, center.y, r2);
}


#define MAX_POINTS	10


static void draw_poly(GdkGC *gc, int fill,
    const struct coord *points, int n_points)
{
	GdkPoint gp[MAX_POINTS];
	int i;

	if (n_points > MAX_POINTS)
		abort();
	for (i = 0; i != n_points; i++) {
		gp[i].x = points[i].x;
		gp[i].y = points[i].y;
	}
	if (fill)
		gdk_draw_polygon(DA, gc, fill, gp, n_points);
	else {
		gdk_draw_line(DA, gc, gp[0].x, gp[0].y, gp[1].x, gp[1].y);
		gdk_draw_line(DA, gc, gp[1].x, gp[1].y, gp[2].x, gp[2].y);
	}
}


static void draw_arrow(GdkGC *gc, int fill,
    struct coord from, struct coord to, int len, double angle)
{
	struct coord p[3];
	struct coord side;

	if (from.x == to.x && from.y == to.y) {
		side.x = 0;
		side.y = -len;
	} else {
		side = normalize(sub_vec(to, from), len);
	}
	p[0] = add_vec(to, rotate(side, 180-angle));
	p[1] = to;
	p[2] = add_vec(to, rotate(side, 180+angle));
	draw_poly(gc, fill, p, 3);
}


static enum mode get_mode(const struct inst *self)
{
	if (selected_inst == self)
		return mode_selected;
	return self->active || bright(self) ? mode_active : mode_inactive;
}


/* ----- vec --------------------------------------------------------------- */


unit_type gui_dist_vec(struct inst *self, struct coord pos, unit_type scale)
{
	unit_type d;

	d = dist_point(pos, self->u.vec.end)/scale;
	return d > VEC_EYE_R ? -1 : d;
}


/*
 * The circles at a vector's tip enjoy the highest selection priority. However,
 * users will probably also expected a click on a nicely exposed stem to work.
 * So we give it second look after having exhausted all other options.
 */

unit_type gui_dist_vec_fallback(struct inst *self, struct coord pos,
    unit_type scale)
{
	unit_type d;

	d = dist_line(pos, self->base, self->u.vec.end)/scale;
	return d > SELECT_R ? -1 : d;
}


void gui_highlight_vec(struct inst *self)
{
	struct coord center = translate(self->u.vec.end);

	draw_circle(DA, gc_highlight, FALSE, center.x, center.y, VEC_EYE_R);
}


void gui_draw_vec(struct inst *self)
{
	struct coord from = translate(self->base);
	struct coord to = translate(self->u.vec.end);
	GdkGC *gc;

	gc = gc_vec[get_mode(self)];
	draw_arrow(gc, TRUE, from, to, VEC_ARROW_LEN, VEC_ARROW_ANGLE);
	gdk_draw_line(DA, gc, from.x, from.y, to.x, to.y);
	draw_circle(DA, gc, FALSE, to.x, to.y, VEC_EYE_R);
}


/* ----- line -------------------------------------------------------------- */


unit_type gui_dist_line(struct inst *self, struct coord pos, unit_type scale)
{
	unit_type r, d;

	r = self->u.rect.width/scale/2;
	if (r < SELECT_R)
		r = SELECT_R;
	d = dist_line(pos, self->base, self->u.rect.end)/scale;
	return d > r ? -1 : d;
}


void gui_draw_line(struct inst *self)
{
	struct coord min = translate(self->base);
	struct coord max = translate(self->u.rect.end);
	GdkGC *gc;

	gc = gc_obj[get_mode(self)];
	set_width(gc, self->u.rect.width/draw_ctx.scale);
	gdk_draw_line(DA, gc, min.x, min.y, max.x, max.y);
}


/* ----- rect -------------------------------------------------------------- */


unit_type gui_dist_rect(struct inst *self, struct coord pos, unit_type scale)
{
	unit_type r, d;

	r = self->u.rect.width/scale/2;
	if (r < SELECT_R)
		r = SELECT_R;
	d = dist_rect(pos, self->base, self->u.rect.end)/scale;
	return d > r ? -1 : d;
}


void gui_draw_rect(struct inst *self)
{
	struct coord min = translate(self->base);
	struct coord max = translate(self->u.rect.end);
	GdkGC *gc;

	sort_coord(&min, &max);
	gc = gc_obj[get_mode(self)];
	set_width(gc, self->u.rect.width/draw_ctx.scale);
	gdk_draw_rectangle(DA, gc, FALSE,
	    min.x, min.y, max.x-min.x, max.y-min.y);
}


/* ----- pad --------------------------------------------------------------- */


unit_type gui_dist_pad(struct inst *self, struct coord pos, unit_type scale)
{
	unit_type d;

	if (inside_rect(pos, self->base, self->u.pad.other))
		return SELECT_R;
	d = dist_rect(pos, self->base, self->u.pad.other)/scale;
	return d > SELECT_R ? -1 : d;
}


static void pad_text_in_rect(struct inst *self,
    struct coord min, struct coord max)
{
	GdkGC *gc;
	struct coord c;
	unit_type h, w;
	int rot;

	w = max.x-min.x;
	h = max.y-min.y;
	rot = w/1.1 < h;
	gc = gc_ptext[get_mode(self)];
	c = add_vec(min, max);
	h = max.y-min.y;
	w = max.x-min.x;
	render_text(DA, gc, c.x/2, c.y/2, rot ? 0 : 90,
	    self->u.pad.name, PAD_FONT, 0.5, 0.5,
	    w-2*PAD_BORDER, h-2*PAD_BORDER);
}


static void maximize_box(struct coord *min_box, struct coord *max_box,
    unit_type x_min, unit_type y_min, unit_type x_max, unit_type y_max)
{
	unit_type d_box, d_new, d;

	d_box = max_box->x-min_box->x;
	d = max_box->y-min_box->y;
	if (d < d_box)
		d_box = d;

	d_new = x_max-x_min;
	d = y_max-y_min;
	if (d < d_new)
		d_new = d;

	if (d_new < d_box)
		return;

	min_box->x = x_min;
	min_box->y = y_min;
	max_box->x = x_max;
	max_box->y = y_max;
}


static void gui_draw_pad_text(struct inst *self)
{
	struct coord pad_min = translate(self->base);
	struct coord pad_max = translate(self->u.pad.other);
	struct coord hole_min, hole_max;
	struct coord box_min, box_max;

	sort_coord(&pad_min, &pad_max);
	if (!self->u.pad.hole) {
		pad_text_in_rect(self, pad_min, pad_max);
		return;
	}

	hole_min = translate(self->u.pad.hole->base);
	hole_max = translate(self->u.pad.hole->u.hole.other);
	sort_coord(&hole_min, &hole_max);

	box_min.x = pad_min.x;					/* top */
	box_min.y = pad_min.y;
	box_max.x = pad_max.x;
	box_max.y = hole_min.y;

	maximize_box(&box_min, &box_max,
	    pad_min.x, hole_max.y, pad_max.x, pad_max.y);	/* bottom */
	maximize_box(&box_min, &box_max,
	    pad_min.x, pad_min.y, hole_min.x, pad_max.y);	/* left */
	maximize_box(&box_min, &box_max,
	    hole_max.x, pad_min.y, pad_max.x, pad_max.y);	/* right */

	pad_text_in_rect(self, box_min, box_max);
}


static GdkGC *pad_gc(const struct inst *inst, int *fill)
{
	*fill = TRUE;
	switch (layers_to_pad_type(inst->u.pad.layers)) {
	case pt_bare:
		return gc_pad_bare[get_mode(inst)];
	case pt_mask:
		*fill = FALSE;
		return gc_pad_mask[get_mode(inst)];
	default:
		return gc_pad[get_mode(inst)];
	}
}


void gui_draw_pad(struct inst *self)
{
	struct coord min = translate(self->base);
	struct coord max = translate(self->u.pad.other);
	GdkGC *gc;
	int fill;

	gc = pad_gc(self, &fill);
	sort_coord(&min, &max);
	gdk_draw_rectangle(DA, gc, fill,
	    min.x, min.y, max.x-min.x, max.y-min.y);

	gui_draw_pad_text(self);
}


static void draw_rounded_rect(GdkGC *gc, struct coord a, struct coord b,
     int fill)
{
	struct coord min = translate(a);
	struct coord max = translate(b);
	unit_type h, w, r;

	sort_coord(&min, &max);
	h = max.y-min.y;
	w = max.x-min.x;
	if (h > w) {
		r = w/2;
		draw_arc(DA, gc, fill, min.x+r, max.y-r, r, 180, 0);
		if (fill)
			gdk_draw_rectangle(DA, gc, fill,
			    min.x, min.y+r, w, h-2*r);
		else {
			gdk_draw_line(DA, gc, min.x, min.y+r, min.x, max.y-r);
			gdk_draw_line(DA, gc, max.x, min.y+r, max.x, max.y-r);
		}
		draw_arc(DA, gc, fill, min.x+r, min.y+r, r, 0, 180);
	} else {
		r = h/2;
		draw_arc(DA, gc, fill, min.x+r, min.y+r, r, 90, 270);
		if (fill)
			gdk_draw_rectangle(DA, gc, fill,
			    min.x+r, min.y, w-2*r, h);
		else {
			gdk_draw_line(DA, gc, min.x+r, min.y, max.x-r, min.y);
			gdk_draw_line(DA, gc, min.x+r, max.y, max.x-r, max.y);
		}
		draw_arc(DA, gc, fill, max.x-r, min.y+r, r, 270, 90);
	}
}


void gui_draw_rpad(struct inst *self)
{
	GdkGC *gc;
	int fill;

	gc = pad_gc(self, &fill);
	draw_rounded_rect(gc, self->base, self->u.pad.other, fill);
	gui_draw_pad_text(self);
}


/* ----- hole -------------------------------------------------------------- */


unit_type gui_dist_hole(struct inst *self, struct coord pos, unit_type scale)
{
	unit_type d;

	/* @@@ not quite right ... */
	if (inside_rect(pos, self->base, self->u.hole.other))
		return SELECT_R;
	d = dist_rect(pos, self->base, self->u.hole.other)/scale;
	return d > SELECT_R ? -1 : d;
}


void gui_draw_hole(struct inst *self)
{
	draw_rounded_rect(gc_hole[get_mode(self)],
	    self->base, self->u.hole.other, 1);
	draw_rounded_rect(gc_rim[get_mode(self)],
	    self->base, self->u.hole.other, 0);
}


/* ----- arc --------------------------------------------------------------- */


unit_type gui_dist_arc(struct inst *self, struct coord pos, unit_type scale)
{
	struct coord c = self->base;
	struct coord p;
	unit_type r, d_min, d;
	double angle, a1, a2;

	r = self->u.arc.width/scale/2;
	if (r < SELECT_R)
		r = SELECT_R;

	/* check endpoints */

	p = rotate_r(c, self->u.arc.r, self->u.arc.a1);
	d_min = hypot(pos.x-p.x, pos.y-p.y);

	p = rotate_r(c, self->u.arc.r, self->u.arc.a2);
	d = hypot(pos.x-p.x, pos.y-p.y);
	if (d < d_min)
		d_min = d;

	if (d_min/scale <= r)
		return d;

	/* distance from the circle containing the arc */

	d = dist_circle(pos, c, self->u.arc.r)/scale;
	if (d > r)
		return -1;
	if (self->u.arc.a1 == self->u.arc.a2)
		return d;

	/* see if we're close to the part that's actually drawn */

	angle = theta(c, pos);
	a1 = self->u.arc.a1;
	a2 = self->u.arc.a2;
	if (angle < 0)
		angle += 360;
	if (a2 < a1)
		a2 += 360;
	if (angle < a1)
		angle += 360;
	return angle >= a1 && angle <= a2 ? d : -1;
}


void gui_draw_arc(struct inst *self)
{
	struct coord center = translate(self->base);
	GdkGC *gc;

	gc = gc_obj[get_mode(self)];
	set_width(gc, self->u.arc.width/draw_ctx.scale);
	draw_arc(DA, gc, FALSE, center.x, center.y,
	    self->u.arc.r/draw_ctx.scale, self->u.arc.a1, self->u.arc.a2);
}


/* ----- meas -------------------------------------------------------------- */


static struct coord offset_vec(struct coord a, struct coord b,
    const struct inst *self)
{
	struct coord res;
	double f;

	res.x = a.y-b.y;
	res.y = b.x-a.x;
	if (res.x == 0 && res.y == 0)
		return res;
	f = self->u.meas.offset/hypot(res.x, res.y);
	res.x *= f;
	res.y *= f;
	return res;
}


void project_meas(const struct inst *inst, struct coord *a1, struct coord *b1)
{
	const struct meas *meas = &inst->obj->u.meas;
	struct coord off;

	*a1 = inst->base;
	*b1 = inst->u.meas.end;
	switch (meas->type) {
	case mt_xy_next:
	case mt_xy_max:
		break;
	case mt_x_next:
	case mt_x_max:
		b1->y = a1->y;
		break;
	case mt_y_next:
	case mt_y_max:
		b1->x = a1->x;
		break;
	default:
		abort();
	}
	off = offset_vec(*a1, *b1, inst);
	*a1 = add_vec(*a1, off);
	*b1 = add_vec(*b1, off);
}


unit_type gui_dist_meas(struct inst *self, struct coord pos, unit_type scale)
{
	struct coord a1, b1;
	unit_type d;

	project_meas(self, &a1, &b1);
	d = dist_line(pos, a1, b1)/scale;
	return d > SELECT_R ? -1 : d;
}


char *format_len(const char *label, unit_type len, enum curr_unit unit)
{
	const char *u = "";
	double n;
	int mm;

	switch (unit) {
	case curr_unit_mm:
		n = units_to_mm(len);
		mm = 1;
		break;
	case curr_unit_mil:
		n = units_to_mil(len);
		mm = 0;
		break;
	case curr_unit_auto:
		n = units_to_best(len, &mm);
		u = mm ? "mm" : "mil";
		break;
	default:
		abort();
	}
	return stralloc_printf(mm ?
	    "%s" MM_FORMAT_SHORT "%s" : 
	    "%s" MIL_FORMAT_SHORT "%s",
	    label, n, u);
}


void gui_draw_meas(struct inst *self)
{
	const struct meas *meas = &self->obj->u.meas;
	struct coord a0, b0, a1, b1, c, d;
	GdkGC *gc;
	double len;
	char *s;

	a0 = translate(self->base);
	b0 = translate(self->u.meas.end);
	project_meas(self, &a1, &b1);

	len = dist_point(a1, b1);
	a1 = translate(a1);
	b1 = translate(b1);
	gc = gc_meas[get_mode(self)];
	gdk_draw_line(DA, gc, a0.x, a0.y, a1.x, a1.y);
	gdk_draw_line(DA, gc, b0.x, b0.y, b1.x, b1.y);
	gdk_draw_line(DA, gc, a1.x, a1.y, b1.x, b1.y);
	draw_arrow(gc, FALSE, a1, b1, MEAS_ARROW_LEN, MEAS_ARROW_ANGLE);
	draw_arrow(gc, FALSE, b1, a1, MEAS_ARROW_LEN, MEAS_ARROW_ANGLE);

	c = add_vec(a1, b1);
	d = sub_vec(b1, a1);
	s = format_len(meas->label ? meas->label : "", len, curr_unit);
	render_text(DA, gc, c.x/2, c.y/2, -atan2(d.y, d.x)/M_PI*180, s,
	    MEAS_FONT, 0.5, -MEAS_BASELINE_OFFSET,
	    dist_point(a1, b1)-1.5*MEAS_ARROW_LEN, 0);
	free(s);
}


/* ----- frame ------------------------------------------------------------- */


unit_type gui_dist_frame_eye(struct inst *self, struct coord pos,
    unit_type scale)
{
	unit_type d;

	d = dist_point(pos, self->base)/scale;
	return d > FRAME_EYE_R2 ? -1 : d;
}


static unit_type dist_from_corner_line(struct inst *self, struct coord pos,
    struct coord vec, unit_type scale)
{
	struct coord ref;

	ref.x = self->bbox.min.x;
	ref.y = self->bbox.max.y;
	return dist_line(pos, ref, add_vec(ref, vec))/scale;
}


unit_type gui_dist_frame(struct inst *self, struct coord pos, unit_type scale)
{
	unit_type d_min, d;
	struct coord vec;

	d_min = dist_point(pos, self->base)/scale;

	vec.x = FRAME_SHORT_X*scale;
	vec.y = 0;
	d = dist_from_corner_line(self, pos, vec, scale);
	if (d < d_min)
		d_min = d;

	vec.x = 0;
	vec.y = FRAME_SHORT_Y*scale;
	d = dist_from_corner_line(self, pos, vec, scale);
	if (d < d_min)
		d_min = d;

	return d_min > SELECT_R ? -1 : d_min;
}


void gui_draw_frame(struct inst *self)
{
	struct coord center = translate(self->base);
	struct coord corner = { self->bbox.min.x, self->bbox.max.y };
	GdkGC *gc;

	gc = self->u.frame.active ? gc_active_frame : gc_frame[get_mode(self)];
	draw_eye(gc, center, FRAME_EYE_R1, FRAME_EYE_R2);
	if (self->u.frame.ref == frames)
		return;
	corner = translate(corner);
	corner.x -= FRAME_CLEARANCE;
	corner.y -= FRAME_CLEARANCE;
	gdk_draw_line(DA, gc, corner.x, corner.y,
	    corner.x+FRAME_SHORT_X, corner.y);
	gdk_draw_line(DA, gc, corner.x, corner.y,
	    corner.x, corner.y+FRAME_SHORT_Y);
	render_text(DA, gc, corner.x, corner.y, 0, self->u.frame.ref->name,
	    FRAME_FONT, 0, -FRAME_BASELINE_OFFSET, 0, 0);
}
