/*
 * var.c - Environment variable repository
 *
 * Copyright (C) 2008 by OpenMoko, Inc.
 * Written by Werner Almesberger <werner@openmoko.org>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "var.h"


static struct var {
	const char *name;	/* NULL if deleted */
	const char *value;
} *vars = NULL, *curr_var;

static int n_vars = 0;


void set_var(const char *name, const char *value)
{
	struct var *p, *unused = NULL;

	for (p = vars; p != vars+n_vars; p++) {
		if (!p->name)
			unused = p;
		else {
			if (!strcmp(p->name, name))
				break;
		}
	}
	if (p == vars+n_vars && unused)
		p = unused;
	if (p == vars+n_vars) {
		vars = realloc(vars, sizeof(struct var)*2*(n_vars+1));
		if (!vars) {
			perror("realloc");
			exit(1);
		}
		memset(vars+n_vars, 0, sizeof(struct var)*(n_vars+2));
		p = vars+n_vars;
		n_vars = 2*(n_vars+1);
	}
	if (!p->name) {
		p->name = strdup(name);
		if (!p->name) {
			perror("strdup");
			exit(1);
		}
	}
	if (p->value)
		free((void *) p->value);
	p->value = strdup(value);
	if (!p->value) {
		perror("strdup");
		exit(1);
	}
}


void del_var(const char *name)
{
	struct var *p;

	for (p = vars; p != vars+n_vars; p++)
		if (p->name && !strcmp(p->name, name))
			break;
	if (p == vars+n_vars)
		return;
	free((void *) p->name);
	free((void *) p->value);
	p->name = NULL;
	p->value = NULL;
}


static int comp(const void *_a, const void *_b)
{
	const struct var *a = _a;
	const struct var *b = _b;

	return a->name ? b->name ? strcmp(a->name, b->name) :
	    1 : b->name ? -1 : 0;
}


void reset_var(void)
{
	if (!vars)
		return;
	qsort(vars, n_vars, sizeof(struct var), comp);
	curr_var = vars;
}


const char *next_var(const char **value)
{
	if (!vars)
		return NULL;
	while (curr_var != vars+n_vars && !curr_var->name)
		curr_var++;
	if (curr_var == vars+n_vars)
		return NULL;
	*value = curr_var->value;
	return (curr_var++)->name;
}
