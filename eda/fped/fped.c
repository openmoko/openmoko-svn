/*
 * fped.c - Footprint editor, main function
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "cpp.h"
#include "util.h"
#include "error.h"
#include "obj.h"
#include "inst.h"
#include "file.h"
#include "gui.h"


extern void scan_empty(void);
extern int yyparse(void);

char *save_file_name = NULL;


static void load_file(const char *name)
{
	if (file_exists(name) == 1) {
		reporter = report_parse_error;
		run_cpp_on_file(name);
	} else {
		scan_empty();
	}
	(void) yyparse();
}


static void usage(const char *name)
{
	fprintf(stderr, "usage: %s [-k|-p] [in_file [out_file]]\n\n", name);
	fprintf(stderr, "  -k  write KiCad output, then exit\n");
	fprintf(stderr, "  -p  write Postscript output, then exit\n");
	exit(1);
}


int main(int argc, char **argv)
{
	const char *name = *argv;
	int error;
	int batch_write_kicad = 0, batch_write_ps = 0;
	int c;

	error = gui_init(&argc, &argv);
	if (error)
		return error;

	while ((c = getopt(argc, argv, "kp")) != EOF)
		switch (c) {
		case 'k':
			batch_write_kicad = 1;
			break;
		case 'p':
			batch_write_ps = 1;
			break;
		default:
			usage(name);
		}

	switch (argc-optind) {
	case 0:
		scan_empty();
		(void) yyparse();
		break;
	case 1:
		load_file(argv[optind]);
		save_file_name = argv[optind];
		break;
	case 2:
		load_file(argv[optind]);
		save_file_name = argv[optind+1];
		if (!strcmp(save_file_name, "-"))
			save_file_name = NULL;
		break;
	default:
		usage(name);
	}

	if (!part_name)
		part_name = stralloc("_");

	reporter = report_to_stderr;
	if (!instantiate())
		return 1;

	if (batch_write_kicad)
		write_kicad();
	if (batch_write_ps)
		write_ps();
	if (batch_write_kicad || batch_write_ps)
		exit(0);
		
//	inst_debug();
	error = gui_main();
	if (error)
		return error;

//	dump(stdout);

	return 0;
}
