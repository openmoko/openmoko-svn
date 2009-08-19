/*
 * postscript.c - Dump objects in Postscript
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

#include "coord.h"
#include "inst.h"
#include "gui_status.h"
#include "gui_inst.h"
#include "postscript.h"


#define	PS_DOT_DIST		mm_to_units(0.03)
#define	PS_DOT_DIAM		mm_to_units(0.01)
#define	PS_HATCH		mm_to_units(0.1)
#define	PS_HATCH_LINE		mm_to_units(0.015)
#define	PS_FONT_OUTLINE		mm_to_units(0.025)
#define	PS_MEAS_LINE		mm_to_units(0.015)
#define	PS_MEAS_ARROW_LEN	mm_to_units(0.07)
#define	PS_MEAS_ARROW_ANGLE	30
#define	PS_MEAS_TEXT_HEIGHT	mm_to_units(0.2)
#define	PS_MEAS_BASE_OFFSET	mm_to_units(0.05)
#define	PS_CROSS_WIDTH		mm_to_units(0.01)
#define	PS_CROSS_DASH		mm_to_units(0.1)


struct postscript_params postscript_params = {
	.zoom		= 10.0,
	.show_pad_names	= 1,
	.show_stuff	= 0,
	.label_vecs	= 0,
	.show_meas	= 1,
};


static void ps_pad_name(FILE *file, const struct inst *inst)
{
	struct coord a = inst->base;
	struct coord b = inst->u.pad.other;
	unit_type h, w;

	h = a.y-b.y;
	w = a.x-b.x;
	if (h < 0)
		h = -h;
	if (w < 0)
		w = -w;
	fprintf(file, "0 setgray /Helvetica-Bold findfont dup\n");
	fprintf(file, "   (%s) %d %d\n", inst->u.pad.name, w/2, h/2);
	fprintf(file, "   4 copy 100 maxfont\n");
	fprintf(file, "   maxfont scalefont setfont\n");
	fprintf(file, "   %d %d moveto\n", (a.x+b.x)/2, (a.y+b.y)/2);
	fprintf(file, "   (%s) center %d showoutlined newpath\n",
	    inst->u.pad.name, PS_FONT_OUTLINE);
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
	fprintf(file, "  closepath gsave crosspath grestore stroke\n");

	if (show_name)
		ps_pad_name(file, inst);
}


static void ps_rpad(FILE *file, const struct inst *inst, int show_name)
{
	struct coord a = inst->base;
	struct coord b = inst->u.pad.other;
	unit_type h, w, r;

	sort_coord(&a, &b);
	h = b.y-a.y;
	w = b.x-a.x;
	fprintf(file, "0 setgray %d setlinewidth\n", PS_HATCH_LINE);
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
	fprintf(file, "  closepath gsave hatchpath grestore stroke\n");

	if (show_name)
		ps_pad_name(file, inst);
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


static void ps_vec(FILE *file, const struct inst *inst)
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


static void ps_meas(FILE *file, const struct inst *inst,
    enum curr_unit unit)
{
	struct coord a0, b0, a1, b1;
	struct coord c, d;
	char *s;

	a0 = inst->base;
	b0 = inst->u.meas.end;
	project_meas(inst, &a1, &b1);
	fprintf(file, "1 setlinecap 0 setgray %d setlinewidth\n", PS_MEAS_LINE);
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
//s = stralloc_printf("%s%lgmm", meas->label ? meas->label : "", len);
	fprintf(file, "gsave %d %d moveto\n", c.x/2, c.y/2);
	fprintf(file, "    /Helvetica-Bold findfont dup\n");
	fprintf(file, "    (%s) %d %d\n", s,
	    (int) (dist_point(a1, b1)-1.5*PS_MEAS_ARROW_LEN),
	    PS_MEAS_TEXT_HEIGHT);
	fprintf(file, "    4 copy 100 maxfont maxfont scalefont setfont\n");
	fprintf(file, "    %f rotate\n", atan2(d.y, d.x)/M_PI*180);
	fprintf(file, "    (%s) %d hcenter\n", s, PS_MEAS_BASE_OFFSET);
	fprintf(file, "    show grestore\n");
	free(s);
}


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
     const struct inst *inst)
{
	switch (prio) {
	case ip_pad:
		if (inst->obj->u.pad.rounded)
			ps_rpad(file, inst, postscript_params.show_pad_names);
		else
			ps_pad(file, inst, postscript_params.show_pad_names);
		break;
	case ip_vec:
		if (postscript_params.show_stuff)
			ps_vec(file, inst);
		break;
	case ip_frame:
		if (postscript_params.show_stuff)
			ps_frame(file, inst);
		break;
	case ip_meas:
		if (postscript_params.show_meas)
			ps_meas(file, inst, curr_unit);
		break;
	default:
		break;
	}
}


static void ps_cross(FILE *file, const struct inst *inst)
{
	fprintf(file, "gsave 0 setgray %d setlinewidth\n", PS_CROSS_WIDTH);
	fprintf(file, "    [%d] 0 setdash\n", PS_CROSS_DASH);
	fprintf(file, "    %d 0 moveto %d 0 lineto\n",
	    inst->bbox.min.x, inst->bbox.max.x);
	fprintf(file, "    0 %d moveto 0 %d lineto\n",
	    inst->bbox.min.y, inst->bbox.max.y);
	fprintf(file, "    stroke grestore \n");
}


int postscript(FILE *file)
{
	enum inst_prio prio;
	const struct inst *inst;
	int i;

	fprintf(file, "%%!PS\n");

	fprintf(file,
"currentpagedevice /PageSize get\n"
"    aload pop\n"
"    2 div exch 2 div exch\n"
"    translate\n"
"    %f 72 mul %d div 1000 div dup scale\n",
    (double) postscript_params.zoom , (int) MIL_UNITS);

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
"    0 %d ury lly sub urx llx sub add {\n"	/* for 0 to urx-llx_ury-lly */
"	llx add dup lly moveto\n"
"	ury lly sub sub ury lineto stroke\n"
"    } for\n"
"    grestore newpath } def\n", PS_HATCH);

fprintf(file,
"/crosspath {\n"
"    gsave hatchpath grestore backhatchpath } def\n");

	/*
	 * Stack: font string width height factor -> factor
	 */

	fprintf(file,
"/maxfont {\n"
"    gsave 0 0 moveto\n"
"    /f exch def /h exch def /w exch def\n"
"    exch f scalefont setfont\n"
"    false charpath flattenpath pathbbox\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    w urx llx sub div h ury lly sub div 2 copy gt { exch } if pop\n"
"    f mul grestore } def\n");

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
"    gsave dup false charpath flattenpath pathbbox\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    grestore\n"
"    llx urx sub 2 div\n"
"    currentpoint exch pop lly sub off add rmoveto } def\n");

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

	ps_cross(file, pkgs->insts[ip_frame]);
	FOR_INST_PRIOS_UP(prio)
		FOR_ALL_INSTS(i, prio, inst)
			ps_background(file, prio, inst);
	FOR_INST_PRIOS_UP(prio)
		FOR_ALL_INSTS(i, prio, inst)
			ps_foreground(file, prio, inst);

	fprintf(file, "showpage\n");
	fprintf(file, "%%%%EOF\n");

	fflush(file);
	return !ferror(file);
}
