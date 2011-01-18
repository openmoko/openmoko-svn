/*
 * postscript.c - Dump objects in Postscript
 *
 * Written 2009-2011 by Werner Almesberger
 * Copyright 2009-2011 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "coord.h"
#include "layer.h"
#include "obj.h"
#include "inst.h"
#include "unparse.h"
#include "gui_status.h"
#include "gui_inst.h"
#include "postscript.h"


/*
 * A4 is 210 mm x 297 mm
 * US Letter is 216 mm x 279 mm
 *
 * We pick the smallest dimensions minus a bit of slack and center on the
 * printer page.
 */

#define	PAGE_HALF_WIDTH		mm_to_units(210/2.0-10)	/* A4 */
#define	PAGE_HALF_HEIGHT	mm_to_units(279/2.0-15)	/* US Letter */

/*
 * Page layout:
 *
 * HEADER                 DATE
 * --------------------------- HEADER_HEIGHT+DIVIDER_BORDER below top
 *                    |
 *                    |  2x
 *         10 x       |<------------- roughly at 10/12
 *                    |  1x
 *                    |
 * --------------------------- 50% height
 * Frames in boxes
 *
 */


#define	PS_HEADER_HEIGHT	mm_to_units(8)
#define	PS_DIVIDER_BORDER	mm_to_units(2)
#define	PS_DIVIDER_WIDTH	mm_to_units(0.5)

#define	PS_MISC_TEXT_HEIGHT	mm_to_units(3)

#define	PS_DOT_DIST		mm_to_units(0.03)
#define	PS_DOT_DIAM		mm_to_units(0.01)

#define	PS_HATCH		mm_to_units(0.1)
#define	PS_HATCH_LINE		mm_to_units(0.015)

#define	PS_STRIPE		mm_to_units(0.08)

#define	PS_RIM_LINE		mm_to_units(0.02)

#define	PS_FONT_OUTLINE		mm_to_units(0.025)

#define	PS_VEC_LINE		mm_to_units(0.02)
#define	PS_VEC_ARROW_LEN	mm_to_units(0.3)
#define	PS_VEC_ARROW_ANGLE	20
#define	PS_VEC_TEXT_HEIGHT	mm_to_units(3)		/* ~8.5 pt, real mm */
#define	PS_VEC_BASE_OFFSET	mm_to_units(0.5)	/* real mm */

#define	PS_MEAS_LINE		mm_to_units(0.1)	/* real mm */
#define	PS_MEAS_ARROW_LEN	mm_to_units(0.15)
#define	PS_MEAS_ARROW_ANGLE	30
#define	PS_MEAS_TEXT_HEIGHT	mm_to_units(3)		/* ~8.5 pt, real mm */
#define	PS_MEAS_BASE_OFFSET	mm_to_units(0.5)	/* real mm */
#define	PS_MEAS_MIN_HEIGHT	(PS_MEAS_TEXT_HEIGHT/2)

#define	PS_CROSS_WIDTH		mm_to_units(0.01)
#define	PS_CROSS_DASH		mm_to_units(0.1)

#define TEXT_HEIGHT_FACTOR	1.5	/* height/width of typical text */


struct postscript_params postscript_params = {
	.zoom		= 0,
	.show_pad_names	= 1,
	.show_stuff	= 0,
	.label_vecs	= 0,
	.show_meas	= 1,
};

static const struct postscript_params minimal_params;
static struct postscript_params active_params;


/* ----- Boxes ------------------------------------------------------------- */


static struct box {
	unit_type x, y;		/* width and height */
	unit_type x0, y0;	/* lower left corner */
	struct box *next;
} *boxes = NULL;


static void add_box(unit_type xa, unit_type ya, unit_type xb, unit_type yb)
{
	struct box *box;

	box = alloc_type(struct box);
	box->x = xb-xa;
	box->y = yb-ya;
	box->x0 = xa;
	box->y0 = ya;
	box->next = boxes;
	boxes = box;
}


static void free_boxes(void)
{
	struct box *next;

	while (boxes) {
		next = boxes->next;
		free(boxes);
		boxes = next;
	}
}


static int get_box(unit_type x, unit_type y, unit_type *xa, unit_type *ya)
{
	struct box **box, **best = NULL;
	struct box *b;
	double size, best_size;

	for (box = &boxes; *box; box = &(*box)->next) {
		if ((*box)->x < x || (*box)->y < y)
			continue;
		size = (double) (*box)->x*(*box)->y;
		if (!best || size < best_size) {
			best = box;
			best_size = size;
		}
	}
	if (!best)
		return 0;
	b = *best;
	if (xa)
		*xa = b->x0;
	if (ya)
		*ya = b->y0+b->y-y;

	*best = b->next;
	add_box(b->x0+x, b->y0, b->x0+b->x, b->y0+b->y);
	add_box(b->x0, b->y0, b->x0+x, b->y0+b->y-y);
	free(b);

	return 1;
}


