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
	} type;
	union {
		struct {
			struct vec *ref;
			struct vec *prev;
		} vec;
		struct {
			struct obj *ref;
			struct obj *prev;
		} obj;
	} u;
	struct deletion *next;
} *deletions = NULL;


static struct deletion *new_deletion(enum del_type type)
{
	struct deletion *del;

	del = alloc_type(struct deletion);
	del->type = type;
	del->next = deletions;
	deletions = del;
	return del;
}


/* ----- vectors ----------------------------------------------------------- */


static void dereference_vec(struct vec *vec)
{
	assert(!vec->n_refs);
	put_vec(vec->base);
}


static void destroy_vec(struct vec *vec)
{
	assert(!vec->n_refs);
	free_expr(vec->x);
	free_expr(vec->y);
	free(vec);
}


static void rereference_vec(struct vec *vec)
{
	get_vec(vec->base);
}


int delete_vec(struct vec *vec)
{
	struct vec *walk, *prev;
	struct deletion *del;

	if (vec->n_refs) {
		fail("vector has %d reference%s", vec->n_refs,
		    vec->n_refs == 1 ? "" : "s");
		return 0;
	}
	prev = NULL;
	for (walk = vec->frame->vecs; walk != vec; walk = walk->next)
		prev = walk;
	if (prev)
		prev->next = vec->next;
	else
		vec->frame->vecs = vec->next;
	dereference_vec(vec);
	del = new_deletion(dt_vec);
	del->u.vec.ref = vec;
	del->u.vec.prev = prev;
	return 1;
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
	rereference_vec(vec);
}


/* ----- objects ----------------------------------------------------------- */


static void dereference_obj(struct obj *obj)
{
	switch (obj->type) {
	case ot_frame:
		/* nothing */
		break;
	case ot_pad:
		put_vec(obj->u.pad.other);
		break;
	case ot_line:
		put_vec(obj->u.line.other);
		break;
	case ot_rect:
		put_vec(obj->u.rect.other);
		break;
	case ot_arc:
		put_vec(obj->u.arc.start);
		put_vec(obj->u.arc.end);
		break;
	case ot_meas:
		put_vec(obj->u.meas.other);
		break;
	default:
		abort();
	}
	put_vec(obj->base);
}


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


static void rereference_obj(struct obj *obj)
{
	switch (obj->type) {
	case ot_frame:
		/* nothing */
		break;
	case ot_pad:
		get_vec(obj->u.pad.other);
		break;
	case ot_line:
		get_vec(obj->u.line.other);
		break;
	case ot_rect:
		get_vec(obj->u.rect.other);
		break;
	case ot_arc:
		get_vec(obj->u.arc.start);
		get_vec(obj->u.arc.end);
		break;
	case ot_meas:
		get_vec(obj->u.meas.other);
		break;
	default:
		abort();
	}
	get_vec(obj->base);
}


int delete_obj(struct obj *obj)
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
	dereference_obj(obj);
	del = new_deletion(dt_obj);
	del->u.obj.ref = obj;
	del->u.obj.prev = prev;
	return 1;
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
	rereference_obj(obj);
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
	default:
		abort();
	}
	deletions = del->next;
	free(del);
	return 1;
}


int undelete(void)
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
	default:
		abort();
	}
	deletions = del->next;
	free(del);
	return 1;
}
