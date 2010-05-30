/*
 * util.h - Common utility functions
 *
 * Written 2006, 2009, 2010 by Werner Almesberger
 * Copyright 2006, 2009, 2010 Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


#define alloc_size(s)					\
    ({	void *alloc_size_tmp = malloc(s);		\
	if (!alloc_size_tmp)				\
		abort();				\
	alloc_size_tmp; })

#define alloc_type(t) ((t *) alloc_size(sizeof(t)))

#define	zalloc_size(s)					\
    ({	void *zalloc_size_tmp = alloc_size(s);		\
	memset(zalloc_size_tmp, 0, (s));		\
	zalloc_size_tmp; })

#define zalloc_type(t)					\
    ({	t *zalloc_type_tmp = alloc_type(t);		\
	memset(zalloc_type_tmp, 0, sizeof(t));		\
	zalloc_type_tmp; })

#define stralloc(s)					\
    ({	char *stralloc_tmp = strdup(s);			\
	if (!stralloc_tmp)				\
		abort();				\
	stralloc_tmp; })

#define strnalloc(s, n)					\
    ({	char *strnalloc_tmp = alloc_size((n)+1);	\
	if (!strnalloc_tmp)				\
		abort();				\
	strncpy(strnalloc_tmp, (s), (n));		\
	strnalloc_tmp[n] = 0;				\
	strnalloc_tmp; })

#define swap(a, b) \
    ({	typeof(a) swap_tmp = (a);			\
	(a) = (b);					\
	(b) = swap_tmp; })


char *stralloc_vprintf(const char *fmt, va_list ap);
char *stralloc_printf(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));

int is_id_char(char c, int first);
int is_id(const char *s);

const char *unique(const char *s);
void unique_cleanup(void);

#endif /* !UTIL_H */