/* ----- Helper functions -------------------------------------------------- */


static void ps_string(FILE *file, const char *s)
{
	fputc('(', file);
	while (*s) {
		if (*s == '(' || *s == ')' || *s == '\\')
			fputc('\\', file);
		fputc(*s, file);
		s++;
	}
	fputc(')', file);
}


/* ----- Items ------------------------------------------------------------- */


static void ps_pad_name(FILE *file, const struct inst *inst)
{
	struct coord a = inst->base;
	struct coord b = inst->u.pad.other;
	unit_type h, w;

	if (!*inst->u.pad.name)
		return;
	h = a.y-b.y;
	w = a.x-b.x;
	if (h < 0)
		h = -h;
	if (w < 0)
		w = -w;
	fprintf(file, "0 setgray /Helvetica-Bold findfont dup\n");
	fprintf(file, "   ");
	ps_string(file, inst->u.pad.name);
	fprintf(file, " %d %d\n", w/2, h/2);
	fprintf(file, "   boxfont\n");
	fprintf(file, "   %d %d moveto\n", (a.x+b.x)/2, (a.y+b.y)/2);
	fprintf(file, "   ");
	ps_string(file, inst->u.pad.name);
	fprintf(file, " center %d showoutlined newpath\n", PS_FONT_OUTLINE);
}


static const char *hatch(layer_type layers)
{
	switch (layers_to_pad_type(layers)) {
	case pt_normal:
		return "crosspath";
	case pt_bare:
		return "hatchpath";
	case pt_paste:
		return "backhatchpath";
	case pt_mask:
		return "dotpath";
	case pt_trace:
		return "horpath";
	default:
		abort();
	}
}


static void ps_pad(FILE *file, const struct inst *inst, int show_name)
{
	struct coord a = inst->base;
	struct coord b = inst->u.pad.other;

	fprintf(file, "0 setgray %d setlinewidth\n", PS_HATCH_LINE);
	fprintf(file, "  %d %d moveto\n", a.x, a.y);
	fprintf(file, "  %d %d lineto\n", b.x, a.y);
	fprintf(file, "  %d %d lineto\n", b.x, b.y);
	fprintf(file, "  %d %d lineto\n", a.x, b.y);
	fprintf(file, "  closepath gsave %s grestore stroke\n",
	    hatch(inst->u.pad.layers));

	if (show_name && !inst->u.pad.hole)
		ps_pad_name(file, inst);
}


static void ps_rounded_rect(FILE *file, struct coord a, struct coord b)
{
	unit_type h, w, r;

	sort_coord(&a, &b);
	h = b.y-a.y;
	w = b.x-a.x;

	if (h > w) {
		r = w/2;
		fprintf(file, "  %d %d moveto\n", b.x, b.y-r);
		fprintf(file, "  %d %d %d 0 180 arc\n", a.x+r, b.y-r, r);
		fprintf(file, "  %d %d lineto\n", a.x, a.y+r);
		fprintf(file, "  %d %d %d 180 360 arc\n", a.x+r, a.y+r, r);
	} else {
		r = h/2;
		fprintf(file, "  %d %d moveto\n", b.x-r, a.y);
		fprintf(file, "  %d %d %d -90 90 arc\n", b.x-r, a.y+r, r);
		fprintf(file, "  %d %d lineto\n", a.x+r, b.y);
		fprintf(file, "  %d %d %d 90 270 arc\n", a.x+r, a.y+r, r);
	}
}


static void ps_rpad(FILE *file, const struct inst *inst, int show_name)
{
	fprintf(file, "0 setgray %d setlinewidth\n", PS_HATCH_LINE);
	ps_rounded_rect(file, inst->base, inst->u.pad.other);
	fprintf(file, "  closepath gsave %s grestore stroke\n",
	    hatch(inst->u.pad.layers));

	if (show_name && !inst->u.pad.hole)
		ps_pad_name(file, inst);
}


static void ps_hole(FILE *file, const struct inst *inst, int show_name)
{
	fprintf(file, "1 setgray %d setlinewidth\n", PS_RIM_LINE);
	ps_rounded_rect(file, inst->base, inst->u.hole.other);
	fprintf(file, "  closepath gsave fill grestore\n");
	fprintf(file, "  0 setgray stroke\n");

	if (show_name && inst->u.hole.pad)
		ps_pad_name(file, inst->u.hole.pad);
}


static void ps_line(FILE *file, const struct inst *inst)
{
	struct coord a = inst->base;
	struct coord b = inst->u.rect.end;

	fprintf(file, "1 setlinecap 0.5 setgray %d setlinewidth\n",
	    inst->u.rect.width);
	fprintf(file, "  %d %d moveto %d %d lineto stroke\n",
	    a.x, a.y, b.x, b.y);
}


