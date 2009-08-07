/*
 * meas.h - Measurements
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef MEAS_H
#define MEAS_H

#include "obj.h"


typedef int (*lt_op_type)(struct coord a, struct coord b);


struct meas {
	enum meas_type {
		mt_xy_next,
		mt_x_next,
		mt_y_next,
		mt_xy_max,
		mt_x_max,
		mt_y_max,
		mt_n
	} type;
	char *label; /* or NULL */
	int inverted;
	struct vec *low;
	struct vec *high;
	struct expr *offset;
	struct meas *next;
};

struct sample;


extern struct meas *measurements;


int lt_x(struct coord a, struct coord b);
int lt_y(struct coord a, struct coord b);
int lt_xy(struct coord a, struct coord b);

struct coord meas_find_min(lt_op_type lt, const struct sample *s);
struct coord meas_find_next(lt_op_type lt, const struct sample *s,
    struct coord ref);
struct coord meas_find_max(lt_op_type lt, const struct sample *s);

void meas_start(void);
void meas_post(struct vec *vec, struct coord pos);
int instantiate_meas(void);

#endif /* !MEAS_H */
