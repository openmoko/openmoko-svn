/*
 * dump.c - Dump objects in the native FPD format
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
#include <stdio.h>

#include "util.h"
#include "unparse.h"
#include "obj.h"
#include "dump.h"


/* ----- variables --------------------------------------------------------- */


static void dump_var(FILE *file, const struct table *table,
    const char *indent)
{
	char *s;

	s = unparse(table->rows->values->expr);
	fprintf(file, "%sset %s = %s\n\n", indent, table->vars->name, s);
	free(s);
}


static void dump_table(FILE *file, const struct table *table,
    const char *indent)
{
	const struct var *var;
	const struct row *row;
	const struct value *value;
	char *s;

	if (table->vars && !table->vars->next &&
	    table->rows && !table->rows->next) {
		dump_var(file, table, indent);
		return;
	}
	fprintf(file, "%stable\n%s    {", indent, indent);
	for (var = table->vars; var; var = var->next)
		fprintf(file, "%s %s", var == table->vars ? "" : ",",
		    var->name);
	fprintf(file, " }\n");
	for (row = table->rows; row; row = row->next) {
		fprintf(file, "%s    {", indent);
		for (value = row->values; value; value = value->next) {
			s = unparse(value->expr);
			fprintf(file, "%s %s",
			    value == row->values? "" : ",", s);
			free(s);
		}
		fprintf(file, " }\n");
	}
	fprintf(file, "\n");
}


static void dump_loop(FILE *file, const struct loop *loop, const char *indent)
{
	char *from, *to;

	from = unparse(loop->from.expr);
	to = unparse(loop->to.expr);
	fprintf(file, "%sloop %s = %s, %s\n\n",
	    indent, loop->var.name, from, to);
	free(from);
	free(to);
}


/* ----- vectors and objects ----------------------------------------------- */


static char *generate_name(const struct vec *base)
{
	const struct vec *walk;
	int n;

	n = 0;
	for (walk = base->frame->vecs; walk != base; walk = walk->next)
		n++;
	return stralloc_printf("_%s_%d",
	    base->frame->name ? base->frame->name : "", n);
}


static char *base_name(const struct vec *base, const struct vec *next)
{
	if (!base)
		return stralloc("@");
	if (next && base->next == next)
		return stralloc(".");
	if (base->name)
		return stralloc(base->name);
	return generate_name(base);
}


static char *obj_base_name(const struct vec *base, const struct vec *prev)
{
	if (base && base == prev)
		return stralloc(".");
	return base_name(base, NULL);
}


static int n_vec_refs(const struct vec *vec)
{
	const struct vec *walk;
	int n;

	n = 0;
	for (walk = vec->frame->vecs; walk; walk = walk->next)
		if (walk->base == vec)
			n++;
	return n;
}


/*
 * "need" operates in two modes:
 *
 * - if "prev" is non-NULL, we're looking for objects that need to be put after
 *   the current vector (in "prev"). Only those objects need to be put there
 *   that have at least one base that isn't the frame's origin or already has a
 *   name.
 *
 * - if "prev" is NULL, we're at the end of the frame. We have already used all
 *   the . references we could, so now we have to find out which objects
 *   haven't been dumped yet. "need" still returns the ones that had a need to
 *   be dumped. Again, that's those that have at least one possible "." base.
 *   Since this "." base will have been used by now, the object must have been
 *   dumped.
 */

static int need(const struct vec *base, const struct vec *prev)
{
	if (!base)
		return 0;
	if (base->name)
		return 0;
	if (prev)
		return base == prev;
	return 1;
}


/*
 * If we need a vector that's defined later, we have to defer dumping the
 * object.
 */

static int later(const struct vec *base, const struct vec *prev)
{
	while (prev) {
		if (base == prev)
			return 1;
		prev = prev->next;
	}
	return 0;
}


static int may_dump_obj_now(const struct obj *obj, const struct vec *prev)
{
	int n, l;

	n = need(obj->base, prev);
	l = later(obj->base, prev);

	switch (obj->type) {
	case ot_frame:
		break;
	case ot_line:
		n |= need(obj->u.line.other, prev);
		l |= later(obj->u.line.other, prev);
		break;
	case ot_rect:
		n |= need(obj->u.rect.other, prev);
		l |= later(obj->u.rect.other, prev);
		break;
	case ot_pad:
		n |= need(obj->u.pad.other, prev);
		l |= later(obj->u.pad.other, prev);
		break;
	case ot_arc:
		n |= need(obj->u.arc.start, prev);
		n |= need(obj->u.arc.end, prev);
		l |= later(obj->u.arc.start, prev);
		l |= later(obj->u.arc.end, prev);
		break;
	case ot_meas:
		n |= need(obj->u.meas.other, prev);
		l |= later(obj->u.meas.other, prev);
		break;
	default:
		abort();
	}

	return n && !l;
}