static void ps_rect(FILE *file, const struct inst *inst)
{
	struct coord a = inst->base;
	struct coord b = inst->u.rect.end;

	fprintf(file, "1 setlinecap 0.5 setgray %d setlinewidth\n",
	    inst->u.rect.width);
	fprintf(file, "  %d %d moveto\n", a.x, a.y);
	fprintf(file, "  %d %d lineto\n", b.x, a.y);
	fprintf(file, "  %d %d lineto\n", b.x, b.y);
	fprintf(file, "  %d %d lineto\n", a.x, b.y);
	fprintf(file, "  closepath stroke\n");
}


static void ps_arc(FILE *file, const struct inst *inst)
{
	double a1, a2;

	a1 = inst->u.arc.a1;
	a2 = inst->u.arc.a2;
	if (a2 <= a1)
		a2 += 360;

	fprintf(file, "1 setlinecap 0.5 setgray %d setlinewidth\n",
	    inst->u.arc.width);
	fprintf(file, "  newpath %d %d %d %f %f arc stroke\n",
	    inst->base.x, inst->base.y, inst->u.arc.r, a1, a2);
}


static void ps_frame(FILE *file, const struct inst *inst)
{
}


static void ps_arrow(FILE *file, struct coord from, struct coord to, int len,
    int angle)
{
	struct coord side, p;

	if (from.x == to.x && from.y == to.y) {
		side.x = 0;
		side.y = -len;
	} else {
		side = normalize(sub_vec(to, from), len);
	}

	p = add_vec(to, rotate(side, 180-angle));
	fprintf(file, "  %d %d moveto\n", p.x, p.y);
	fprintf(file, "  %d %d lineto\n", to.x, to.y);

	p = add_vec(to, rotate(side, 180+angle));
	fprintf(file, "  %d %d moveto\n", p.x, p.y);
	fprintf(file, "  %d %d lineto\n", to.x, to.y);
	fprintf(file, "  stroke\n");
}


static void ps_vec(FILE *file, const struct inst *inst)
{
	struct coord a, b, c, d;
	char *s, *sx, *sy;

	a = inst->base;
	b = inst->u.vec.end;
	fprintf(file, "1 setlinecap 0 setgray %d setlinewidth\n", PS_VEC_LINE);
	fprintf(file, "  %d %d moveto\n", a.x, a.y);
	fprintf(file, "  %d %d lineto\n", b.x, b.y);
	fprintf(file, "  stroke\n");

	ps_arrow(file, a, b, PS_VEC_ARROW_LEN, PS_VEC_ARROW_ANGLE);

	if (!active_params.label_vecs)
		return;

	sx = unparse(inst->vec->x);
	sy = unparse(inst->vec->y);
	s = stralloc_printf("(%s, %s)", sx, sy);
	free(sx);
	free(sy);
	c = add_vec(a, b);
	d = sub_vec(b, a);
	fprintf(file, "gsave %d %d moveto\n", c.x/2, c.y/2);
	fprintf(file, "    /Helvetica-Bold findfont dup\n");
	fprintf(file, "    ");
	ps_string(file, s);
	fprintf(file, " %d %d realsize\n",
	    (int) (dist_point(a, b)-2*PS_VEC_ARROW_LEN),
	    PS_VEC_TEXT_HEIGHT);
	fprintf(file, "    boxfont\n");
	fprintf(file, "    %f rotate\n", atan2(d.y, d.x)/M_PI*180);
	fprintf(file, "    ");
	ps_string(file, s);
	fprintf(file, " %d realsize pop 0 hcenter\n", PS_VEC_BASE_OFFSET);
	fprintf(file, "    show grestore\n");
	free(s);
}


/* ----- Measurements ------------------------------------------------------ */


static unit_type guesstimate_text_height(const char *s, unit_type width,
    double zoom)
{
	return width/strlen(s)*TEXT_HEIGHT_FACTOR*zoom;
}


static void ps_meas(FILE *file, const struct inst *inst,
    enum curr_unit unit, double zoom)
{
	struct coord a0, b0, a1, b1;
	struct coord c, d;
	char *s;
	unit_type height, width, offset;

	a0 = inst->base;
	b0 = inst->u.meas.end;
	project_meas(inst, &a1, &b1);
	fprintf(file, "1 setlinecap 0 setgray %d realsize setlinewidth\n",
	    PS_MEAS_LINE);
	fprintf(file, "  %d %d moveto\n", a0.x, a0.y);
	fprintf(file, "  %d %d lineto\n", a1.x, a1.y);
	fprintf(file, "  %d %d lineto\n", b1.x, b1.y);
	fprintf(file, "  %d %d lineto\n", b0.x, b0.y);
	fprintf(file, "  stroke\n");

