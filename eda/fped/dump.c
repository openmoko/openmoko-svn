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
#include "gui_status.h"
#include "dump.h"


/* ----- order items ------------------------------------------------------- */


static void add_item(struct order **curr, struct vec *vec, struct obj *obj)
{
	(*curr)->vec = vec;
	(*curr)->obj = obj;
	(*curr)++;
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
 * If "prev" is non-NULL, we're looking for objects that need to be put after
 * the current vector (in "prev"). Only those objects need to be put there
 * that have at least one base that isn't the frame's origin.
 *
 * We could also make an exception for manually named vectors, but we get
 * better clustering without.
 */

static int need(const struct vec *base, const struct vec *prev)
{
	if (!base)
		return 0;
#if 0
	if (base->name && *base->name != '_')
		return 0;
#endif
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
	while (1) {
		prev = prev->next;
		if (!prev)
			break;
		if (base == prev)
			return 1;
	}
	return 0;
}


static int may_put_obj_now(const struct obj *obj, const struct vec *prev)
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
		return 0;
	default:
		abort();
	}

	return n && !l;
}


static void put_obj(struct order **curr, struct obj *obj,
    struct vec *prev)
{
	if (obj->dumped)
		return;
	obj->dumped = 1;
	add_item(curr, prev, obj);
}

/*
 * Tricky logic ahead: when dumping a vector, we search for a vectors that
 * depends on that vector for ".". If we find one, we dump it immediately after
 * this vector.
 */

static void recurse_vec(struct order **curr, struct vec *vec)
{
	struct vec *next;
	struct obj *obj;

	add_item(curr, vec, NULL);
	for (obj = vec->frame->objs; obj; obj = obj->next)
		if (may_put_obj_now(obj, vec))
			put_obj(curr, obj, vec);
	if (n_vec_refs(vec) == 1) {
		for (next = vec->next; next->base != vec; next = next->next);
		recurse_vec(curr, next);
	}
}


static void order_vecs(struct order **curr, struct vec *vecs)
{
	struct vec *vec;

	for (vec = vecs; vec; vec = vec->next)
		if (!vec->base || n_vec_refs(vec->base) != 1)
			recurse_vec(curr, vec);
}


struct order *order_frame(const struct frame *frame)
{
	struct order *order, *curr;
	struct vec *vec;
	struct obj *obj;
	int n = 0;

	for (vec = frame->vecs; vec; vec = vec->next)
		n++;
	for (obj = frame->objs; obj; obj = obj->next)
		if (obj->type != ot_meas)
			n++;

	for (obj = frame->objs; obj; obj = obj->next)
		obj->dumped = 0;

	order = alloc_size(sizeof(*order)*(n+1));
	curr = order;

	order_vecs(&curr, frame->vecs);

	/* frames based on @ (anything else ?) */
	for (obj = frame->objs; obj; obj = obj->next)
		if (obj->type != ot_meas)
			put_obj(&curr, obj, NULL);

	assert(curr == order+n);
	add_item(&curr, NULL, NULL);

	return order;
}


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


static void generate_name(struct vec *base)
{
	char tmp[10]; /* plenty */
	const char *s;
	struct vec *walk;
	int n = 0;

	while (1) {
		sprintf(tmp, "__%d", n);
		s = unique(tmp);
		for (walk = base->frame->vecs; walk; walk = walk->next)
			if (walk->name == s)
				break;
		if (!walk)
			break;
		n++;
	}
	base->name = s;
}


static const char *base_name(struct vec *base, const struct vec *next)
{
	if (!base)
		return "@";
	if (next && base->next == next)
		return ".";
	if (!base->name)
		generate_name(base);
	return base->name;
}


static const char *obj_base_name(struct vec *base, const struct vec *prev)
{
	if (base && base == prev)
		return ".";
	return base_name(base, NULL);
}


char *print_obj(const struct obj *obj, const struct vec *prev)
{
	const char *base, *s1, *s3;
	char *s, *s2;

	base = obj_base_name(obj->base, prev);
	switch (obj->type) {
	case ot_frame:
		s = stralloc_printf("frame %s %s",
		    obj->u.frame.ref->name, base);
		break;
	case ot_line:
		s1 = obj_base_name(obj->u.line.other, prev);
		s2 = unparse(obj->u.line.width);
		s = stralloc_printf("line %s %s %s", base, s1, s2);
		free(s2);
		break;
	case ot_rect:
		s1 = obj_base_name(obj->u.rect.other, prev);
		s2 = unparse(obj->u.rect.width);
		s = stralloc_printf("rect %s %s %s", base, s1, s2);
		free(s2);
		break;
	case ot_pad:
		s1 = obj_base_name(obj->u.pad.other, prev);
		switch (obj->u.pad.type) {
		case pt_normal:
			s2 = "";
			break;
		case pt_bare:
			s2 = " bare";
			break;
		case pt_paste:
			s2 = " paste";
			break;
		case pt_mask:
			s2 = " mask";
			break;
		default:
			abort();
		}
		s = stralloc_printf("%spad \"%s\" %s %s%s",
		    obj->u.pad.rounded ? "r" : "",
		    obj->u.pad.name, base, s1, s2);
		break;
	case ot_arc:
		s1 = obj_base_name(obj->u.arc.start, prev);
		s2 = unparse(obj->u.arc.width);
		if (obj->u.arc.start == obj->u.arc.end) {
			s = stralloc_printf("circ %s %s %s", base, s1, s2);
		} else {
			s3 = obj_base_name(obj->u.arc.end, prev);
			s = stralloc_printf("arc %s %s %s %s",
			    base, s1, s3, s2);
		}
		free(s2);
		break;
	default:
		abort();
	}
	return s;
}


