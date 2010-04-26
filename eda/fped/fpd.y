%{
/*
 * fpd.y - FootPrint Definition language
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
#include <string.h>

#include "util.h"
#include "error.h"
#include "expr.h"
#include "obj.h"
#include "meas.h"
#include "gui_status.h"
#include "dump.h"
#include "tsort.h"
#include "fpd.h"


struct expr *expr_result;
const char *var_id;
struct value *var_value_list;


static struct frame *curr_frame;
static struct table *curr_table;
static struct row *curr_row;

static struct frame *last_frame = NULL;
static struct vec *last_vec = NULL;

static struct table **next_table;
static struct loop **next_loop;
static struct vec **next_vec;
static struct obj **next_obj;

static int n_vars, n_values;

static const char *id_sin, *id_cos, *id_sqrt;

static struct tsort *tsort;


static struct frame *find_frame(const char *name)
{
	struct frame *f;

	for (f = frames; f; f = f->next)
		if (f->name == name)
			return f;
	return NULL;
}


static struct vec *find_vec(const struct frame *frame, const char *name)
{
	struct vec *v;

	for (v = frame->vecs; v; v = v->next)
		if (v->name == name)
			return v;
	return NULL;
}


static struct obj *find_obj(const struct frame *frame, const char *name)
{
	struct obj *obj;

	for (obj = frame->objs; obj; obj = obj->next)
		if (obj->name == name)
			return obj;
	return NULL;
}


static int find_label(const struct frame *frame, const char *name)
{
	if (find_vec(frame, name))
		return 1;
	if (find_obj(frame, name))
		return 1;
	return 0;
}


static struct var *find_var(const struct frame *frame, const char *name)
{
	const struct table *table;
	struct var *var;
	struct loop *loop;

	for (table = frame->tables; table; table = table->next)
		for (var = table->vars; var; var = var->next)
			if (var->name == name)
				return var;
	for (loop = frame->loops; loop; loop = loop->next)
		if (loop->var.name == name)
			return &loop->var;
	return NULL;
}


static void set_frame(struct frame *frame)
{
	curr_frame = frame;
	next_table = &frame->tables;
	next_loop = &frame->loops;
	next_vec = &frame->vecs;
	next_obj = &frame->objs;
	last_vec = NULL;
}


static void make_var(const char *id, struct expr *expr)
{
	struct table *table;

	table = zalloc_type(struct table);
	table->vars = zalloc_type(struct var);
	table->vars->name = id;
	table->vars->frame = curr_frame;
	table->vars->table = table;
	table->rows = zalloc_type(struct row);
	table->rows->table = table;
	table->rows->values = zalloc_type(struct value);
	table->rows->values->expr = expr;
	table->rows->values->row = table->rows;
	table->active_row = table->rows;
	*next_table = table;
	next_table = &table->next;
}


static void make_loop(const char *id, struct expr *from, struct expr *to)
{
	struct loop *loop;

	loop = alloc_type(struct loop);
	loop->var.name = id;
	loop->var.next = NULL;
	loop->var.frame = curr_frame;
	loop->var.table = NULL;
	loop->from.expr = from;
	loop->from.row = NULL;
	loop->from.next = NULL;
	loop->to.expr = to;
	loop->to.row = NULL;
	loop->to.next = NULL;
	loop->next = NULL;
	loop->active = 0;
	loop->initialized = 0;
	*next_loop = loop;
	next_loop = &loop->next;
}


static struct obj *new_obj(enum obj_type type)
{
	struct obj *obj;

	obj = alloc_type(struct obj);
	obj->type = type;
	obj->name = NULL;
	obj->frame = curr_frame;
	obj->next = NULL;
	obj->lineno = lineno;
	return obj;
}


static int dbg_delete(const char *name)
{
	struct vec *vec;
	struct obj *obj;

	vec = find_vec(curr_frame, name);
	if (vec) {
		delete_vec(vec);
		return 1;
	}
	obj = find_obj(curr_frame, name);
	if (obj) {
		delete_obj(obj);
		return 1;
	}
	yyerrorf("unknown item \"%s\"", name);
	return 0;
}


static int dbg_move(const char *name, int anchor, const char *dest)
{
	struct vec *to, *vec;
	struct obj *obj;
	struct vec **anchors[3];
	int n_anchors;

	to = find_vec(curr_frame, dest);
	if (!to) {
		yyerrorf("unknown vector \"%s\"", dest);
		return 0;
	}
	vec = find_vec(curr_frame, name);
	if (vec) {
		if (anchor) {
			yyerrorf("invalid anchor (%d > 0)", anchor);
			return 0;
		}
		vec->base = to;
		return 1;
	}
	obj = find_obj(curr_frame, name);
	if (!obj) {
		yyerrorf("unknown item \"%s\"", name);
		return 0;
	}
	n_anchors = obj_anchors(obj, anchors);
	if (anchor >= n_anchors) {
		yyerrorf("invalid anchor (%d > %d)", anchor, n_anchors-1);
		return 0;
	}
	*anchors[anchor] = to;
	return 1;
}


static int dbg_print(const struct expr *expr)
{
	const char *s;
	struct num num;

	s = eval_str(expr, curr_frame);
	if (s) {
		printf("%s\n", s);
		return 1;
	}
	num = eval_num(expr, curr_frame);
	if (is_undef(num))
		return 0;
	printf("%lg%s\n", num.n, str_unit(num));
	return 1;
}


static void append_root_frame(void)
{
	root_frame->prev = last_frame;
	if (last_frame)
		last_frame->next = root_frame;
	else
		frames = root_frame;
}


%}


%union {
	struct num num;
	char *str;
	const char *id;
	struct expr *expr;
	struct frame *frame;
	struct table *table;
	struct var *var;
	struct row *row;
	struct value *value;
	struct vec *vec;
	struct obj *obj;
	enum pad_type pt;
	enum meas_type mt;
	struct {
		int inverted;
		int max;
	} mo;
};


%token		START_FPD START_EXPR START_VAR START_VALUES
%token		TOK_SET TOK_LOOP TOK_PACKAGE TOK_FRAME TOK_TABLE TOK_VEC
%token		TOK_PAD TOK_RPAD TOK_HOLE TOK_RECT TOK_LINE TOK_CIRC TOK_ARC
%token		TOK_MEAS TOK_MEASX TOK_MEASY TOK_UNIT
%token		TOK_NEXT TOK_NEXT_INVERTED TOK_MAX TOK_MAX_INVERTED
%token		TOK_DBG_DEL TOK_DBG_MOVE TOK_DBG_PRINT TOK_DBG_DUMP
%token		TOK_DBG_EXIT TOK_DBG_TSORT

%token	<num>	NUMBER
%token	<str>	STRING
%token	<id>	ID LABEL

%type	<table>	table
%type	<var>	vars var
%type	<row>	rows
%type	<value>	row value opt_value_list
%type	<vec>	vec base qbase
%type	<obj>	object obj meas
%type	<expr>	expr opt_expr add_expr mult_expr unary_expr primary_expr
%type	<num>	opt_num
%type	<str>	opt_string
%type	<pt>	pad_type
%type	<mt>	meas_type
%type	<mo>	meas_op

%%

all:
	START_FPD
		{
			root_frame = zalloc_type(struct frame);
			set_frame(root_frame);
			id_sin = unique("sin");
			id_cos = unique("cos");
			id_sqrt = unique("sqrt");
		}
	    fpd
		{
			append_root_frame();
		}
	| START_EXPR expr
		{
			expr_result = $2;
		}
	| START_VAR ID opt_value_list
		{
			var_id = $2;
			var_value_list = $3;
		}
	| START_VALUES row
		{
			var_value_list = $2;
		}
	;

fpd:
	| frame_defs part_name opt_unit frame_items
	;

part_name:
	TOK_PACKAGE STRING
		{
			const char *p;

			if (!*$2) {
				yyerrorf("invalid package name");
				YYABORT;
			}
			for (p = $2; *p; *p++)
				if (*p < 32 || *p > 126) {
					yyerrorf("invalid package name");
					YYABORT;
				}
			pkg_name = $2;
		}
	;

opt_unit:
	| TOK_UNIT ID
		{
			if (!strcmp($2, "mm"))
				curr_unit = curr_unit_mm;
			else if (!strcmp($2, "mil"))
				curr_unit = curr_unit_mil;
			else if (!strcmp($2, "auto"))
				curr_unit = curr_unit_auto;
			else {
				yyerrorf("unrecognized unit \"%s\"", $2);
				YYABORT;
			}
		}
	;

frame_defs:
	| frame_defs frame_def
	;

frame_def:
	TOK_FRAME ID '{'
		{
			if (find_frame($2)) {
				yyerrorf("duplicate frame \"%s\"", $2);
				YYABORT;
			}
			curr_frame = zalloc_type(struct frame);
			curr_frame->name = $2;
			set_frame(curr_frame);
			curr_frame->prev = last_frame;
			if (last_frame)
				last_frame->next = curr_frame;
			else
				frames = curr_frame;
			last_frame = curr_frame;
		}
	    frame_items '}'
		{
			set_frame(root_frame);
		}
	;

frame_items:
	| measurements
	| frame_item frame_items
	;

frame_item:
	table
	| TOK_SET ID '=' expr
		{
			if (find_var(curr_frame, $2)) {
				yyerrorf("duplicate variable \"%s\"", $2);
				YYABORT;
			}
			make_var($2, $4);
		}
	| TOK_LOOP ID '=' expr ',' expr
		{
			if (find_var(curr_frame, $2)) {
				yyerrorf("duplicate variable \"%s\"", $2);
				YYABORT;
			}
			make_loop($2, $4, $6);
		}
	| vec
	| LABEL vec
		{
			if (find_label(curr_frame, $1)) {
				yyerrorf("duplicate label \"%s\"", $1);
				YYABORT;
			}
			$2->name = $1;
		}
	| object
	| LABEL object
		{
			if (find_label(curr_frame, $1)) {
				yyerrorf("duplicate label \"%s\"", $1);
				YYABORT;
			}
			$2->name = $1;
		}
	| TOK_DBG_DEL ID
		{
			if (!dbg_delete($2))
				YYABORT;
		}
	| TOK_DBG_MOVE ID opt_num ID
		{
			if (!dbg_move($2, $3.n, $4))
				YYABORT;
		}
	| TOK_DBG_PRINT expr
		{
			if (!dbg_print($2))
				YYABORT;
		}
	| debug_item
	;

debug_item:
	TOK_DBG_DUMP
		{
			/*
			 * It's okay to do append the root frame multiple
			 * times. If more frames are added afterwards, they
			 * just replace the root frame until it gets appended a
			 * final time when parsing ends.
			 */
			append_root_frame();
			if (!dump(stdout)) {
				perror("stdout");
				exit(1);
			}
		}
	| TOK_DBG_EXIT
		{
			exit(0);
		}
	| TOK_DBG_TSORT '{'
		{
			tsort = begin_tsort();
		}
	    sort_items '}'
		{
			void **sort, **walk;

			sort = end_tsort(tsort);
			for (walk = sort; *walk; walk++)
				printf("%s\n", (char *) *walk);
			free(sort);
		}
	;