	ps_arrow(file, a1, b1, PS_MEAS_ARROW_LEN, PS_MEAS_ARROW_ANGLE);
	ps_arrow(file, b1, a1, PS_MEAS_ARROW_LEN, PS_MEAS_ARROW_ANGLE);

	s = format_len(inst->obj->u.meas.label ? inst->obj->u.meas.label : "",
	    dist_point(a1, b1), unit);

	c = add_vec(a1, b1);
	d = sub_vec(b1, a1);

	/*
	 * First try: put text between the arrows
	 */
	width = dist_point(a1, b1)-1.5*PS_MEAS_ARROW_LEN;
	offset = PS_MEAS_BASE_OFFSET;
	height = 0;
	if (guesstimate_text_height(s, width, zoom) < PS_MEAS_MIN_HEIGHT) {
#if 0
fprintf(stderr, "%s -> width %d height %d vs. %d\n",
    s, width, guesstimate_text_height(s, width, zoom), PS_MEAS_MIN_HEIGHT);
#endif
		/*
		 * Second try: push it above the arrows
		 */
		width = dist_point(a1, b1);
		offset +=
		    PS_MEAS_ARROW_LEN*sin(PS_MEAS_ARROW_ANGLE*M_PI/180)*zoom;

		if (guesstimate_text_height(s, width, zoom) <
		    PS_MEAS_MIN_HEIGHT) {
			height = PS_MEAS_MIN_HEIGHT;
			width = strlen(s)*height;
		}
	}

	if (height) {
		fprintf(file, "gsave %d %d moveto\n", c.x/2, c.y/2);
		fprintf(file, "    /Helvetica-Bold findfont dup\n");
		fprintf(file, "    ");
		ps_string(file, s);
		fprintf(file, " %d realsize %d realsize\n", width, height);
		fprintf(file, "    boxfont\n");
		fprintf(file, "    %f rotate\n", atan2(d.y, d.x)/M_PI*180);
		fprintf(file, "    ");
		ps_string(file, s);
		fprintf(file, " %d realsize hcenter\n", offset);
		fprintf(file, "    show grestore\n");
	} else {
		fprintf(file, "gsave %d %d moveto\n", c.x/2, c.y/2);
		fprintf(file, "    /Helvetica-Bold findfont dup\n");
		fprintf(file, "    ");
		ps_string(file, s);
		fprintf(file, " %d %d realsize\n", width, PS_MEAS_TEXT_HEIGHT);
		fprintf(file, "    boxfont\n");
		fprintf(file, "    %f rotate\n", atan2(d.y, d.x)/M_PI*180);
		fprintf(file, "    ");
		ps_string(file, s);
		fprintf(file, " %d realsize hcenter\n", offset);
		fprintf(file, "    show grestore\n");
	}
	free(s);
}


/* ----- Print layers ------------------------------------------------------ */


static void ps_background(FILE *file, enum inst_prio prio,
     const struct inst *inst)
{
	switch (prio) {
	case ip_line:
		ps_line(file, inst);
		break;
	case ip_rect:
		ps_rect(file, inst);
		break;
	case ip_circ:
	case ip_arc:
		ps_arc(file, inst);
		break;
	default:
		break;
	}
}


static void ps_foreground(FILE *file, enum inst_prio prio,
     const struct inst *inst, double zoom)
{
	switch (prio) {
	case ip_pad_copper:
	case ip_pad_special:
		if (inst->obj->u.pad.rounded)
			ps_rpad(file, inst, active_params.show_pad_names);
		else
			ps_pad(file, inst, active_params.show_pad_names);
		break;
	case ip_hole:
		ps_hole(file, inst, active_params.show_pad_names);
		break;
	case ip_vec:
		if (active_params.show_stuff)
			ps_vec(file, inst);
		break;
	case ip_frame:
		if (active_params.show_stuff)
			ps_frame(file, inst);
		break;
	case ip_meas:
		if (active_params.show_meas)
			ps_meas(file, inst, curr_unit, zoom);
		break;
	default:
		break;
	}
}


/* ----- Package level ----------------------------------------------------- */


static void ps_cross(FILE *file, const struct inst *inst)
{
	fprintf(file, "gsave 0 setgray %d setlinewidth\n", PS_CROSS_WIDTH);
	fprintf(file, "    [%d] 0 setdash\n", PS_CROSS_DASH);
	fprintf(file, "    %d 0 moveto %d 0 lineto\n",
	    inst->bbox.min.x, inst->bbox.max.x);
	fprintf(file, "    0 %d moveto 0 %d lineto\n",
	    inst->bbox.min.y, inst->bbox.max.y);
	fprintf(file, "    stroke grestore\n");
}


