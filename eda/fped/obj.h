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

#include <assert.h>
#include <gtk/gtk.h>

#include "expr.h"
#include "coord.h"
#include "meas.h"
#include "layer.h"


struct var {
	const char *name;
	struct var *next;

	/* back reference */
	struct frame *frame;
	struct table *table; /* NULL if loop */

	/* for the GUI */
	GtkWidget *widget;

	/* for evaluation */
	int visited;
};

struct value {
	struct expr *expr;
	struct value *next;

	/* back reference, NULL if loop */
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
	struct row *active_row;
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
	double n;	/* start value when it was active */
	int iterations;	/* iterations when it was active */

	/* for evaluation */
	int initialized;
};

struct sample;

struct vec {
	const char *name; /* NULL if anonymous */
	struct expr *x;
	struct expr *y;
	struct vec *base; /* NULL if frame */
	struct vec *next;

	/* used during generation */
	struct coord pos;

	/* used when editing */
	struct frame *frame;

	/* index into table of samples */
	int n;

	/* for re-ordering after a move */
	int mark;

	/* for the GUI */
	GtkWidget *list_widget; /* NULL if items aren't shown */
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

	/* generating and editing */
	struct obj *active_ref;

	/* for dumping */
	int dumped;

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

struct frame_ref {
	struct frame *ref;
	int lineno;
};

struct rect {
	struct vec *other; /* NULL if frame origin */
	struct expr *width;
};

struct pad {
	char *name;
	struct vec *other; /* NULL if frame origin */
	int rounded;
	enum pad_type type;
};

struct arc {
	struct vec *start; /* NULL if frame origin */
	struct vec *end; /* NULL if this is a circle */
	struct expr *width;
};

struct obj {
	enum obj_type type;
	union {
		struct frame_ref frame;
		struct rect rect;
		struct rect line;
		struct pad pad;
		struct arc arc;
		struct meas meas;
	} u;
	struct frame *frame;
	struct vec *base;
	struct obj *next;
	int lineno;

	/* for dumping */
	int dumped;

	/* for the GUI */
	GtkWidget *list_widget; /* NULL if items are not shown */
};


extern char *pkg_name;
extern struct frame *frames;
extern struct frame *root_frame;
extern struct frame *active_frame;
extern void *instantiation_error;


int instantiate(void);
void obj_cleanup(void);

#endif /* !OBJ_H */
