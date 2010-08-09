/*
 * hole.c - Classify holes and connect them with pads
 *
 * Written 2010 by Werner Almesberger
 * Copyright 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include "error.h"
#include "inst.h"
#include "overlap.h"
#include "hole.h"


static int check_through_hole(struct inst *pad, struct inst *hole)
{
	if (!overlap(pad, hole, ao_none))
		return 1;
	if (!inside(hole, pad)) {
		fail("hole (line %d) not completely inside "
		    "pad \"%s\" (line %d)", hole->obj->lineno,
		    pad->u.pad.name, pad->obj->lineno);
		instantiation_error = pad->obj;
		return 0;
	}
	if (hole->u.hole.pad) {
		/*
		 * A hole can only be on several pads if the pads themselves
		 * overlap. We'll catch this error in refine_copper.
		 */
		return 1;
	}
	if (pad->u.pad.hole) {
		fail("pad \"%s\" (line %d) has multiple holes (lines %d, %d)",
		    pad->u.pad.name, pad->obj->lineno,
		    hole->obj->lineno, pad->u.pad.hole->obj->lineno);
		instantiation_error = pad->obj;
		return 0;
	}
	pad->u.pad.hole = hole;
	hole->u.hole.pad = pad;
	return 1;
}


static int connect_holes(const struct pkg *pkg)
{
	struct inst *pad, *hole;

	for (pad = pkg->insts[ip_pad_copper]; pad; pad = pad->next)
		for (hole = pkg->insts[ip_hole]; hole; hole = hole->next)
			if (!check_through_hole(pad, hole))
				return 0;
	return 1;
}


static void clear_links(const struct pkg *pkg)
{
	struct inst *pad, *hole;

	for (pad = pkg->insts[ip_pad_copper]; pad; pad = pad->next)
		pad->u.pad.hole = NULL;
	for (pad = pkg->insts[ip_pad_special]; pad; pad = pad->next)
		pad->u.pad.hole = NULL;
	for (hole = pkg->insts[ip_hole]; hole; hole = hole->next)
		hole->u.hole.pad = NULL;
}


int link_holes(void)
{
	const struct pkg *pkg;

	for (pkg = pkgs; pkg; pkg = pkg->next) {
		clear_links(pkg);
		if (!connect_holes(pkg))
			return 0;
	}
        return 1;
}