static void ps_draw_package(FILE *file, const struct pkg *pkg, double zoom)
{
	enum inst_prio prio;
	const struct inst *inst;

	fprintf(file, "gsave %f dup scale\n", zoom);
	ps_cross(file, pkgs->insts[ip_frame]);
	FOR_INST_PRIOS_UP(prio) {
		FOR_PKG_INSTS(pkgs, prio, inst)
			ps_background(file, prio, inst);
		FOR_PKG_INSTS(pkg, prio, inst)
			ps_background(file, prio, inst);
	}
	FOR_INST_PRIOS_UP(prio) {
		FOR_PKG_INSTS(pkgs, prio, inst)
			ps_foreground(file, prio, inst, zoom);
		FOR_PKG_INSTS(pkg, prio, inst)
			ps_foreground(file, prio, inst, zoom);
	}
	fprintf(file, "grestore\n");
}


/* ----- Object frames ----------------------------------------------------- */


static void ps_draw_frame(FILE *file, const struct pkg *pkg,
    const struct inst *outer, double zoom)
{
	enum inst_prio prio;
	const struct inst *inst;

	fprintf(file, "gsave %f dup scale\n", zoom);
	ps_cross(file, outer);
        FOR_INST_PRIOS_UP(prio) {
		FOR_PKG_INSTS(pkgs, prio, inst)
			if (inst->outer == outer)
				ps_background(file, prio, inst);
		FOR_PKG_INSTS(pkg, prio, inst)
			if (inst->outer == outer)
				ps_background(file, prio, inst);
	}
	FOR_INST_PRIOS_UP(prio) {
		FOR_PKG_INSTS(pkgs, prio, inst)
			if (inst->outer == outer)
				ps_foreground(file, prio, inst, zoom);
		FOR_PKG_INSTS(pkg, prio, inst)
			if (inst->outer == outer)
				ps_foreground(file, prio, inst, zoom);
	}
	fprintf(file, "grestore\n");
}


static int generate_frames(FILE *file, const struct pkg *pkg,
    const struct frame *frame, double zoom)
{
	const struct inst *inst;
	unit_type x, y, xa, ya;
	unit_type cx, cy, border;
	int ok;

	/*
	 * This doesn't work yet. The whole idea of just picking the current
	 * instance of each object and drawing it is flawed, since we may have
	 * very different sizes in a frame, so one big vector may dominate all
	 * the finer details.
	 *
	 * Also, the amount of text can be large and force tiny fonts to make
	 * things fit.
	 *
	 * A better approach would be to use a more qualitative display than a
	 * quantitative one, emphasizing the logical structure of the drawing
	 * and not the actual sizes.
	 *
 	 * This could be done by ranking vectors by current, average, maximum,
	 * etc. size, then let their size be determined by the amount of text
	 * that's needed and the size of subordinate vectors. One difficulty
	 * would be in making vectors with a fixed length ratio look correct,
	 * particularly 1:1.
	 *
	 * Furthermore, don't write on the vector but put the text horizontally
	 * on either the left or the right side.
	 *
	 * Frame references could be drawn by simply connecting a line to the
	 * area of the respective frame. And let's not forget that we also need
	 * to list the variables somewhere.
	 */
	return 0;

	while (frame) {
		if (frame->name)
			for (inst = pkg->insts[ip_frame]; inst;
			    inst = inst->next)
				if (inst->u.frame.ref == frame)
					goto found_frame;
		frame = frame->next;
	}
	if (!frame)
		return 1;

found_frame:
	border = PS_MEAS_TEXT_HEIGHT+PS_DIVIDER_WIDTH+PS_DIVIDER_BORDER/2;
	x = (inst->bbox.max.x-inst->bbox.min.x)*zoom+2*border;
	y = (inst->bbox.max.y-inst->bbox.min.y)*zoom+2*border;
	if (!get_box(x, y, &xa, &ya))
		return 0;

	/*
	 * Recurse down first, so that we only draw something if we can be sure
	 * that all the rest can be drawn too.
	 */

	ok = generate_frames(file, pkg, frame->next, zoom);
	if (!ok)
		return 0;


#if 1
fprintf(file, "0 setlinewidth 0.8 setgray\n");
fprintf(file, "%d %d moveto\n", xa+border, ya+border);
fprintf(file, "%d %d lineto\n", xa+x-border, ya+border);
fprintf(file, "%d %d lineto\n", xa+x-border, ya+y-border);
fprintf(file, "%d %d lineto\n", xa+border, ya+y-border);
fprintf(file, "closepath fill\n");
#endif
	cx = xa+x/2-(inst->bbox.min.x+inst->bbox.max.x)/2*zoom;
	cy = ya+y/2-(inst->bbox.min.y+inst->bbox.max.y)/2*zoom;

	fprintf(file, "%% Frame %s\n", frame->name ? frame->name : "(root)");
	fprintf(file, "gsave %d %d translate\n", cx, cy);
	ps_draw_frame(file, pkg, inst, zoom);
	fprintf(file, "grestore\n");

	return 1;
}


