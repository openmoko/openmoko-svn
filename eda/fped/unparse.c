/*
 * unparse.c - Dump an expression tree into a string
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * This is crazily inefficient but who cares :-)
 */

#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "expr.h"
#include "unparse.h"


enum prec {
	prec_add,
	prec_mult,
	prec_unary,
	prec_primary,
};


static int precedence(op_type op)
{
	if (op == op_add || op == op_sub)
		return prec_add;
	if (op == op_mult || op == op_div)
		return prec_mult;
	if (op == op_minus)
		return prec_unary;
	if (op == op_num || op == op_string || op == op_var ||
	    op == op_sin || op == op_cos || op == op_sqrt)
		return prec_primary;
	abort();
}


static char *merge3(char *a, const char *op, char *b)
{
	char *buf;

	buf = alloc_size(strlen(op)+strlen(a)+strlen(b)+1);
	sprintf(buf, "%s%s%s", a, op, b);
	free(a);
	free(b);
	return buf;
}


static char *merge2(const char *op, char *a)
{
	char *buf;

	buf = alloc_size(strlen(op)+strlen(a)+1);
	sprintf(buf, "%s%s", op, a);
	free(a);
	return buf;
}


static char *unparse_op(const struct expr *expr, enum prec prec);


static char *unparse_fn(const char *name, const struct expr *expr)
{
	char *buf, *tmp;

	tmp = unparse_op(expr->u.op.a, prec_add);
	buf = alloc_size(strlen(name)+strlen(tmp)+3);
	sprintf(buf, "%s(%s)", name, tmp);
	free(tmp);
	return buf;
}


static char *unparse_op(const struct expr *expr, enum prec prec)
{
	char tmp[100];
	char *buf, *temp;

	
	if (prec > precedence(expr->op)) {
		temp = unparse_op(expr, prec_add);
		buf = alloc_size(strlen(temp)+3);
		sprintf(buf, "(%s)", temp);
		free(temp);
		return buf;
	}
	if (expr->op == op_num) {
		snprintf(tmp, sizeof(tmp), "%lg%s",
		    expr->u.num.n, str_unit(expr->u.num));
		return stralloc(tmp);
	}
	if (expr->op == op_string)
		return stralloc_printf("\"%s\"", expr->u.str);
	if (expr->op == op_var)
		return stralloc(expr->u.var);
	if (expr->op == op_minus)
		return merge2("-", unparse_op(expr->u.op.a, prec_unary));
	if (expr->op == op_add)
		return merge3(unparse_op(expr->u.op.a, prec_add), "+",
		    unparse_op(expr->u.op.b, prec_add));
	if (expr->op == op_sub)
		return merge3(unparse_op(expr->u.op.a, prec_add), "-",
		    unparse_op(expr->u.op.b, prec_mult));
	if (expr->op == op_mult)
		return merge3(unparse_op(expr->u.op.a, prec_mult), "*",
		    unparse_op(expr->u.op.b, prec_mult));
	if (expr->op == op_div)
		return merge3(unparse_op(expr->u.op.a, prec_mult), "/",
		    unparse_op(expr->u.op.b, prec_primary));
	if (expr->op == op_sin)
		return unparse_fn("sin", expr);
	if (expr->op == op_cos)
		return unparse_fn("cos", expr);
	if (expr->op == op_sqrt)
		return unparse_fn("sqrt", expr);
	abort();
}


char *unparse(const struct expr *expr)
{
	return expr ? unparse_op(expr, prec_add) : stralloc("");
}
