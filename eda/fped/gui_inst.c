/*
 * gui_inst.c - GUI, instance functions
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
#include <math.h>
#include <gtk/gtk.h>

#include "util.h"
#include "inst.h"
#include "gui.h"
#include "gui_util.h"
#include "gui_style.h"
#include "gui_inst.h"


#define DA	GDK_DRAWABLE(ctx->widget->window)


/* ----- coordinate translation -------------------------------------------- */


struct coord translate(const struct draw_ctx *ctx, struct coord pos)
{
	pos.x -= ctx->center.x;
	pos.y -= ctx->center.y;
	pos.x /= ctx->scale;
	pos.y /= ctx->scale;
	pos.y = -pos.y;
	pos.x += ctx->widget->allocation.width/2;
	pos.y += ctx->widget->allocation.height/2;
//fprintf(stderr, "%d %d\n", (int) pos.x, (int) pos.y);
	return pos;
}


struct coord canvas_to_coord(const struct draw_ctx *ctx, int x, int y)
{
	struct coord pos;

	x -= ctx->widget->allocation.width/2;
	y -= ctx->widget->allocation.height/2;
        y = -y;
        pos.x = x*ctx->scale+ctx->center.x;
        pos.y = y*ctx->scale+ctx->center.y;
	return pos;
}


/* ----- drawing primitives ------------------------------------------------ */


static void draw_arc(struct draw_ctx *ctx, GdkGC *gc, int fill,
    int x, int y, int r, double a1, double a2)
{
	if (a1 == a2)
		a2 = a1+360;
	gdk_draw_arc(DA, gc, fill, x-r, y-r, 2*r, 2*r, a1*64, (a2-a1)*64);
}


static void draw_circle(struct draw_ctx *ctx, GdkGC *gc, int fill,
    int x, int y, int r)
{
	draw_arc(ctx, gc, fill, x, y, r, 0, 360);
}


static void draw_eye(struct draw_ctx *ctx, GdkGC *gc, struct coord center,
    int r1, int r2)
{
	draw_circle(ctx, gc, TRUE, center.x, center.y, r1);
	draw_circle(ctx, gc, FALSE, center.x, center.y, r2);
}


#define MAX_POINTS	10


