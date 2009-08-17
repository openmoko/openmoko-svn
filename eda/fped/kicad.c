/*
 * kicad.c - Dump objects in the KiCad board/module format
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdio.h>
#include <time.h>

#include "coord.h"
#include "inst.h"
#include "kicad.h"


enum kicad_layer {
	layer_bottom,		/* "copper" */
	layer_l15,
	layer_l14,
	layer_l13,
	layer_l12,
	layer_l11,
	layer_l10,
	layer_l9,
	layer_l8,
	layer_l7,
	layer_l6,
	layer_l5,
	layer_l4,
	layer_l3,
	layer_l2,
	layer_top,		/* "component" */
	layer_glue_bottom,	/* adhesive, copper side */
	layer_glue_top,		/* adhesive, component side */
	layer_paste_bottom,	/* solder paste */
	layer_paste_top,
	layer_silk_bottom,	/* silk screen */
	layer_silk_top,
	layer_mask_bottom,	/* solder mask */
	layer_mask_top,
	layer_draw,		/* general drawing */
	layer_comment,
	layer_eco1,
	layer_eco2,
	layer_edge,		/* edge */
};


static void kicad_pad(FILE *file, const struct inst *inst)
{
	struct coord min, max;
	unit_type tmp;

	min.x = units_to_kicad(inst->base.x);
	min.y = units_to_kicad(inst->base.y);
	max.x = units_to_kicad(inst->u.pad.other.x);
	max.y = units_to_kicad(inst->u.pad.other.y);

	if (min.x > max.x) {
		tmp = min.x;
		min.x = max.x;
		max.x = tmp;
	}
	if (min.y > max.y) {
		tmp = min.y;
		min.y = max.y;
		max.y = tmp;
	}

	fprintf(file, "$PAD\n");

	/*
	 * name, shape (rectangle), Xsize, Ysize, Xdelta, Ydelta, Orientation
 	 */
	fprintf(file, "Sh \"%s\" %c %d %d 0 0 0\n",
	    inst->u.pad.name, inst->obj->u.pad.rounded ? 'O' : 'R',
	    max.x-min.x, max.y-min.y);

	/*
	 * Attributes: pad type, N, layer mask
	 */
	fprintf(file, "At SMD N %8.8X\n",
	    (1 << layer_top) |
	    (1 << layer_paste_top) |
	    (1 << layer_mask_top));

	/*
	 * Position: Xpos, Ypos
	 */
	fprintf(file, "Po %d %d\n", (min.x+max.x)/2, -(min.y+max.y)/2);

	fprintf(file, "$EndPAD\n");
}


static void kicad_line(FILE *file, const struct inst *inst)
{
	/*
	 * Xstart, Ystart, Xend, Yend, Width, Layer
	 */
	fprintf(file, "DS %d %d %d %d %d %d\n",
	    units_to_kicad(inst->base.x),
	    -units_to_kicad(inst->base.y),
	    units_to_kicad(inst->u.rect.end.x),
	    -units_to_kicad(inst->u.rect.end.y),
	    units_to_kicad(inst->u.rect.width),
	    layer_silk_top);
}


static void kicad_rect(FILE *file, const struct inst *inst)
{
	unit_type xa, ya, xb, yb;
	unit_type width;

	xa = units_to_kicad(inst->base.x);
	ya = units_to_kicad(inst->base.y);
	xb = units_to_kicad(inst->u.rect.end.x);
	yb = units_to_kicad(inst->u.rect.end.y);
	width = units_to_kicad(inst->u.rect.width);

	fprintf(file, "DS %d %d %d %d %d %d\n",
	    xa, -ya, xa, -yb, width, layer_silk_top);
	fprintf(file, "DS %d %d %d %d %d %d\n",
	    xa, -yb, xb, -yb, width, layer_silk_top);
	fprintf(file, "DS %d %d %d %d %d %d\n",
	    xb, -yb, xb, -ya, width, layer_silk_top);
	fprintf(file, "DS %d %d %d %d %d %d\n",
	    xb, -ya, xa, -ya, width, layer_silk_top);
}


static void kicad_circ(FILE *file, const struct inst *inst)
{
	/*
	 * Xcenter, Ycenter, Xpoint, Ypoint, Width, Layer
	 */
	fprintf(file, "DC %d %d %d %d %d %d\n",
	    units_to_kicad(inst->base.x),
	    -units_to_kicad(inst->base.y),
	    units_to_kicad(inst->base.x),
	    -units_to_kicad(inst->base.y+inst->u.arc.r),
	    units_to_kicad(inst->u.arc.width),
	    layer_silk_top);
}