static void dump_obj(FILE *file, struct obj *obj, const char *indent,
    const struct vec *prev)
{
	char *base, *s1, *s2, *s3;

	if (obj->dumped)
		return;
	obj->dumped = 1;
	base = obj_base_name(obj->base, prev);
	switch (obj->type) {
	case ot_frame:
		fprintf(file, "%sframe %s %s\n",
		    indent, obj->u.frame.ref->name, base);
		break;
	case ot_line:
		s1 = obj_base_name(obj->u.line.other, prev);
		s2 = unparse(obj->u.line.width);
		fprintf(file, "%sline %s %s %s\n", indent, base, s1, s2);
		free(s1);
		free(s2);
		break;
	case ot_rect:
		s1 = obj_base_name(obj->u.rect.other, prev);
		s2 = unparse(obj->u.rect.width);
		fprintf(file, "%srect %s %s %s\n", indent, base, s1, s2);
		free(s1);
		free(s2);
		break;
	case ot_pad:
		s1 = obj_base_name(obj->u.pad.other, prev);
		fprintf(file, "%spad \"%s\" %s %s\n", indent,
		    obj->u.pad.name, base, s1);
		free(s1);
		break;
	case ot_arc:
		s1 = obj_base_name(obj->u.arc.start, prev);
		s3 = unparse(obj->u.arc.width);
		if (obj->u.arc.start == obj->u.arc.end) {
			fprintf(file, "%scirc %s %s %s\n",
			    indent, base, s1, s3);
		} else {
			s2 = obj_base_name(obj->u.arc.end, prev);
			fprintf(file, "%sarc %s %s %s %s\n", indent,
			    base, s1, s2, s3);
			free(s2);
		}
		free(s1);
		free(s3);
		break;
	case ot_meas:
		s1 = obj_base_name(obj->u.meas.other, prev);
		s2 = unparse(obj->u.meas.offset);
		fprintf(file, "%smeas %s %s %s\n", indent, base, s1, s2);
		free(s1);
		free(s2);
		break;
	default:
		abort();
	}
	free(base);
}


/*
 * Tricky logic ahead: when dumping a vector, we search for a vectors that
 * depends on that vector for ".". If we find one, we dump it immediately after
 * this vector.
 */

static void recurse_vec(FILE *file, const struct vec *vec, const char *indent)
{
	const struct vec *next;
	struct obj *obj;
	char *base, *x, *y, *s;

	base = base_name(vec->base, vec);
	x = unparse(vec->x);
	y = unparse(vec->y);
	if (vec->name)
		fprintf(file, "%s%s: vec %s(%s, %s)\n",
		    indent, vec->name, base, x, y);
	else {
		s = generate_name(vec);
		fprintf(file, "%s%s: vec %s(%s, %s)\n", indent, s, base, x, y);
		free(s);
	}
	free(base);
	free(x);
	free(y);

	for (obj = vec->frame->objs; obj; obj = obj->next)
		if (may_dump_obj_now(obj, vec))
			dump_obj(file, obj, indent, vec);
	if (n_vec_refs(vec) == 1) {
		for (next = vec->next; next->base != vec; next = next->next);
		recurse_vec(file, next, indent);
	}
}


static void dump_vecs(FILE *file, const struct vec *vecs, const char *indent)
{
	const struct vec *vec;

	for (vec = vecs; vec; vec = vec->next)
		if (!vec->base || n_vec_refs(vec->base) != 1)
			recurse_vec(file, vec, indent);
}


/* ----- frames ------------------------------------------------------------ */


static void dump_frame(FILE *file, const struct frame *frame,
    const char *indent)
{
	const struct table *table;
	const struct loop *loop;
	struct obj *obj;

	for (table = frame->tables; table; table = table->next)
		dump_table(file, table, indent);
	for (loop = frame->loops; loop; loop = loop->next)
		dump_loop(file, loop, indent);
	for (obj = frame->objs; obj; obj = obj->next)
		obj->dumped = 0;
	dump_vecs(file, frame->vecs, indent);

	/* duh. do we need this ? */
	for (obj = frame->objs; obj; obj = obj->next)
		dump_obj(file, obj, indent, NULL);
}


/* ----- file -------------------------------------------------------------- */


int dump(FILE *file)
{
	const struct frame *frame;

	fprintf(file, "/* MACHINE-GENERATED ! */\n\n");
	for (frame = frames; frame; frame = frame->next) {
		if (!frame->name)
			dump_frame(file, frame, "");
		else {
			fprintf(file, "frame %s {\n", frame->name);
			dump_frame(file, frame, "\t");
			fprintf(file, "}\n\n");
		}
	}
	fflush(file);
	return !ferror(file);
}
