/*
 * unparse.h - Dump an expression tree into a string
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef UNPARSE_H
#define UNPARSE_H

#include "expr.h"


char *unparse(const struct expr *expr);

#endif /* !UNPARSE_H */