/* ----- Page level -------------------------------------------------------- */


static void ps_hline(FILE *file, int y)
{
	fprintf(file, "gsave %d setlinewidth\n", PS_DIVIDER_WIDTH);
	fprintf(file, "    %d %d moveto\n", -PAGE_HALF_WIDTH, y);
	fprintf(file, "    %d 0 rlineto stroke grestore\n", PAGE_HALF_WIDTH*2);
}


static void ps_header(FILE *file, const struct pkg *pkg)
{
	fprintf(file, "gsave %d %d moveto\n",
	    -PAGE_HALF_WIDTH, PAGE_HALF_HEIGHT-PS_HEADER_HEIGHT);
	fprintf(file, "    /Helvetica-Bold findfont dup\n");
	fprintf(file, "    ");
	ps_string(file, pkg->name);
	fprintf(file, " %d %d\n", PAGE_HALF_WIDTH, PS_HEADER_HEIGHT);
	fprintf(file, "    boxfont\n");
	fprintf(file, "    ");
	ps_string(file, pkg->name);
	fprintf(file, " show grestore\n");

	ps_hline(file, PAGE_HALF_HEIGHT-PS_HEADER_HEIGHT-PS_DIVIDER_BORDER);
}


static void ps_page(FILE *file, int page, const struct pkg *pkg)
{
	fprintf(file, "%%%%Page: %d %d\n", page, page);

	fprintf(file, "%%%%BeginPageSetup\n");
	fprintf(file,
"currentpagedevice /PageSize get\n"
"    aload pop\n"
"    2 div exch 2 div exch\n"
"    translate\n"
"    72 %d div 1000 div dup scale\n",
    (int) MIL_UNITS);
	fprintf(file, "%%%%EndPageSetup\n");
	fprintf(file, "[ /Title ");
	ps_string(file, pkg->name);
	fprintf(file, " /OUT pdfmark\n");
}


static void ps_unit(FILE *file,
    unit_type x, unit_type y, unit_type w, unit_type h)
{
	const char *s;

	switch (curr_unit) {
	case curr_unit_mm:
		s = "Dimensions in mm";
		break;
	case curr_unit_mil:
		s = "Dimensions in mil";
		break;
	case curr_unit_auto:
		return;
	default:
		abort();
	}

	fprintf(file, "gsave %d %d moveto\n", x, y);
	fprintf(file, "    /Helvetica findfont dup\n");
	fprintf(file, "    ");
	ps_string(file, s);
	fprintf(file, " %d %d\n", w, h);
	fprintf(file, "    boxfont\n");
	fprintf(file, "    ");
	ps_string(file, s);
	fprintf(file, " show grestore\n");
}