sort_items:
	| sort_items '+' ID
		{
			add_node(tsort, (void *) $3, 0);
		}
	| sort_items '-' ID
		{
			add_node(tsort, (void *) $3, 1);
		}
	| sort_items ID ID opt_num
		{
			struct node *a, *b;

			/* order is important here ! */
			a = add_node(tsort, (void *) $2, 0);
			b = add_node(tsort, (void *) $3, 0);
			add_edge(a, b, $4.n);
		}
	;

table:
	TOK_TABLE
		{
			$<table>$ = zalloc_type(struct table);
			*next_table = $<table>$;
			curr_table = $<table>$;
			n_vars = 0;
		}
	    '{' vars '}' rows
		{
			$$ = $<table>2;
			$$->vars = $4;
			$$->rows = $6;
			$$->active_row = $6;
			next_table = &$$->next;
		}
	;

vars:
	var
		{
			$$ = $1;
		}
	| vars ',' var
		{
			struct var **walk;

			$$ = $1;
			for (walk = &$$; *walk; walk = &(*walk)->next);
			*walk = $3;
		}
	;

var:
	ID
		{
			if (find_var(curr_frame, $1)) {
				yyerrorf("duplicate variable \"%s\"", $1);
				YYABORT;
			}
			$$ = zalloc_type(struct var);
			$$->name = $1;
			$$->frame = curr_frame;
			$$->table = curr_table;
			$$->next = NULL;
			n_vars++;
		}
	;
	
	
