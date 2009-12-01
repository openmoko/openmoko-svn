/*
 * expr.h - Expressions and values
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef EXPR_H
#define EXPR_H

#include <math.h>


#define UNDEF HUGE_VAL


struct frame;
struct expr;
struct value;

enum num_type {
	nt_none,
	nt_mm,
	nt_mil,
};

struct num {
	enum num_type type;
	int exponent;
	double n;
};

typedef struct num (*op_type)(const struct expr *self,
    const struct frame *frame);

struct expr {
	op_type op;
	union {
		struct num num;
		const char *var;
		char *str;
		struct {
			struct expr *a;
			struct expr *b;
		} op;
	} u;
	int lineno;
};


extern struct num undef;


#define	is_undef(num)		((num).type == nt_none)
#define	is_dimensionless(num)	(!(num).exponent)


static inline int is_distance(struct num num)
{
	return (num.type == nt_mm || num.type == nt_mil) && num.exponent == 1;
}


void fail_expr(const struct expr *expr);

const char *str_unit(struct num n);


static inline struct num make_num(double n)
{
	struct num res;

	res.type = nt_mm;
	res.exponent = 0;
	res.n = n;
	return res;
}


static inline struct num make_mm(double mm)
{
	struct num res;

	res.type = nt_mm;
	res.exponent = 1;
	res.n = mm;
	return res;
}


static inline struct num make_mil(double mil)
{
	struct num res;

	res.type = nt_mil;
	res.exponent = 1;
	res.n = mil;
	return res;
}


int to_unit(struct num *n);

struct num op_num(const struct expr *self, const struct frame *frame);
struct num op_var(const struct expr *self, const struct frame *frame);
struct num op_string(const struct expr *self, const struct frame *frame);

struct num op_sin(const struct expr *self, const struct frame *frame);
struct num op_cos(const struct expr *self, const struct frame *frame);
struct num op_sqrt(const struct expr *self, const struct frame *frame);

struct num op_minus(const struct expr *self, const struct frame *frame);

struct num op_add(const struct expr *self, const struct frame *frame);
struct num op_sub(const struct expr *self, const struct frame *frame);
struct num op_mult(const struct expr *self, const struct frame *frame);
struct num op_div(const struct expr *self, const struct frame *frame);

struct expr *new_op(op_type op);
struct expr *binary_op(op_type op, struct expr *a, struct expr *b);

struct num eval_var(const struct frame *frame, const char *name);

/*
 * eval_str returns NULL if the result isn't a string. Evaluation may then
 * be attempted with eval_num, and the result can be converted accordingly.
 */
const char *eval_str(const struct expr *expr, const struct frame *frame);

struct num eval_num(const struct expr *expr, const struct frame *frame);

/* if frame == NULL, we only check the syntax without expanding */
char *expand(const char *name, const struct frame *frame);

struct expr *new_num(struct num num);
struct expr *parse_expr(const char *s);
void free_expr(struct expr *expr);

int parse_var(const char *s, const char **id, struct value **values,
    int max_values);
int parse_values(const char *s, struct value **values);
void free_values(struct value *values, int keep_expr);

#endif /* !EXPR_H */