static void ps_package(FILE *file, const struct pkg *pkg, int page)
{
	struct bbox bbox;
	unit_type x, y;
	unit_type w, h;
	double f;
	unit_type c, d;
	int done;

	ps_page(file, page, pkg);
	ps_header(file, pkg);

	x = 2*PAGE_HALF_WIDTH-2*PS_DIVIDER_BORDER;
	y = PAGE_HALF_HEIGHT-PS_HEADER_HEIGHT-3*PS_DIVIDER_BORDER;

	bbox = inst_get_bbox();
	w = 2*(-bbox.min.x > bbox.max.x ? -bbox.min.x : bbox.max.x);
	h = 2*(-bbox.min.y > bbox.max.y ? -bbox.min.y : bbox.max.y);

	/*
	 * Zoom such that we can fit at least one drawing
	 */

	if (w > x/2 || h > y) {
		f = (double) x/w;
		if ((double) y/h < f)
			f = (double) y/h;
		if (f > 1)
			f = 1;
	} else {
		for (f = 20; f > 1; f--)
			if (x/(f+2) >= w && y/f >= h)
				break;
	}

	/*
	 * Decide if we have room for two, one, or zero smaller views
	 */

	c = y/2+PS_DIVIDER_BORDER;
	active_params = postscript_params;
	if (x/(f+2) >= w && y/3 > h) {
		/* main drawing */
		fprintf(file, "gsave %d %d translate\n",
		    (int) (x/(f+2)*f/2)-PAGE_HALF_WIDTH, c);
		ps_draw_package(file, pkg, f);

		active_params = minimal_params;

		/* divider */
		d = PAGE_HALF_WIDTH-2*x/(f+2);
		fprintf(file, "grestore gsave %d setlinewidth\n",
		    PS_DIVIDER_WIDTH);
		fprintf(file, "    %d %d moveto 0 %d rlineto stroke\n",
		    d-PS_DIVIDER_BORDER, PS_DIVIDER_BORDER, y);

		/* x1 package */
		fprintf(file, "grestore gsave %d %d translate\n",
		    (d+PAGE_HALF_WIDTH)/2, y/6*5+PS_DIVIDER_BORDER);
		ps_draw_package(file, pkg, 1);

		/* x2 package */
		fprintf(file, "grestore gsave %d %d translate\n",
		    (d+PAGE_HALF_WIDTH)/2, y/3+PS_DIVIDER_BORDER);
		ps_draw_package(file, pkg, 2);
	} else if (x/(f+1) >= w && y/2 > h) {
		/* main drawing */
		fprintf(file, "gsave %d %d translate\n",
		    (int) (x/(f+1)*f/2)-PAGE_HALF_WIDTH, c);
		ps_draw_package(file, pkg, f);

		active_params = minimal_params;

		/* divider */
		d = PAGE_HALF_WIDTH-x/(f+1);
		fprintf(file, "grestore gsave %d setlinewidth\n",
		    PS_DIVIDER_WIDTH);
		fprintf(file, "    %d %d moveto 0 %d rlineto stroke\n",
		    d-PS_DIVIDER_BORDER, PS_DIVIDER_BORDER, y);

		/* x1 package */
		fprintf(file, "grestore gsave %d %d translate\n",
		    (d+PAGE_HALF_WIDTH)/2, c);
		ps_draw_package(file, pkg, 1);
	} else {
		fprintf(file, "gsave 0 %d translate\n", c);
		ps_draw_package(file, pkg, f);
	}
	fprintf(file, "grestore\n");

	ps_unit(file, -PAGE_HALF_WIDTH, PS_DIVIDER_BORDER, PAGE_HALF_WIDTH,
	    PS_MISC_TEXT_HEIGHT);
	ps_hline(file, 0);

	/*
	 * Put the frames
	 *
	 * @@@ is it really a good idea to use the same zoom for all of them ?
	 */

	active_params.show_stuff = 1;
	active_params.label_vecs = 1;
	for (f = 20; f >= 0.1; f = f > 1 ? f-1 : f-0.1) {
		add_box(-PAGE_HALF_WIDTH, -PAGE_HALF_HEIGHT, PAGE_HALF_WIDTH,
		    -PS_DIVIDER_BORDER);
		done = generate_frames(file, pkg, frames, f);
		free_boxes();
		if (done)
			break;
	}

	fprintf(file, "showpage\n");
}


/* ----- File level -------------------------------------------------------- */