static void kicad_arc(FILE *file, const struct inst *inst)
{
	struct coord p;
	double a;

	/*
	 * The documentation says:
	 * Xstart, Ystart, Xend, Yend, Angle, Width, Layer
	 *
	 * But it's really:
	 * Xcenter, Ycenter, Xend, Yend, ...
	 */
	p = rotate_r(inst->base, inst->u.arc.r, inst->u.arc.a2);
	a = inst->u.arc.a2-inst->u.arc.a1;
	while (a <= 0)
		a += 360;
	while (a > 360)
		a -= 360;
	fprintf(file, "DA %d %d %d %d %d %d %d\n",
	    units_to_kicad(inst->base.x),
	    -units_to_kicad(inst->base.y),
	    units_to_kicad(p.x),
	    -units_to_kicad(p.y),
	    (int) (a*10.0),
	    units_to_kicad(inst->u.arc.width),
	    layer_silk_top);
}


static void kicad_inst(FILE *file, enum inst_prio prio, const struct inst *inst)
{
	switch (prio) {
	case ip_pad:
		kicad_pad(file, inst);
		break;
	case ip_line:
		kicad_line(file, inst);
		break;
	case ip_rect:
		kicad_rect(file, inst);
		break;
	case ip_circ:
		kicad_circ(file, inst);
		break;
	case ip_arc:
		kicad_arc(file, inst);
		break;
	default:
		/*
		 * Don't try to export vectors, frame references, or
		 * measurements.
		 */
		break;
	}
}


static void kicad_module(FILE *file, const struct pkg *pkg, time_t now)
{
	enum inst_prio prio;
	const struct inst *inst;

	/*
	 * Module library name
	 */
	fprintf(file, "$MODULE %s\n", pkg->name);

	/*
	 * Xpos = 0, Ypos = 0, 15 layers, last modification, timestamp,
	 * moveable, not autoplaced.
	 */
	fprintf(file, "Po 0 0 0 15 %8.8lX 00000000 ~~\n", (long) now);

	/*
	 * Module library name again
	 */
	fprintf(file, "Li %s\n", pkg->name);

#if 0 /* optional */
	/*
	 * Description
	 */
	fprintf(file, "Cd %s\n", pkg->name);
#endif

	/*
	 *
	 */
	fprintf(file, "Sc %8.8lX\n", (long) now);

	/*
	 * Attributes: SMD = listed in the automatic insertion list
	 */
	fprintf(file, "At SMD\n");

	/*
	 * Rotation cost: 0 for 90 deg, 0 for 180 deg, 0 = disable rotation
	 */
	fprintf(file, "Op 0 0 0\n");

	/*
	 * Text fields: Tn = field number, Xpos, Ypos, Xsize ("emspace"),
	 * Ysize ("emspace"), rotation, pen width, N (none), V = visible,
	 * comment layer. All dimensions are 1/10 mil.
	 */

	fprintf(file, "T0 0 -150 200 200 0 40 N V %d \"%s\"\n",
	    layer_comment, pkg->name);
	fprintf(file, "T1 0 150 200 200 0 40 N I %d \"Val*\"\n",
	    layer_comment);

	FOR_INST_PRIOS_UP(prio) {
		for (inst = pkgs->insts[prio]; inst; inst = inst->next)
			kicad_inst(file, prio, inst);
		for (inst = pkg->insts[prio]; inst; inst = inst->next)
			kicad_inst(file, prio, inst);
	}

	fprintf(file, "$EndMODULE %s\n", pkg->name);
}


int kicad(FILE *file)
{
	const struct pkg *pkg;
	time_t now = time(NULL);

	fprintf(file, "PCBNEW-LibModule-V1 %s", ctime(&now));

	fprintf(file, "$INDEX\n");
	for (pkg = pkgs; pkg; pkg = pkg->next)
		if (pkg->name)
			fprintf(file, "%s\n", pkg->name);
	fprintf(file, "$EndINDEX\n");

	for (pkg = pkgs; pkg; pkg = pkg->next)
		if (pkg->name)
			kicad_module(file, pkg, now);

	fprintf(file, "$EndLIBRARY\n");

	fflush(file);
	return !ferror(file);
}
