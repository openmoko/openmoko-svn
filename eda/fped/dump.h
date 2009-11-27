/*
 * dump.h - Dump objects in the native FPD format
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef DUMP_H
#define DUMP_H

#include <stdio.h>

#include "obj.h"


/*
 * vec       obj
 * --------------------------------------------------------------
 * NULL	     NULL	end of list
 * non-NULL  NULL	vector
 * NULL      non-NULL	object, no previous vector
 * non-NULL  non-NULL	object, with previous vector
 */

struct order {
	struct vec *vec;
	struct obj *obj;
};


const char *print_label(struct vec *vec);
char *print_vec(const struct vec *vec);
char *print_obj(const struct obj *obj, const struct vec *prev);
char *print_meas(const struct obj *obj);

struct order *order_frame(const struct frame *frame);

int dump(FILE *file);

#endif /* !DUMP_H */
