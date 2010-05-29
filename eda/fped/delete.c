/*
 * delete.c - Object deletion
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>
#include <assert.h>

#include "util.h"
#include "error.h"
#include "expr.h"
#include "obj.h"
#include "delete.h"


static struct deletion {
	enum del_type {
		dt_vec,
		dt_obj,
		dt_frame,
		dt_table,
		dt_row,
		dt_column,
		dt_loop,
	} type;
	union {
		struct {
			struct frame *ref;
			struct frame *prev;
		} frame;
		struct {
			struct vec *ref;
			struct vec *prev;
		} vec;
		struct {
			struct obj *ref;
			struct obj *prev;
		} obj;
		struct {
			struct table *ref;
			struct table *prev;
		} table;
		struct {
			struct row *ref;
			struct row *prev;
		} row;
		struct column {
			struct var *var;
			struct value *values;
			struct table *table;
			int n;
		} col;
		struct {
			struct loop *ref;
			struct loop *prev;
		} loop;
	} u;
	int group;
	struct deletion *next;
} *deletions = NULL;

static int groups = 0;


static void do_delete_vec(struct vec *vec);
static void do_delete_obj(struct obj *obj);


/* ----- helper functions -------------------------------------------------- */


static struct deletion *new_deletion(enum del_type type)
{
	struct deletion *del;

	del = alloc_type(struct deletion);
	del->type = type;
	del->group = groups;
	del->next = deletions;
	deletions = del;
	return del;
}


static void reset_active_ref(struct frame *ref)
{
	const struct frame *frame;
	struct obj *obj = NULL;

	for (frame = frames; frame; frame = frame->next)
		for (obj = frame->objs; obj; obj = obj->next)
			if (obj->type == ot_frame && obj->u.frame.ref == ref)
				break;
	ref->active_ref = obj;
}


/* ----- vectors ----------------------------------------------------------- */


static void destroy_vec(struct vec *vec)
{
	free_expr(vec->x);
	free_expr(vec->y);
	free(vec);
}


static void delete_vecs_by_ref(struct vec *vecs, const struct vec *ref)
{
	while (vecs) {
		if (vecs->base == ref)
			do_delete_vec(vecs);
		vecs = vecs->next;
	}
}


static int obj_has_ref(const struct obj *obj, const struct vec *ref)
{
	if (obj->base == ref)
		return 1;
	switch (obj->type) {
	case ot_frame:
		return 0;
	case ot_line:
		return obj->u.line.other == ref;
	case ot_rect:
		return obj->u.rect.other == ref;
	case ot_pad:
		return obj->u.pad.other == ref;
	case ot_hole:
		return obj->u.hole.other == ref;
	case ot_arc:
		return obj->u.arc.start == ref || obj->u.arc.end == ref;
	case ot_meas:
		return obj->u.meas.high == ref;
	default:
		abort();
	}
}


static void delete_objs_by_ref(struct obj **objs, const struct vec *ref)
{
	struct obj *obj;

	for (obj = *objs; obj; obj = obj->next)
		if (obj_has_ref(obj, ref))
			do_delete_obj(obj);
}


static void do_delete_vec(struct vec *vec)
{
	struct vec *walk, *prev;
	struct deletion *del;

	prev = NULL;
	for (walk = vec->frame->vecs; walk != vec; walk = walk->next)
		prev = walk;
	if (prev)
		prev->next = vec->next;
	else
		vec->frame->vecs = vec->next;
	del = new_deletion(dt_vec);
	del->u.vec.ref = vec;
	del->u.vec.prev = prev;

	delete_vecs_by_ref(vec->frame->vecs, vec);
	delete_objs_by_ref(&vec->frame->objs, vec);
	/*
	 * Catch measurements. During final cleanup, we may operate on an empty
	 * list of frames, hence the test.
	 */
	if (frames)
		delete_objs_by_ref(&frames->objs, vec);
}


void delete_vec(struct vec *vec)
{
	groups++;
	do_delete_vec(vec);
}


static void undelete_vec(struct vec *vec, struct vec *prev)
{
	if (prev) {
		assert(vec->next == prev->next);
		prev->next = vec;
	} else {
		assert(vec->next == vec->frame->vecs);
		vec->frame->vecs = vec;
	}
}


/* ----- objects ----------------------------------------------------------- */


static void destroy_obj(struct obj *obj)
{
	switch (obj->type) {
	case ot_frame:
		if (obj->u.frame.ref->active_ref == obj)
			reset_active_ref(obj->u.frame.ref);
		break;
	case ot_pad:
		free(obj->u.pad.name);
		break;
	case ot_hole:
		break;
	case ot_line:
		if (obj->u.line.width)
			free_expr(obj->u.line.width);
		break;
	case ot_rect:
		if (obj->u.rect.width)
			free_expr(obj->u.rect.width);
		break;
	case ot_arc:
		if (obj->u.arc.width)
			free_expr(obj->u.arc.width);
		break;
	case ot_meas:
		if (obj->u.meas.label)
			free(obj->u.meas.label);
		if (obj->u.meas.offset)
			free_expr(obj->u.meas.offset);
		break;
	default:
		abort();
	}
	free(obj);
}