rows:
		{
			$$ = NULL;
		}
	| '{'
		{
			$<row>$ = alloc_type(struct row);
			$<row>$->table = curr_table;
			curr_row = $<row>$;;
			n_values = 0;
		}
	    row '}'
		{
			if (n_vars != n_values) {
				yyerrorf("table has %d variables but row has "
				    "%d values", n_vars, n_values);
				YYABORT;
			}
			$<row>2->values = $3;
		}
	    rows
		{
			$$ = $<row>2;
			$$->next = $6;
		}
	;

row:
	value
		{
			$$ = $1;
		}
	| row ',' value
		{
			struct value **walk;

			$$ = $1;
			for (walk = &$$; *walk; walk = &(*walk)->next);
			*walk = $3;
		}
	;

value:
	expr
		{
			$$ = alloc_type(struct value);
			$$->expr = $1;
			$$->row = curr_row;
			$$->next = NULL;
			n_values++;
		}
	;

vec:
	TOK_VEC base '(' expr ',' expr ')'
		{
			$$ = alloc_type(struct vec);
			$$->name = NULL;
			$$->base = $2;
			$$->x = $4;
			$$->y = $6;
			$$->frame = curr_frame;
			$$->next = NULL;
			last_vec = $$;
			*next_vec = $$;
			next_vec = &$$->next;
		}
	;

