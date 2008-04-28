/*
 * envedit.c - U-Boot environment editor
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
#include <unistd.h>
#include <string.h>

#include "cpp.h"
#include "env.h"


static void usage(const char *name)
{
	fprintf(stderr,
"usage: %s [-f env_file [-n]] [-i file [-c]] [-o file|-p] [-s bytes]\n"
"       %8s[-D var[=value]] [var=[value] ...]\n\n"
"  -c             ignore CRC errors in input environment\n"
"  -f env_file    read changed from file (default: no changes from file)\n"
"  -i file        read environment from file (default: use empty environment)\n"
"  -n             don't run env_file through CPP\n"
"  -o file        write environment to file (default: write to stdout)\n"
"  -p             print environment in human-readable form\n"
"  -s bytes       environment size in bytes (default: 16384)\n"
"  -D var[=value] define a variable for env_file processing only\n"
"  var=           remove the specified variable\n"
"  var=value      set the specified variable\n",
	  name, "");
	exit(1);
}


int main(int argc, char **argv)
{
	int warn_crc = 0;
	const char *env_file = NULL;
	const char *in_file = NULL, *out_file = NULL;
	int cpp = 1;
	int print = 0;
	char *tmp, *end;
	int c, i;

	while ((c = getopt(argc, argv, "cf:i:no:ps:D:")) != EOF)
		switch (c) {
		case 'c':
			warn_crc = 1;
			break;
		case 'f':
			env_file = optarg;
			break;
		case 'i':
			in_file = optarg;
			break;
		case 'n':
			cpp = 0;
			break;
		case 'o':
			out_file = optarg;
			break;
		case 'p':
			print = 1;
			break;
		case 's':
			env_size = strtoul(optarg, &end, 0);
			if (*end)
				usage(*argv);
			break;
		case 'D':
			tmp = malloc(strlen(optarg)+3);
			if (!tmp) {
				perror("strdup");
				exit(1);
			}
			tmp[0] = '-';
			tmp[1] = 'D';
			strcpy(tmp+2, optarg);
			add_cpp_arg(tmp);
			free(tmp);
			break;
		default:
			usage(*argv);
		}

	if (!in_file && warn_crc)
		usage(*argv);
	if (out_file && print)
		usage(*argv);
	if (!env_file && !cpp)
		usage(*argv);

	if (in_file) {
		read_env(in_file, warn_crc);
		parse_env();
	}
	if (env_file)
		edit_env(env_file, cpp);

	for (i = optind; i != argc; i++)
		set_env(argv[i]);

	if (print)
		print_env("-");
	else
		write_env(out_file ? out_file : "-");

	return 0;
}