static void do_delete_obj(struct obj *obj)
{
	struct obj *walk, *prev;
	struct deletion *del;

	prev = NULL;
	for (walk = obj->frame->objs; walk != obj; walk = walk->next)
		prev = walk;
	if (prev)
		prev->next = obj->next;
	else
		obj->frame->objs = obj->next;
	del = new_deletion(dt_obj);
	del->u.obj.ref = obj;
	del->u.obj.prev = prev;
	if (obj->type == ot_frame && obj->u.frame.ref->active_ref == obj)
		reset_active_ref(obj->u.frame.ref);
}


void delete_obj(struct obj *obj)
{
	groups++;
	do_delete_obj(obj);
}


static void undelete_obj(struct obj *obj, struct obj *prev)
{
	if (prev) {
		assert(obj->next == prev->next);
		prev->next = obj;
	} else {
		assert(obj->next == obj->frame->objs);
		obj->frame->objs = obj;
	}
}



/* ----- rows -------------------------------------------------------------- */


static void destroy_row(struct row *row)
{
	struct value *next_value;

	while (row->values) {
		next_value = row->values->next;
		free_expr(row->values->expr);
		free(row->values);
		row->values = next_value;
	}
	free(row);
}


void delete_row(struct row *row)
{
	struct deletion *del;
	struct row *walk, *prev;

	groups++;
	prev = NULL;
	for (walk = row->table->rows; walk != row; walk = walk->next)
		prev = walk;
	if (prev)
		prev->next = row->next;
	else
		row->table->rows = row->next;
	del = new_deletion(dt_row);
	del->u.row.ref = row;
	del->u.row.prev = prev;
}


static void undelete_row(struct row *row, struct row *prev)
{
	if (prev) {
		assert(row->next == prev->next);
		prev->next = row;
	} else {
		assert(row->next == row->table->rows);
		row->table->rows = row;
	}
}


/* ----- columns ----------------------------------------------------------- */


void delete_column(struct table *table, int n)
{
	struct deletion *del;
	struct column *col;
	struct var **var;
	struct row *row;
	struct value **next, **value;
	int i;

	groups++;

	del = new_deletion(dt_column);
	col = &del->u.col;
	col->table = table;
	col->n = n;

	var = &table->vars;
	for (i = 0; i != n; i++)
		var = &(*var)->next;
	col->var = *var;
	*var = (*var)->next;

	next = &col->values;
	for (row = table->rows; row; row = row->next) {
		value = &row->values;
		for (i = 0; i != n; i++)
			value = &(*value)->next;
		*next = *value;
		*value = (*value)->next;
		next = &(*next)->next;
	}
	*next = NULL;
}


static void undelete_column(const struct column *col)
{
	struct var **var;
	struct row *row;
	struct value **anchor, *value, *next;
	int i;

	var = &col->table->vars;
	for (i = 0; i != col->n; i++)
		var = &(*var)->next;
	col->var->next = *var;
	*var = col->var;

	value = col->values;
	for (row = col->table->rows; row; row = row->next) {
		anchor = &row->values;
		for (i = 0; i != col->n; i++)
			anchor = &(*anchor)->next;
		next = value->next;
		value->next = *anchor;
		*anchor = value;
		value = next;
	}
}


/* ----- tables ------------------------------------------------------------ */


static void destroy_table(struct table *table)
{
	struct var *next_var;

	while (table->vars) {
		next_var = table->vars->next;
		free(table->vars);
		table->vars = next_var;
	}
	while (table->rows) {
		delete_row(table->rows);
		destroy();
	}
	free(table);
}


void delete_table(struct table *table)
{
	struct frame *frame = table->vars->frame;
	struct deletion *del;
	struct table *walk, *prev;

	groups++;
	prev = NULL;
	for (walk = frame->tables; walk != table; walk = walk->next)
		prev = walk;
	if (prev)
		prev->next = table->next;
	else
		frame->tables = table->next;
	del = new_deletion(dt_table);
	del->u.table.ref = table;
	del->u.table.prev = prev;
}


static void undelete_table(struct table *table, struct table *prev)
{
	struct frame *frame = table->vars->frame;

	if (prev) {
		assert(table->next == prev->next);
		prev->next = table;
	} else {
		assert(table->next == frame->tables);
		frame->tables = table;
	}
}


/* ----- loops ------------------------------------------------------------- */


static void destroy_loop(struct loop *loop)
{
	free_expr(loop->from.expr);
	free_expr(loop->to.expr);
	free(loop);
}


