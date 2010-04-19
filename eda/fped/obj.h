/*
 * obj.h - Object definition model
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
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


/*
 * Objects contain various fields that help to select instances under various
 * conditions. They are "current", "active", and "found":
 *
 * - current: the path taken while instantiating. E.g., we may make one frame
 *   reference the "current" reference of this frame and then recurse into it.
 *   "Current" is reset to a null value after instantiation is complete, to
 *   allow other functions (such as expression evaluation) to distinguish
 *   between instantiation and editing.
 *
 * - active: the path selected by the user, through the GUI. This allows the
 *   user to reach any instance, similar to how instantiation visits all
 *   instances. The difference to "current" is that "active" is persistent
 *   across instantiation while "current" iterates through all possible values
 *   during instantiation.
 *
 * - found: then clicking on an unselected instance, fped will try to activate
 *   this instance. In order to do so, it needs to determine which choices need
 *   to be activated to reach the instance. "Found" records this information.
 *   At the end of the search, all "found" choices become "active".
 *
 *   If, during the search, an instance can be reached with the "found" choice
 *   being equal to the choice active at that time, "found" will not be set to
 *   any other value. This prevents searches from affecting choices that play
 *   no role in the selection of the instance.
 */


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

	/* For searching */
	struct row *found_row;	/* NULL if not found yet */
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

	/* For searching */
	int found;	/* -1 if not found yet */

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

	/* For searching */
	struct obj *found_ref;	/* NULL if not found yet */

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
	const char *name; /* NULL if anonymous */
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


struct inst;

/*
 * Search callback from inst, invoked after the instance has been populated.
 */

void find_inst(const struct inst *inst);

/*
 * If invoking search_inst before calling "instantiate", loop and tables are
 * adjusted such that an instance matching the one passed to search_inst will
 * become active. Note that this doesn't necessarily succeed, in which case no
 * change is made. Also, if multiple matches are encountered, the result is
 * arbitrary.
 */

void search_inst(const struct inst *inst);

int obj_anchors(struct obj *obj, struct vec ***anchors);

int instantiate(void);
void obj_cleanup(void);

#endif /* !OBJ_H */