base:
	'@'
		{
			$$ = NULL;
		}
	| '.'
		{
			$$ = last_vec;
			if (!$$) {
				yyerrorf(". without predecessor");
				YYABORT;
			}
		}
	| ID
		{
			$$ = find_vec(curr_frame, $1);
			if (!$$) {
				yyerrorf("unknown vector \"%s\"", $1);
				YYABORT;
			}
		}
	;

object:
	obj
		{
			$$ = $1;
			*next_obj = $1;
			next_obj = &$1->next;
		}
	;

obj:
	TOK_PAD STRING base base pad_type
		{
			$$ = new_obj(ot_pad);
			$$->base = $3;
			$$->u.pad.name = $2;
			$$->u.pad.other = $4;
			$$->u.pad.rounded = 0;
			$$->u.pad.type = $5;
		}
	| TOK_RPAD STRING base base pad_type
		{
			$$ = new_obj(ot_pad);
			$$->base = $3;
			$$->u.pad.name = $2;
			$$->u.pad.other = $4;
			$$->u.pad.rounded = 1;
			$$->u.pad.type = $5;
		}
	| TOK_HOLE base base
		{
			$$ = new_obj(ot_hole);
			$$->base = $2;
			$$->u.hole.other = $3;
		}
	| TOK_RECT base base opt_expr
		{
			$$ = new_obj(ot_rect);
			$$->base = $2;
			$$->u.rect.other = $3;
			$$->u.rect.width = $4;
		}
	| TOK_LINE base base opt_expr
		{
			$$ = new_obj(ot_line);
			$$->base = $2;
			$$->u.line.other = $3;
			$$->u.line.width = $4;
		}
	| TOK_CIRC base base opt_expr
		{
			$$ = new_obj(ot_arc);
			$$->base = $2;
			$$->u.arc.start = $3;
			$$->u.arc.end = $3;
			$$->u.arc.width = $4;
		}
	| TOK_ARC base base base opt_expr
		{
			$$ = new_obj(ot_arc);
			$$->base = $2;
			$$->u.arc.start = $3;
			$$->u.arc.end = $4;
			$$->u.arc.width = $5;
		}
	| TOK_FRAME ID 
		{ 
		    $<num>$.n = lineno;
		}
		    base
		{
			$$ = new_obj(ot_frame);
			$$->base = $4;
			$$->u.frame.ref = find_frame($2);
			if (!$$->u.frame.ref) {
				yyerrorf("unknown frame \"%s\"", $2);
				YYABORT;
			}
			if (!$$->u.frame.ref->active_ref)
				$$->u.frame.ref->active_ref = $$;
			$$->u.frame.lineno = $<num>3.n;
		}
	;

