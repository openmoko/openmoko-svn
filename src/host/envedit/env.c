/*
 * env.c - Environment access and file handling
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


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <zlib.h>

#include "var.h"
#include "cpp.h"
#include "parse.h"
#include "env.h"


#define SCRATCH_SIZE	1024	/* buffer for reading in extra environment
				   data. Any value > 0 is fine. */


unsigned long env_size = 16384;

static char *env;


static uint32_t crc_env(void)
{
	return crc32(crc32(0, NULL, 0), (void *) env+4, env_size-4);
}


static FILE *file_open(const char *name, const char *mode, FILE *default_file)
{
	FILE *file;

	if (!strcmp(name, "-"))
		return default_file;
	file = fopen(name, mode);
	if (file)
		return file;
	perror(name);
	exit(1);
}


static void file_close(const char *name, FILE *file, FILE *default_file)
{
	if (file == default_file)
		return;
	if (fclose(file) != EOF)
		return;
	perror(name);
	exit(1);
}


void read_env(const char *name, int warn_crc)
{
	FILE *file;
	size_t got;
	uint32_t orig, crc;

	env = malloc(env_size+1);
	if (!env) {
		perror("malloc");
		exit(1);
	}
	env[env_size] = 0; /* make sure parse_env stops */

	file = file_open(name, "r", stdin);
	got = fread(env, 1, env_size, file);

	/* too small for CRC plus \0\0 */
	if (got < 6) {
		fprintf(stderr, "environment is too small (%lu bytes < 6)\n",
		    (unsigned long) got);
		exit(1);
	}

	if (got < env_size)
		memset(env+got, 0, env_size-got);
	else {
		char scratch[SCRATCH_SIZE];
		size_t more;
		
		do {
			more = fread(scratch, 1, SCRATCH_SIZE, file);
			got += more;
		}
		while (more);
	}
	if (got != env_size)
		fprintf(stderr,
		    "warning: environment is %lu bytes, expected %lu\n",
		    (unsigned long) got, env_size);

	orig = (uint8_t) env[0] | ((uint8_t) env[1] << 8) |
	    ((uint8_t) env[2] << 16) | ((uint8_t) env[3] << 24);
	crc = crc_env();
	if (crc != orig) {
		fprintf(stderr,
		    "CRC error: file says 0x%08lx, calculated 0x%08lx\n",
		    (unsigned long) orig, (unsigned long) crc);
		if (!warn_crc)
			exit(1);
	}

	file_close(name, file, stdin);
}


void parse_env(void)
{
	char *p, *next, *eq;

	for (p = env+4; p != env+env_size; p = next+1) {
		next = strchr(p, 0);
		if (next == p)
			break;
		if (next == env+env_size) {
			fprintf(stderr,
			    "warning: unterminated entry in environment "
			    "\"%s\"\n", p);
			break;
		}
		eq = strchr(p, '=');
		if (!eq) {
			fprintf(stderr,
			    "warning: ignoring invalid environment entry "
			    "\"%s\"\n", p);
			continue;
		}
		if (next == eq+1) {
			fprintf(stderr,
			    "warning: skipping empty entry for \"%s\"\n", p);
			continue;
		}
		*eq = 0;
		set_var(p, eq+1);
	}
}


void print_env(const char *name)
{
	FILE *file;

	file = file_open(name, "w", stdout);
	reset_var();
	while (1) {
		const char *var, *value;

		var = next_var(&value);
		if (!var)
			break;
		if (fprintf(file, "%s=%s\n", var, value) < 0) {
			perror(name);
			exit(1);
		}
	}
	file_close(name, file, stdout);
}


void write_env(const char *name)
{
	FILE *file;
	char *p;
	int n;
	uint32_t crc;
	size_t wrote;

	memset(env, 0, env_size);
	reset_var();
	for (p = env+4; 1; p += n+1) {
		const char *var, *value;

		var = next_var(&value);
		if (!var)
			break;
		if (p-env+strlen(var)+strlen(value)+3 > env_size) {
			fprintf(stderr, "environment too big\n");
			exit(1);
		}
		n = sprintf(p, "%s=%s", var, value);
	}
	crc = crc_env();
	env[0] = crc;
	env[1] = crc >> 8;
	env[2] = crc >> 16;
	env[3] = crc >> 24;

	file = file_open(name, "w", stdout);
	wrote = fwrite(env, 1, env_size, file);
	if (ferror(file)) {
		perror(name);
		exit(1);
	}
	if (wrote != env_size) {
		fprintf(stderr, "%s: short write %lu < %lu\n", name,
		    (unsigned long) wrote, env_size);
		exit(1);
	}
	file_close(name, file, stdout);
}


void set_env(const char *var)
{
	char *tmp, *eq;

	tmp = strdup(var);
	if (!tmp) {
		perror("strdup");
		exit(1);
	}
	eq = strchr(tmp, '=');
	if (!eq) {
		fprintf(stderr, "invalid assignment syntax: \"%s\"\n", tmp);
		exit(1);
	}
	*eq = 0;
	if (eq[1])
		set_var(var, eq+1);
	else
		del_var(var);
}


void edit_env(const char *name, int cpp)
{
	FILE *file;

	if (!cpp)
		file = file_open(name, "r", stdin);
	else {
		run_cpp_on_file(name);
		file = stdin;
	}
	parse_edit(file);
	if (cpp)
		reap_cpp();
	else
		file_close(name, file, stdin);
}
