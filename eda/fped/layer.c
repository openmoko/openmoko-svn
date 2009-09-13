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


#include <stdlib.h>

#include "layer.h"


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
