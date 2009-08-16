/*
 * fpd.c - Things fpd.l and fpd.y export
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef FPD_H
#define FPD_H

#include "expr.h"
#include "obj.h"


extern struct expr *expr_result;
extern const char *var_id;
extern struct value *var_value_list;


void scan_empty(void);
void scan_expr(const char *s);
void scan_var(const char *s);

int yyparse(void);

#endif /* !FPD_H */