static void prologue(FILE *file, int pages)
{
	fprintf(file, "%%!PS-Adobe-3.0\n");
	fprintf(file, "%%%%Pages: %d\n", pages);
	fprintf(file, "%%%%EndComments\n");

	fprintf(file, "%%%%BeginDefaults\n");
	fprintf(file, "%%%%PageResources: font Helvetica Helvetica-Bold\n");
	fprintf(file, "%%%%EndDefaults\n");

	fprintf(file, "%%%%BeginProlog\n");

	fprintf(file,
"/dotpath {\n"
"    gsave flattenpath pathbbox clip newpath\n"
"    1 setlinecap %d setlinewidth\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    llx %d urx {\n"
"	lly %d ury {\n"
"	    1 index exch moveto 0 0 rlineto stroke\n"
"	} for\n"
"    } for\n"
"    grestore newpath } def\n", PS_DOT_DIAM, PS_DOT_DIST, PS_DOT_DIST);

	fprintf(file,
"/hatchpath {\n"
"     gsave flattenpath pathbbox clip newpath\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    lly ury sub %d urx llx sub {\n"	/* for -(ury-lly) to urx-llx */
"	llx add dup lly moveto\n"
"	ury lly sub add ury lineto stroke\n"
"    } for\n"
"    grestore newpath } def\n", PS_HATCH);

	fprintf(file,
"/backhatchpath {\n"
"     gsave flattenpath pathbbox clip newpath\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    0 %d ury lly sub urx llx sub add {\n"	/* for 0 to urx-llx+ury-lly */
"	llx add dup lly moveto\n"
"	ury lly sub sub ury lineto stroke\n"
"    } for\n"
"    grestore newpath } def\n", PS_HATCH);

fprintf(file,
"/crosspath {\n"
"    gsave hatchpath grestore backhatchpath } def\n");

	fprintf(file,
"/horpath {\n"
"     gsave flattenpath pathbbox clip newpath\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    lly %d ury {\n"			/* for lly to ury */
"	dup llx exch moveto\n"
"	urx exch lineto stroke\n"
"    } for\n"
"    grestore newpath } def\n", PS_STRIPE);

	/*
	 * Stack: font string width height factor -> factor
	 *
	 * Hack: sometimes, scalefont can't produce a suitable font and just
	 * gives us something zero-sized, which trips the division. We just
	 * ignore this case for now. Since maxfont is used in pairs, the
	 * second one may still succeed.
	 */

	fprintf(file,
"/sdiv { dup 0 eq { pop 1 } if div } def\n"
"/maxfont {\n"
"    gsave 0 0 moveto\n"
"    /f exch def /h exch def /w exch def\n"
"    exch f scalefont setfont\n"
"    false charpath flattenpath pathbbox\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    w urx llx sub sdiv h ury lly sub sdiv 2 copy gt { exch } if pop\n"
"    f mul grestore } def\n");

	/*
	 * Unrotate: - -> -
	 */

	fprintf(file,
"/getscale { matrix currentmatrix dup 0 get dup mul exch 1 get dup mul\n"
"    add sqrt } def\n");

	/*
	 * Stack: string -> string
	 */

	fprintf(file,
"/center {\n"
"    currentpoint /y exch def /x exch def\n"
"    gsave dup false charpath flattenpath pathbbox\n"
"    /ury exch def /urx exch def\n"
"    /lly exch def /llx exch def\n"
"    grestore\n"
"    x llx urx add 2 div sub y lly ury add 2 div sub rmoveto } def\n");

	/*
	 * Stack: string dist -> string
	 */

	fprintf(file,
"/hcenter {\n"
"    /off exch def\n"
"    gsave matrix setmatrix dup false charpath flattenpath pathbbox\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    grestore\n"
//"    /currscale getscale def\n"
"    llx urx sub 2 div\n"
//"    off lly sub rmoveto } def\n");
"    off rmoveto } def\n");

	/*
	 * Stack: string outline_width -> -
	 */

	fprintf(file,
"/showoutlined {\n"
"    gsave 2 mul setlinewidth 1 setgray\n"
"    dup false charpath flattenpath stroke grestore\n"
"    show } def\n");

	/*
	 * Stack: string -> string
	 */

	fprintf(file,
"/debugbox { gsave dup false charpath flattenpath pathbbox\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    0 setgray 100 setlinewidth\n"
"    llx lly urx llx sub ury lly sub rectstroke grestore } def\n");

	/*
	 * Stack: int -> int
	 */

	fprintf(file,
"/originalsize 1 0 matrix currentmatrix idtransform pop def\n"
"/realsize {\n"
"    254 div 72 mul 1000 div 0 matrix currentmatrix idtransform\n"
"    dup mul exch dup mul add sqrt\n"
"    originalsize div } def\n");

	/*
	 * Stack: font string x-size y-size -> -
	 */

	fprintf(file,
"/boxfont { 4 copy 1000 maxfont maxfont scalefont setfont } def\n");

	/*
	 * Ignore pdfmark. From
	 * http://www.adobe.com/devnet/acrobat/pdfs/pdfmark_reference.pdf
	 * Page 10, Example 1.1.
	 */

	fprintf(file,
"/pdfmark where { pop }\n"
"    { /globaldict where { pop globaldict } { userdict } ifelse"
"    /pdfmark /cleartomark load put } ifelse\n");

	fprintf(file, "%%%%EndProlog\n");
}


static void epilogue(FILE *file)
{
	fprintf(file, "%%%%EOF\n");
}


static int ps_for_all_pkg(FILE *file,
    void (*fn)(FILE *file, const struct pkg *pkg, int page))
{
	struct pkg *pkg;
	int pages;

	for (pkg = pkgs; pkg; pkg = pkg->next)
		if (pkg->name)
			pages++;
	prologue(file, pages);
	pages = 0;
	for (pkg = pkgs; pkg; pkg = pkg->next)
		if (pkg->name)
			fn(file, pkg, ++pages);
	epilogue(file);

	fflush(file);
	return !ferror(file);
}


int postscript(FILE *file)
{
	return ps_for_all_pkg(file, ps_package);
}


/*
 * Experimental. Doesn't work properly.
 */

static void ps_package_fullpage(FILE *file, const struct pkg *pkg, int page)
{
	unit_type cx, cy;
	struct bbox bbox;
	double fx, fy, f;

	ps_page(file, page, pkg);
	active_params = postscript_params;
	bbox = inst_get_bbox();
	cx = (bbox.min.x+bbox.max.x)/2;
	cy = (bbox.min.y+bbox.max.y)/2;
	if (active_params.zoom)
		f = active_params.zoom;
	else {
		fx = 2.0*PAGE_HALF_WIDTH/(bbox.max.x-bbox.min.x);
		fy = 2.0*PAGE_HALF_HEIGHT/(bbox.max.y-bbox.min.y);
		f = fx < fy ? fx : fy;
	}
	fprintf(file, "%d %d translate\n", (int) (-cx*f), (int) (-cy*f));
	ps_draw_package(file, pkg, f);
	fprintf(file, "showpage\n");
}


int postscript_fullpage(FILE *file)
{
	return ps_for_all_pkg(file, ps_package_fullpage);
}