void delete_loop(struct loop *loop)
{
	struct frame *frame = loop->var.frame;
	struct deletion *del;
	struct loop *walk, *prev;

	groups++;
	prev = NULL;
	for (walk = frame->loops; walk != loop; walk = walk->next)
		prev = walk;
	if (prev)
		prev->next = loop->next;
	else
		frame->loops = loop->next;
	del = new_deletion(dt_loop);
	del->u.loop.ref = loop;
	del->u.loop.prev = prev;
}


static void undelete_loop(struct loop *loop, struct loop *prev)
{
	struct frame *frame = loop->var.frame;

	if (prev) {
		assert(loop->next == prev->next);
		prev->next = loop;
	} else {
		assert(loop->next == frame->loops);
		frame->loops = loop;
	}
}


/* ----- frames ------------------------------------------------------------ */


static void destroy_frame(struct frame *frame)
{
	while (frame->tables) {
		delete_table(frame->tables);
		destroy();
	}
	while (frame->loops) {
		delete_loop(frame->loops);
		destroy();
	}
	while (frame->vecs) {
		delete_vec(frame->vecs);
		destroy();
	}
	while (frame->objs) {
		delete_obj(frame->objs);
		destroy();
	}
	free(frame);
}


static int qual_ref(const struct frame_qual *qual, const struct frame *ref)
{
	while (qual) {
		if (qual->frame == ref)
			return 1;
		qual = qual->next;
	}
	return 0;
}


static void delete_references(const struct frame *ref)
{
	struct frame *frame;
	struct obj *obj;

	for (frame = frames; frame; frame = frame->next)
		for (obj = frame->objs; obj; obj = obj->next)
			switch (obj->type) {
			case ot_frame:
				if (obj->u.frame.ref == ref)
					do_delete_obj(obj);
				break;
			case ot_meas:
				if (obj->base->frame == ref ||
				    obj->u.meas.high->frame == ref ||
				    qual_ref(obj->u.meas.low_qual, ref) ||
				    qual_ref(obj->u.meas.high_qual, ref))
					do_delete_obj(obj);
				break;
			default:
				break;
			}
	for (obj = ref->objs; obj; obj = obj->next)
		if (obj->type == ot_frame)
			if (obj->u.frame.ref->active_ref == obj)
				reset_active_ref(obj->u.frame.ref);
}


void delete_frame(struct frame *frame)
{
	struct deletion *del;
	struct frame *walk;
	groups++;

	del = new_deletion(dt_frame);
	del->u.frame.ref = frame;
	del->u.frame.prev = NULL;
	for (walk = frames; walk != frame; walk = walk->next)
		del->u.frame.prev = walk;
	if (del->u.frame.prev)
		del->u.frame.prev->next = frame->next;
	else
		frames = frame->next; /* hmm, deleting the root frame ? */

	delete_references(frame);
}


static void undelete_frame(struct frame *frame, struct frame *prev)
{
	if (prev) {
		assert(frame->next == prev->next);
		prev->next = frame;
	} else {
		assert(frame->next == frames);
		frames = frame;
	}
}


/* ----- destroy/undelete interface ---------------------------------------- */


static void destroy_one(void)
{
	struct deletion *del;

	del = deletions;
	switch (del->type) {
	case dt_vec:
		destroy_vec(del->u.vec.ref);
		break;
	case dt_obj:
		destroy_obj(del->u.obj.ref);
		break;
	case dt_frame:
		destroy_frame(del->u.frame.ref);
		break;
	case dt_loop:
		destroy_loop(del->u.loop.ref);
		break;
	case dt_table:
		destroy_table(del->u.table.ref);
		break;
	case dt_row:
		destroy_row(del->u.row.ref);
		break;
	case dt_column:
		abort(); /* @@@ later */
		break;
	default:
		abort();
	}
	deletions = del->next;
	free(del);
}


void destroy(void)
{
	int group;

	assert(deletions);
	group = deletions->group;
	while (deletions && deletions->group == group)
		destroy_one();
}


static int undelete_one(void)
{
	struct deletion *del;

	if (!deletions)
		return 0;
	del = deletions;
	switch (del->type) {
	case dt_vec:
		undelete_vec(del->u.vec.ref, del->u.vec.prev);
		break;
	case dt_obj:
		undelete_obj(del->u.obj.ref, del->u.obj.prev);
		break;
	case dt_frame:
		undelete_frame(del->u.frame.ref, del->u.frame.prev);
		break;
	case dt_loop:
		undelete_loop(del->u.loop.ref, del->u.loop.prev);
		break;
	case dt_table:
		undelete_table(del->u.table.ref, del->u.table.prev);
		break;
	case dt_row:
		undelete_row(del->u.row.ref, del->u.row.prev);
		break;
	case dt_column:
		undelete_column(&del->u.col);
		break;
	default:
		abort();
	}
	deletions = del->next;
	free(del);
	return 1;
}


int undelete(void)
{
	int group;

	if (!deletions)
		return 0;
	group = deletions->group;
	while (deletions && deletions->group == group)
		undelete_one();
	return 1;
}


void purge(void)
{
	while (deletions)
		destroy();
}