pad_type:
		{
			$$ = pt_normal;
		}
	| ID
		{
			if (!strcmp($1, "bare"))
				$$ = pt_bare;
			else if (!strcmp($1, "paste"))
				$$ = pt_paste;
			else if (!strcmp($1, "mask"))
				$$ = pt_mask;
			else {
				yyerrorf("unknown pad type \"%s\"", $1);
				YYABORT;
			}
		}
	;

measurements:
	meas
		{
			*next_obj = $1;
			next_obj = &$1->next;
		}
	| measurements meas
		{
			*next_obj = $2;
			next_obj = &$2->next;
		}
	| measurements debug_item
	;

meas:
	meas_type opt_string qbase meas_op qbase opt_expr
		{
			struct meas *meas;

			$$ = new_obj(ot_meas);
			meas = &$$->u.meas;
			meas->type = $4.max ? $1+3 : $1;
			meas->label = $2;
			$$->base = $3;
			meas->inverted = $4.inverted;
			meas->high = $5;
			meas->offset = $6;
			$$->next = NULL;
		}
	;

qbase:
	ID
		{
			$$ = find_vec(root_frame, $1);
			if (!$$) {
				yyerrorf("unknown vector \"%s\"", $1);
				YYABORT;
			}
		}
	| ID '.' ID
		{
			const struct frame *frame;

			frame = find_frame($1);
			$$ = frame ? find_vec(frame, $3) : NULL;
			if (!$$) {
				yyerrorf("unknown vector \"%s.%s\"", $1, $3);
				YYABORT;
			}
		}
	;

meas_type:
	TOK_MEAS
		{
			$$ = mt_xy_next;
		}
	| TOK_MEASX
		{
			$$ = mt_x_next;
		}
	| TOK_MEASY
		{
			$$ = mt_y_next;
		}
	;

meas_op:
	TOK_NEXT
		{
			$$.max = 0;
			$$.inverted = 0;
		}
	| TOK_NEXT_INVERTED
		{
			$$.max = 0;
			$$.inverted = 1;
		}
	| TOK_MAX
		{
			$$.max = 1;
			$$.inverted = 0;
		}
	| TOK_MAX_INVERTED
		{
			$$.max = 1;
			$$.inverted = 1;
		}
	;

opt_num:
		{
			$$.n = 0;
		}
	| NUMBER
		{
			$$ = $1;
		}
	;

opt_string:
		{
			$$ = NULL;
		}
	| STRING
		{
			$$ = $1;
		}
	;

opt_expr:
		{
			$$ = NULL;
		}
	| expr
		{
			$$ = $1;
		}
	;

expr:
	add_expr
		{
			$$ = $1;
		}
	;

add_expr:
	mult_expr
		{
			$$ = $1;
		}
	| add_expr '+' mult_expr
		{
			$$ = binary_op(op_add, $1, $3);
		}
	| add_expr '-' mult_expr
		{
			$$ = binary_op(op_sub, $1, $3);
		}
	;

mult_expr:
	unary_expr
		{
			$$ = $1;
		}
	| mult_expr '*' unary_expr
		{
			$$ = binary_op(op_mult, $1, $3);
		}
	| mult_expr '/' unary_expr
		{
			$$ = binary_op(op_div, $1, $3);
		}
	;

unary_expr:
	primary_expr
		{
			$$ = $1;
		}
	| '-' primary_expr
		{
			$$ = binary_op(op_minus, $2, NULL);
		}
	;

primary_expr:
	NUMBER
		{
			$$ = new_op(op_num);
			$$->u.num = $1;
		}
	| ID
		{
			$$ = new_op(op_var);
			$$->u.var = $1;
		}
	| STRING
		{
			$$ = new_op(op_string);
			$$->u.str = $1;
		}
	| '(' expr ')'
		{
			$$ = $2;
		}
	| ID '(' expr ')'
		{
			if ($1 == id_sin)
				$$ = binary_op(op_sin, $3, NULL);
			else if ($1 == id_cos)
				$$ = binary_op(op_cos, $3, NULL);
			else if ($1 == id_sqrt)
				$$ = binary_op(op_sqrt, $3, NULL);
			else {
				yyerrorf("unknown function \"%s\"", $1);
				YYABORT;
			}
		}
	;

/* special sub-grammar */

opt_value_list:
		{
			$$ = NULL;
		}
	| '=' row
		{
			$$ = $2;
		}
	;