static void draw_poly(struct draw_ctx *ctx, GdkGC *gc, int fill,
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


static void draw_arrow(struct draw_ctx *ctx, GdkGC *gc, int fill,
    struct coord from, struct coord to, int len, double angle)
{
	struct coord p[3];
	struct coord side;

	side = normalize(sub_vec(to, from), len);
	p[0] = add_vec(to, rotate(side, 180-angle));
	p[1] = to;
	p[2] = add_vec(to, rotate(side, 180+angle));
	draw_poly(ctx, gc, fill, p, 3);
}


static enum mode get_mode(struct inst *self)
{
	if (selected_inst == self)
		return mode_selected;
	if (self->active)
		return self->in_path ? mode_active_in_path : mode_active;
	return self->in_path ? mode_inactive_in_path : mode_inactive;
}


/* ----- vec --------------------------------------------------------------- */


unit_type gui_dist_vec(struct inst *self, struct coord pos, unit_type scale)
{
	unit_type d;

	d = dist_point(pos, self->u.rect.end)/scale;
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

	d = dist_line(pos, self->base, self->u.rect.end)/scale;
	return d > SELECT_R ? -1 : d;
}


void gui_draw_vec(struct inst *self, struct draw_ctx *ctx)
{
	struct coord from = translate(ctx, self->base);
	struct coord to = translate(ctx, self->u.rect.end);
	GdkGC *gc;

	gc = gc_vec[get_mode(self)];
	draw_arrow(ctx, gc, TRUE, from, to, VEC_ARROW_LEN, VEC_ARROW_ANGLE);
	gdk_draw_line(DA, gc, from.x, from.y, to.x, to.y);
	draw_circle(ctx, gc, FALSE, to.x, to.y, VEC_EYE_R);
}


/* ----- line -------------------------------------------------------------- */


unit_type gui_dist_line(struct inst *self, struct coord pos, unit_type scale)
{
	unit_type r, d;

	r = self->u.rect.width/scale/2;
	if (r < SELECT_R)
		r = SELECT_R;
	d = dist_line(pos, self->bbox.min, self->bbox.max)/scale;
	return d > r ? -1 : d;
}


void gui_draw_line(struct inst *self, struct draw_ctx *ctx)
{
	struct coord min = translate(ctx, self->base);
	struct coord max = translate(ctx, self->u.rect.end);
	GdkGC *gc;

	gc = gc_obj[get_mode(self)];
	set_width(gc, self->u.rect.width/ctx->scale);
	gdk_draw_line(DA, gc, min.x, min.y, max.x, max.y);
}


/* ----- rect -------------------------------------------------------------- */


unit_type gui_dist_rect(struct inst *self, struct coord pos, unit_type scale)
{
	unit_type r, d;

	r = self->u.rect.width/scale/2;
	if (r < SELECT_R)
		r = SELECT_R;
	d = dist_rect(pos, self->bbox.min, self->bbox.max)/scale;
	return d > r ? -1 : d;
}


void gui_draw_rect(struct inst *self, struct draw_ctx *ctx)
{
	struct coord min = translate(ctx, self->bbox.min);
	struct coord max = translate(ctx, self->bbox.max);
	GdkGC *gc;

	gc = gc_obj[get_mode(self)];
	set_width(gc, self->u.rect.width/ctx->scale);
	gdk_draw_rectangle(DA, gc, FALSE,
	    min.x, max.y, max.x-min.x, min.y-max.y);
}


/* ----- pad --------------------------------------------------------------- */


unit_type gui_dist_pad(struct inst *self, struct coord pos, unit_type scale)
{
	unit_type d;

	if (inside_rect(pos, self->bbox.min, self->bbox.max))
		return SELECT_R;
	d = dist_rect(pos, self->bbox.min, self->bbox.max)/scale;
	return d > SELECT_R ? -1 : d;
}


void gui_draw_pad(struct inst *self, struct draw_ctx *ctx)
{
	struct coord min = translate(ctx, self->bbox.min);
	struct coord max = translate(ctx, self->bbox.max);
	GdkGC *gc;
	struct coord c;
	unit_type h, w;

	gc = gc_pad[get_mode(self)];
	gdk_draw_rectangle(DA, gc, TRUE,
	    min.x, max.y, max.x-min.x, min.y-max.y);

	gc = gc_ptext[get_mode(self)];
	c = add_vec(min, max);
	h = min.y-max.y;
	w = max.x-min.x;
	render_text(DA, gc, c.x/2, c.y/2, w <= h*1.1 ? 0 : 90, self->u.name,
	    PAD_FONT, 0.5, 0.5,
	    w-2*PAD_BORDER, h-2*PAD_BORDER);
}


/* ----- arc --------------------------------------------------------------- */


static struct coord rotate_r(struct coord center, unit_type r, double angle)
{
	struct coord res;

	angle = angle/180.0*M_PI;
	res.x = center.x+r*cos(angle);
	res.y = center.y+r*sin(angle);
	return res;
}


unit_type gui_dist_arc(struct inst *self, struct coord pos, unit_type scale)
{
	struct coord c = self->base;
	struct coord p;
	unit_type r, d_min, d;
	double angle, a2;

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

	angle = atan2(pos.y-c.y, pos.x-c.x)/M_PI*180.0;
	if (angle < 0)
		angle += 180;
	a2 = self->u.arc.a2;
	if (a2 < self->u.arc.a1)
		a2 += 180;
	return angle >= self->u.arc.a1 && angle <= a2 ? d : -1;
}


void gui_draw_arc(struct inst *self, struct draw_ctx *ctx)
{
	struct coord center = translate(ctx, self->base);
	GdkGC *gc;

	gc = gc_obj[get_mode(self)];
	set_width(gc, self->u.arc.width/ctx->scale);
	draw_arc(ctx, gc, FALSE, center.x, center.y,
	    self->u.arc.r/ctx->scale, self->u.arc.a1, self->u.arc.a2);
}


/* ----- meas -------------------------------------------------------------- */


static struct coord offset_vec(const struct inst *self)
{
	struct coord a, b, res;
	double f;

	a = self->base;
	b = self->u.meas.end;
	res.x = a.y-b.y;
	res.y = b.x-a.x;
	if (res.x == 0 && res.y == 0)
		return res;
	f = self->u.meas.offset/hypot(res.x, res.y);
	res.x *= f;
	res.y *= f;
	return res;
}


unit_type gui_dist_meas(struct inst *self, struct coord pos, unit_type scale)
{
	struct coord a, b, off;
	unit_type d;

	off = offset_vec(self);
	a = add_vec(self->base, off);
	b = add_vec(self->u.meas.end, off);
	d = dist_line(pos, a, b)/scale;
	return d > SELECT_R ? -1 : d;
}


void gui_draw_meas(struct inst *self, struct draw_ctx *ctx)
{
	struct coord a0, b0, a1, b1, off, c, d;
	GdkGC *gc;
	char *s;

	off = offset_vec(self);
	a0 = translate(ctx, self->base);
	b0 = translate(ctx, self->u.meas.end);
	a1 = translate(ctx, add_vec(self->base, off));
	b1 = translate(ctx, add_vec(self->u.meas.end, off));
	gc = gc_meas[get_mode(self)];
	gdk_draw_line(DA, gc, a0.x, a0.y, a1.x, a1.y);
	gdk_draw_line(DA, gc, b0.x, b0.y, b1.x, b1.y);
	gdk_draw_line(DA, gc, a1.x, a1.y, b1.x, b1.y);
	draw_arrow(ctx, gc, FALSE, a1, b1, MEAS_ARROW_LEN, MEAS_ARROW_ANGLE);
	draw_arrow(ctx, gc, FALSE, b1, a1, MEAS_ARROW_LEN, MEAS_ARROW_ANGLE);

	c = add_vec(a1, b1);
	d = sub_vec(b0, a0);
	s = stralloc_printf("%lgmm",
	    units_to_mm(dist_point(self->base, self->u.meas.end)));
	render_text(DA, gc, c.x/2, c.y/2, -atan2(d.y, d.x)/M_PI*180, s,
	    MEAS_FONT, 0.5, -MEAS_BASELINE_OFFSET,
	    dist_point(a1, b1)-1.5*MEAS_ARROW_LEN, 0);
	free(s);
}


/* ----- frame ------------------------------------------------------------- */


void gui_draw_frame(struct inst *self, struct draw_ctx *ctx)
{
	struct coord center = translate(ctx, self->base);
	struct coord corner = { self->bbox.min.x, self->bbox.max.y };
	GdkGC *gc;

	gc = gc_frame[get_mode(self)];
	draw_eye(ctx, gc, center, FRAME_EYE_R1, FRAME_EYE_R2);
	if (!self->u.frame.ref->name)
		return;
	corner = translate(ctx, corner);
	corner.x -= FRAME_CLEARANCE;
	corner.y -= FRAME_CLEARANCE;
	gdk_draw_line(DA, gc, corner.x, corner.y,
	    corner.x+FRAME_SHORT_X, corner.y);
	gdk_draw_line(DA, gc, corner.x, corner.y,
	    corner.x, corner.y+FRAME_SHORT_Y);
	render_text(DA, gc, corner.x, corner.y, 0, self->u.frame.ref->name,
	    FRAME_FONT, 0, -FRAME_BASELINE_OFFSET, 0, 0);
}
