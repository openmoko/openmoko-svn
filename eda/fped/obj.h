/*
 * obj.h - Object definition model
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef OBJ_H
#define OBJ_H

#include <gtk/gtk.h>

#include "expr.h"
#include "coord.h"


struct var {
	const char *name;
	struct var *next;

	/* back reference */
	struct frame *frame;

	/* for the GUI */
	GtkWidget *widget;

	/* for evaluation */
	int visited;
};

struct value {
	struct expr *expr;
	struct value *next;

	/* back reference */
	struct row *row;

	/* for the GUI */
	GtkWidget *widget;
};

struct row {
	struct value *values;
	struct row *next;

	/* back reference */
	struct table *table;
};

struct table {
	struct var *vars;
	struct row *rows;
	struct table *next;

	/* used during generation and when editing */
	struct row *curr_row;

	/* GUI use */
	int active;	/* n-th row is active, 0 based */
};

struct loop {
	struct var var;
	struct value from;
	struct value to;
	struct loop *next;

	/* used during generation */
	double curr_value;

	/* GUI use */
	int active;	/* n-th iteration is active, 0 based */

	/* for evaluation */
	int initialized;
};

struct vec {
	const char *name; /* NULL if anonymous */
	struct expr *x;
	struct expr *y;
	struct vec *base; /* NULL if frame */
	int n_refs;
	struct vec *next;

	/* used during generation */
	struct coord pos;

	/* used when editing */
	struct frame *frame;
};

struct frame {
	const char *name; /* NULL if top-level */
	struct table *tables;
	struct loop *loops;
	struct vec *vecs;
	struct obj *objs;
	struct frame *next;
	struct frame *prev; /* for the list of frames in the GUI */

	/* used during generation */
	const struct frame *curr_parent;

	/* for the GUI */
	GtkWidget *label;
};

enum obj_type {
	ot_frame,
	ot_rect,
	ot_pad,
	ot_line,
	ot_arc,
	ot_meas,
};

struct rect {
	struct vec *other; /* NULL if frame origin */
	struct expr *width;
};

struct pad {
	char *name;
	struct vec *other; /* NULL if frame origin */
};

struct arc {
	struct vec *start; /* NULL if frame origin */
	struct vec *end; /* NULL if this is a circle */
	struct expr *width;
};

struct meas {
	struct vec *other; /* NULL if frame origin */
	struct expr *offset;
};

struct obj {
	enum obj_type type;
	union {
		struct frame *frame;
		struct rect rect;
		struct rect line;
		struct pad pad;
		struct arc arc;
		struct meas meas;
	} u;
	struct vec *base;
	struct obj *next;
};


extern struct frame *frames;
extern struct frame *root_frame;
extern struct frame *active_frame;


int instantiate(void);

#endif /* !OBJ_H */
