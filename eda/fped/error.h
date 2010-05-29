/*
 * error.h - Error reporting
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef ERROR_H
#define ERROR_H


extern int lineno;

extern void (*reporter)(const char *s);


void yywarn(const char *s);

void yyerrorf(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
void yyerror(const char *s);

void report_to_stderr(const char *s);
void report_parse_error(const char *s);
void fail(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));

#endif /* !ERROR_H */
