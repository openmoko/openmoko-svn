/*
 * meas.h - Measurements
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef MEAS_H
#define MEAS_H


#include "coord.h"
#include "expr.h"
#include "bitset.h"


typedef int (*lt_op_type)(struct coord a, struct coord b);

struct vec;
struct obj;

struct frame_qual {
	const struct frame *frame;
	struct frame_qual *next;
};

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
	/* low is obj->base */
	struct vec *high;
	struct expr *offset;
		
	/* frame qualifiers */
	struct frame_qual *low_qual;
	struct frame_qual *high_qual;
};

struct sample {
	struct coord pos;
	struct bitset *frame_set;
	struct sample *next;
};


extern int n_samples;


int lt_x(struct coord a, struct coord b);
int lt_y(struct coord a, struct coord b);
int lt_xy(struct coord a, struct coord b);

const struct sample *meas_find_min(lt_op_type lt, const struct sample *s,
    const struct bitset *qual);
const struct sample *meas_find_next(lt_op_type lt, const struct sample *s,
    struct coord ref, const struct bitset *qual);
const struct sample *meas_find_max(lt_op_type lt, const struct sample *s,
    const struct bitset *qual);


void reset_samples(struct sample **samples, int n);
void meas_start(void);
void meas_post(const struct vec *vec, struct coord pos,
    const struct bitset *frame_set);
int instantiate_meas(int n_frames);

#endif /* !MEAS_H */
