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
#include "postscript.h"


#define	DOT_DIST	mm_to_units(0.03)
#define	DOT_DIAM	mm_to_units(0.01)
#define	HATCH		mm_to_units(0.1)
#define	HATCH_LINE	mm_to_units(0.02)
#define	FONT_OUTLINE	mm_to_units(0.025)


struct postscript_params postscript_params = {
	.zoom		= 10.0,
	.show_pad_names	= 1,
	.show_stuff	= 0,
	.label_vecs	= 0,
	.show_meas	= 0,
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
	    inst->u.pad.name, FONT_OUTLINE);
}


static void ps_pad(FILE *file, const struct inst *inst, int show_name)
{
	struct coord a = inst->base;
	struct coord b = inst->u.pad.other;

	fprintf(file, "0 setgray %d setlinewidth\n", HATCH_LINE);
	fprintf(file, "  %d %d moveto\n", a.x, a.y);
	fprintf(file, "  %d %d lineto\n", b.x, a.y);
	fprintf(file, "  %d %d lineto\n", b.x, b.y);
	fprintf(file, "  %d %d lineto\n", a.x, b.y);
	fprintf(file, "  closepath gsave hatchpath grestore stroke\n");

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
	fprintf(file, "0 setgray %d setlinewidth\n", HATCH_LINE);
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


static void ps_meas(FILE *file, const struct inst *inst)
{
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
			ps_meas(file, inst);
		break;
	default:
		break;
	}
}


int postscript(FILE *file)
{
	enum inst_prio prio;
	const struct inst *inst;

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
"    gsave pathbbox clip newpath\n"
"    1 setlinecap %d setlinewidth\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    llx %d urx {\n"
"	lly %d ury {\n"
"	    1 index exch moveto 0 0 rlineto stroke\n"
"	} for\n"
"    } for\n"
"    grestore newpath } def\n", DOT_DIAM, DOT_DIST, DOT_DIST);

	fprintf(file,
"/hatchpath {\n"
"     gsave pathbbox clip newpath\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    lly ury sub %d urx llx sub {\n"	/* for -(ury-lly) to urx-llx */
"	llx add dup lly moveto\n"
"	ury lly sub add ury lineto stroke\n"
"    } for\n"
"    grestore newpath } def\n", HATCH);

	/*
	 * Stack: font string width height factor -> factor
	 */

	fprintf(file,
"/maxfont {\n"
"    gsave 0 0 moveto\n"
"    /f exch def /h exch def /w exch def\n"
"    exch f scalefont setfont\n"
"    false charpath pathbbox\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    w urx llx sub div h ury lly sub div 2 copy gt { exch } if pop\n"
"    f mul grestore } def\n");

	/*
	 * Stack: string -> string
	 */

	fprintf(file,
"/center {\n"
"    gsave dup false charpath pathbbox\n"
"    /ury exch def /urx exch def /lly exch def /llx exch def\n"
"    grestore\n"
"    llx urx sub 2 div lly ury sub 2 div rmoveto } def\n");

	/*
	 * Stack: string outline_width -> -
	 */

	fprintf(file,
"/showoutlined {\n"
"    gsave 2 mul setlinewidth 1 setgray\n"
"    dup false charpath stroke grestore\n"
"    show } def\n");

	FOR_INSTS_UP(prio, inst)
		ps_background(file, prio, inst);
	FOR_INSTS_UP(prio, inst)
		ps_foreground(file, prio, inst);

	fprintf(file, "showpage\n");
	fprintf(file, "%%%%EOF\n");

	fflush(file);
	return !ferror(file);
}
