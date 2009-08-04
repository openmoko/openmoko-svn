/*
 * util.c - Common utility functions
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "util.h"



/* ----- printf buffer allocation ------------------------------------------ */


char *stralloc_vprintf(const char *fmt, va_list ap)
{
	va_list aq;
	char *buf;
	int n;

	va_copy(aq, ap);
	n = vsnprintf(NULL, 0, fmt, aq);
	va_end(aq);
	buf = alloc_size(n+1);
	vsnprintf(buf, n+1, fmt, ap);
	return buf;
}


char *stralloc_printf(const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	s = stralloc_vprintf(fmt, ap);
	va_end(ap);
	return s;
}


/* ----- identifier syntax check ------------------------------------------- */


int is_id_char(char c, int first) 
{
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
		return 1;
	if (first)
		return 0;
	return c >= '0' && c <= '9';
}


int is_id(const char *s)
{
	const char *p;

	if (!*s)
		return 0;
	for (p = s; *p; p++)
		if (!is_id_char(*p, s == p))
			return 0;
	return 1;
}


/* ----- unique identifiers ------------------------------------------------ */


static struct unique {
	const char *s;
	struct unique *next;
} *uniques = NULL;


/* @@@ consider using rb trees */

const char *unique(const char *s)
{
	struct unique **walk;

	for (walk = &uniques; *walk; walk = &(*walk)->next)
		if (!strcmp(s, (*walk)->s))
			return (*walk)->s;
	*walk = alloc_type(struct unique);
	(*walk)->s = stralloc(s);
	(*walk)->next = NULL;
	return (*walk)->s;
}