/* ----- print measurement ------------------------------------------------- */


static const char *meas_type_name[mt_n] = {
	"meas", "measx", "measy",
	"meas", "measx", "measy",
};



static char *print_meas_base(struct vec *base)
{
	const char *name;

	name = base_name(base, NULL);
	if (base->frame == root_frame)
		return stralloc(name);
	return stralloc_printf("%s.%s", base->frame->name, name);
}


char *print_meas(const struct obj *obj)
{
	char *s, *t;
	char *s1, *s2, *s3;

	assert(obj->type == ot_meas);

	s = stralloc_printf("%s ", meas_type_name[obj->u.meas.type]);
	if (obj->u.meas.label) {
		t = stralloc_printf("%s\"%s\" ", s, obj->u.meas.label);
		free(s);
		s = t;
	}
	s1 = print_meas_base(obj->base);
	s2 = stralloc_printf(" %s ",
		    obj->u.meas.type < 3 ? obj->u.meas.inverted ? "<-" : "->" :
		    obj->u.meas.inverted ? "<<" : ">>");
	s3 = print_meas_base(obj->u.meas.high);
	t = stralloc_printf("%s%s%s%s", s, s1, s2, s3);
	free(s);
	free(s1);
	free(s2);
	free(s3);
	s = t;

	if (!obj->u.meas.offset)
		return s;

	s1 = unparse(obj->u.meas.offset);
	t = stralloc_printf("%s %s", s, s1);
	free(s);
	free(s1);
	return t;
}


/* ----- print vector ------------------------------------------------------ */


const char *print_label(struct vec *vec)
{
	if (!vec->name)
		generate_name(vec);
	return vec->name;
}


char *print_vec(const struct vec *vec)
{
	const char *base;
	char *x, *y, *s;

	base = base_name(vec->base, vec);
	x = unparse(vec->x);
	y = unparse(vec->y);
	if (vec->name)
		s = stralloc_printf("vec %s(%s, %s)", base, x, y);
	else {
		s = stralloc_printf("vec %s(%s, %s)", base, x, y);
	}
	free(x);
	free(y);
	return s;
}


/* ----- frames ------------------------------------------------------------ */


static void dump_frame(FILE *file, struct frame *frame, const char *indent)
{
	const struct table *table;
	const struct loop *loop;
	struct obj *obj;
	struct order *order;
	const struct order *item;
	char *s;
	const char *s1;

	if (frame->dumped)
		return;
	frame->dumped = 1;

	for (obj = frame->objs; obj; obj = obj->next)
		if (obj->type == ot_frame)
			dump_frame(file, obj->u.frame.ref, "\t");

	if (frame->name)
		fprintf(file, "frame %s {\n", frame->name);

	for (table = frame->tables; table; table = table->next)
		dump_table(file, table, indent);
	for (loop = frame->loops; loop; loop = loop->next)
		dump_loop(file, loop, indent);

	order = order_frame(frame);
	for (item = order; item->vec || item->obj; item++) {
		if (item->obj) {
			s = print_obj(item->obj, item->vec);
			fprintf(file, "%s%s\n", indent, s);
		} else {
			s1 = print_label(item->vec);
			s = print_vec(item->vec);
			fprintf(file, "%s%s: %s\n", indent, s1, s);
		}
		free(s);
	}
	free(order);

	for (obj = frame->objs; obj; obj = obj->next) {
		if (obj->dumped)
			continue;
		s = print_meas(obj);
		fprintf(file, "%s%s\n", indent, s);
		free(s);
	}

	if (frame->name)
		fprintf(file, "}\n\n");
}


/* ----- file -------------------------------------------------------------- */


static void dump_unit(FILE *file)
{
	switch (curr_unit) {
	case curr_unit_mm:
		fprintf(file, "unit mm\n");
		break;
	case curr_unit_mil:
		fprintf(file, "unit mil\n");
		break;
	case curr_unit_auto:
		fprintf(file, "unit auto\n");
		break;
	default:
		abort();
	}
}


int dump(FILE *file)
{
	struct frame *frame;

	fprintf(file, "/* MACHINE-GENERATED ! */\n\n");
	for (frame = frames; frame; frame = frame->next)
		frame->dumped = 0;
	for (frame = frames; frame; frame = frame->next) {
		if (!frame->name) {
			fprintf(file, "package \"%s\"\n", pkg_name);
			dump_unit(file);
			dump_frame(file, frame, "");
		} else {
			dump_frame(file, frame, "\t");
		}
	}
	fflush(file);
	return !ferror(file);
}
