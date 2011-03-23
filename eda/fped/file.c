/*
 * file.c - File handling
 *
 * Written 2009-2011 by Werner Almesberger
 * Copyright 2009-2011 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "dump.h"
#include "kicad.h"
#include "postscript.h"
#include "util.h"
#include "file.h"
#include "fped.h"


/* ----- general helper functions ------------------------------------------ */


char *set_extension(const char *name, const char *ext)
{
	char *s = stralloc(name);
	char *slash, *dot;
	char *res;

	slash = strrchr(s, '/');
	dot = strrchr(slash ? slash : s, '.');
	if (dot)
		*dot = 0;
	res = stralloc_printf("%s.%s", s, ext);
	free(s);
	return res;
}


int file_exists(const char *name)
{
	struct stat st;

	if (stat(name, &st) >= 0)
		return 1;
	if (errno == ENOENT)
		return 0;
	perror(name);
	return -1;
}


int save_to(const char *name, int (*fn)(FILE *file, const char *one),
    const char *one)
{
	FILE *file;

	file = fopen(name, "w");
	if (!file) {
		perror(name);
		return 0;
	}
	if (!fn(file, one)) {
		perror(name);
		return 0;
	}
	if (fclose(file) == EOF) {
		perror(name);
		return 0;
	}
	return 1;
}


void save_with_backup(const char *name, int (*fn)(FILE *file, const char *one),
    const char *one)
{
	char *s = stralloc(name);
	char *back, *tmp;
	char *slash, *dot;
	int n, res;

	/* save to temporary file */

	slash = strrchr(s, '/');
	if (!slash)
		tmp = stralloc_printf("~%s", s);
	else {
		*slash = 0;
		tmp = stralloc_printf("%s/~%s", s, slash+1);
		*slash = '/';
	}

	if (!save_to(tmp, fn, one))
		return;

	/* move existing file out of harm's way */

	dot = strrchr(slash ? slash : s, '.');
	if (dot)
		*dot = 0;
	n = 0;
	while (1) {
		back = stralloc_printf("%s~%d%s%s",
		    s, n, dot ? "." : "", dot ? dot+1 : "");
		res = file_exists(back);
		if (!res)
			break;
		free(back);
		if (res < 0)
			return;
		n++;
	}
	if (rename(name, back) < 0) {
		if (errno != ENOENT) {
			perror(name);
			free(back);
			return;
		}
	} else {
		fprintf(stderr, "renamed %s to %s\n", name, back);
	}
	free(back);

	/* rename to final name */

	if (rename(tmp, name) < 0) {
		perror(name);
		free(tmp);
		return;
	}
	free(tmp);

	fprintf(stderr, "saved to %s\n", name);
}


/* ----- application-specific save handlers -------------------------------- */


void save_fpd(void)
{
	if (save_file_name)
		save_with_backup(save_file_name, dump, NULL);
	else {
		if (!dump(stdout, NULL))
			perror("stdout");
	}
}


void write_kicad(void)
{
	char *name;

	if (save_file_name) {
		name = set_extension(save_file_name, "mod");
		save_to(name, kicad, NULL);
		free(name);
	} else {
		if (!kicad(stdout, NULL))
			perror("stdout");
	}
}


static void do_write_ps(int (*fn)(FILE *file, const char *one),
    const char *one)
{
	char *name;

	if (save_file_name) {
		name = set_extension(save_file_name, "ps");
		save_to(name, fn, one);
		free(name);
	} else {
		if (!fn(stdout, one))
			perror("stdout");
	}
}


void write_ps(const char *one)
{
	do_write_ps(postscript, one);
}


void write_ps_fullpage(const char *one)
{
	do_write_ps(postscript_fullpage, one);
}
