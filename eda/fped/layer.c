/*
 * layer.c - PCB layers on a pad
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * We don't reject solder paste pads that don't cover anything yet.
 * That way, things can be constructed step by step without getting blue
 * screens all the time.
 */


#include <stdlib.h>

#include "error.h"
#include "overlap.h"
#include "inst.h"
#include "obj.h"
#include "layer.h"


/*
 * Shorthands for the layers we use in a general sense.
 */

#define LAYER_COPPER	(1 << layer_top)
#define LAYER_PASTE	(1 << layer_paste_top)
#define LAYER_MASK	(1 << layer_mask_top)


/* ----- Conversion between pad types and layer sets ----------------------- */


layer_type pad_type_to_layers(enum pad_type type)
{
	layer_type layers = 0;

	switch (type) {
	case pt_normal:
		layers = LAYER_PASTE;
		/* fall through */
	case pt_bare:
		layers |= LAYER_COPPER | LAYER_MASK;
		break;
	case pt_paste:
		layers = LAYER_PASTE;
		break;
	case pt_mask:
		layers = LAYER_MASK;
		break;
	default:
		abort();
	}
	return layers;
}


enum pad_type layers_to_pad_type(layer_type layers)
{
	if (layers & LAYER_COPPER) {
		if (layers & LAYER_PASTE)
			return pt_normal;
		return pt_bare;
	} else {
		if (layers & LAYER_PASTE)
			return pt_paste;
		if (layers & LAYER_MASK)
			return pt_mask;
		abort();
	}
}


/* ----- Refine layers after instantiation --------------------------------- */


static int refine_overlapping(struct inst *copper, struct inst *other)
{
	if (other->u.pad.layers & LAYER_PASTE) {
		copper->u.pad.layers &= ~LAYER_PASTE;
		if (!inside(other, copper)) {
			fail("solder paste without copper underneath "
			    "(\"%s\" line %d, \"%s\" line %d)",
			    copper->u.pad.name, copper->obj->lineno,
			    other->u.pad.name, other->obj->lineno);
			instantiation_error = other->obj;
			return 0;
		}
	}
	if (other->u.pad.layers & LAYER_MASK)
		copper->u.pad.layers &= ~LAYER_MASK;
	return 1;
}


static int refine_copper(const struct pkg *pkg_copper, struct inst *copper)
{
	const struct pkg *pkg;
	struct inst *other;

	for (pkg = pkgs; pkg; pkg = pkg->next) {
		/*
		 * Pads in distinct packages can happily coexist.
		 */
		if (pkg != pkgs && pkg_copper != pkgs && pkg_copper != pkg)
			continue;
		for (other = pkg->insts[ip_pad_copper]; other;
		    other = other->next)
			if (copper != other && overlap(copper, other)) {
				fail("overlapping copper pads "
				    "(\"%s\" line %d, \"%s\" line %d)",
				    copper->u.pad.name, copper->obj->lineno,
				    other->u.pad.name, other->obj->lineno);
				instantiation_error = copper->obj;
				return 0;
			}
		for (other = pkg->insts[ip_pad_special]; other;
		    other = other->next)
			if (overlap(copper, other))
				if (!refine_overlapping(copper, other))
					return 0;
	}
	return 1;
}


int refine_layers(void)
{
	const struct pkg *pkg;
	struct inst *copper;

	for (pkg = pkgs; pkg; pkg = pkg->next)
		for (copper = pkg->insts[ip_pad_copper]; copper;
		    copper = copper->next)
			if (!refine_copper(pkg, copper))
				return 0;
	return 1;
}
