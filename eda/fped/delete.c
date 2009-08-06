/*
 * delete.c - Object deletion
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
		struct {
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
	case ot_arc:
		return obj->u.arc.start == ref || obj->u.arc.end == ref;
	case ot_meas:
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

	delete_vecs_by_ref(vec->frame->vecs, vec);
	delete_objs_by_ref(&vec->frame->objs, vec);

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
		/* nothing */
		break;
	case ot_pad:
		free(obj->u.pad.name);
		break;
	case ot_line:
		free_expr(obj->u.line.width);
		break;
	case ot_rect:
		free_expr(obj->u.rect.width);
		break;
	case ot_arc:
		free_expr(obj->u.arc.width);
		break;
	case ot_meas:
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
	groups++;
}


/* ----- tables ------------------------------------------------------------ */


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


static void delete_references(const struct frame *ref)
{
	struct frame *frame;
	struct obj *obj;

	for (frame = frames; frame; frame = frame->next)
		for (obj = frame->objs; obj; obj = obj->next)
			if (obj->type == ot_frame)
				if (obj->u.frame.ref == ref)
					delete_obj(obj);
}


void delete_frame(struct frame *frame)
{
	struct deletion *del;

	groups++;
	delete_references(frame);

	del = new_deletion(dt_frame);
	del->u.frame.ref = frame;
	del->u.frame.prev = frame->prev;

	if (frame->next)
		frame->next->prev = frame->prev;
	if (frame->prev)
		frame->prev->next = frame->next;
	else
		frames = frame->next;
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
	frame->next->prev = frame;
}


/* ----- destroy/undelete interface ---------------------------------------- */


int destroy(void)
{
	struct deletion *del;

	if (!deletions)
		return 0;
	del = deletions;
	switch (del->type) {
	case dt_vec:
		destroy_vec(del->u.vec.ref);
		break;
	case dt_obj:
		destroy_obj(del->u.obj.ref);
		break;
	case dt_frame:
		abort();
		/* @@@ later */
		break;
	default:
		abort();
	}
	deletions = del->next;
	free(del);
	return 1;
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
