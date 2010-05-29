/*
 * error.c - Error reporting
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "error.h"


extern char *yytext;

int lineno = 1;
void (*reporter)(const char *s) = report_to_stderr;


void yywarn(const char *s)
{
	/* we use yywarn only when starting */
	fprintf(stderr, "%d: warning: %s near \"%s\"\n", lineno, s, yytext);
}


void yyerrorf(const char *fmt, ...)
{
	va_list ap;
	char *buf;
	int n;

	va_start(ap, fmt);
	n = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);
	buf = alloc_size(n+1);
	va_start(ap, fmt);
	vsnprintf(buf, n+1, fmt, ap);
	va_end(ap);
	fail("%s", buf);
	free(buf);
}


void yyerror(const char *s)
{
	yyerrorf("%s", s);
}


void report_parse_error(const char *s)
{
	fprintf(stderr, "%d: %s near \"%s\"\n", lineno, s, yytext);
	exit(1);
}


void report_to_stderr(const char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}


void fail(const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	s = stralloc_vprintf(fmt, ap);
	va_end(ap);
	reporter(s);
	free(s);
}
