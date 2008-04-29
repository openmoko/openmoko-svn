/*
 * parse.c - Environment edits parser
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
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "var.h"
#include "parse.h"


static enum state {
	st_0,		/* waiting for variable */
	st_var,		/* in variable name */
	st_spc_var,	/* skipping whitespace after variable name */
	st_val,		/* in value */
	st_spc_val,	/* skipping whitespace in value */
	st_spc_val_add,	/* add whitespace to value, then skip */
} state = st_0;


static char *var = NULL, *value = NULL;
static int line = 1;


static void syntax(void)
{
	fprintf(stderr, "syntax error in line %d\n", line);
	exit(1);
}


static void trim(char *s)
{
	char *p;

	for (p = strchr(s, 0)-1; p >= s && isspace(*p); p--)
		*p = 0;
}


static void flush(void)
{
	assert(var);
	if (!value)
		del_var(var);
	else {
		trim(value);
		set_var(var, value);
	}
	free(var);
	if (value)
		free(value);
	var = value = NULL;
}


/*
 * A bit on the inefficient side, but who really cares ?
 */

static void add_char(char **base, char c)
{
	int len;

	len = *base ? strlen(*base) : 0;
	*base = realloc(*base, len+2);
	(*base)[len] = c;
	(*base)[len+1] = 0;
}


static void add_to_var(char c)
{
	add_char(&var, c);
}


static void add_to_value(char c)
{
	add_char(&value, c);
}


static int alpha(char c)
{
	return c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}


static int alnum(char c)
{
	return isdigit(c) ? 1 : alpha(c);
}


static void in_line(int c)
{
	switch (state) {
	case st_var:
		if (isspace(c)) {
			state = st_spc_var;
			break;
		}
		if (c == '=') {
			state = st_spc_val;
			break;
		}
		if (!alnum(c))
			syntax();
		add_to_var(c);
		break;
	case st_spc_var:
		if (isspace(c))
			break;
		if (c == '=') {
			state = st_spc_val;
			break;
		}
		syntax();
		break;
	case st_spc_val_add:
		if (isspace(c)) {
			add_to_value(' ');
			state = st_spc_val;
		}
		/* fall through */
	case st_spc_val:
		if (isspace(c))
			break;
		add_to_value(c);
		state = st_val;
		break;
	case st_val:
		if (!isspace(c))
			add_to_value(c);
		else
			add_to_value(' ');
		break;
	default:
		abort();
	}
}


static void newline(int c)
{
	switch (state) {
	flush:
		flush();
		state = st_0;
		/* fall through */
	case st_0:
		if (isspace(c))
			syntax();
		if (!alpha(c))
			syntax();
		add_to_var(c);
		state = st_var;
		break;
	case st_var:
		if (!isspace(c))
			syntax();
		state = st_spc_var;
		/* fall through */
	case st_spc_var:
		in_line(c);
		break;
	case st_spc_val_add:
		/* fall through */
	case st_val:
		trim(value);
		add_to_value(' ');
		state = st_spc_val;
		/* fall through */
	case st_spc_val:
		if (!isspace(c))
			goto flush;
		break;
	default:
		abort();
	}
}


static void double_newline(void)
{
	switch (state) {
	case st_0:
		break;
	case st_var:
	case st_spc_var:
		syntax();
		break;
	case st_val:
	case st_spc_val:
		flush();
		state = st_0;
		break;
	default:
		abort();
	}
}


void parse_edit(FILE *file)
{
	int comment = 0, nl = 1;

	while (1) {
		int c;

		c = fgetc(file);
//fprintf(stderr, "[%d -> '%c']\n",state,c);
		if (c == EOF)
			break;
		if (c == '#')
			comment = 1;
		if (c == '\n')
			line++;
		if (comment) {
			if (c != '\n')
				continue;
			comment = 0;
			nl = 0;
		}
		if (c == '\n') {
			if (nl)
				double_newline();
			nl = 1;
			continue;
		}
		if (nl)
			newline(c);
		else
			in_line(c);
		nl = 0;
	}

	switch (state) {
	case st_0:
		break;
	case st_var:
	case st_spc_var:
		syntax();
		break;
	case st_val:
	case st_spc_val:
		flush();
		break;
	default:
		abort();
	}
}
