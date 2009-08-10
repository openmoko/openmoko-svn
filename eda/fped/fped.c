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

#include "cpp.h"
#include "util.h"
#include "error.h"
#include "obj.h"
#include "inst.h"
#include "gui.h"


extern void scan_empty(void);
extern int yyparse(void);

char *save_file = NULL;


static void load_file(const char *name)
{
	reporter = report_parse_error;
	run_cpp_on_file(name);
	(void) yyparse();
}


static void usage(const char *name)
{
	fprintf(stderr, "usage: %s [in_file [out_file]]\n", name);
	exit(1);
}


int main(int argc, char **argv)
{
	const char *name = *argv;
	int error;

	error = gui_init(&argc, &argv);
	if (error)
		return error;
	switch (argc) {
	case 1:
		scan_empty();
		(void) yyparse();
		break;
	case 3:
		save_file = argv[2];
		/* fall through */
	case 2:
		load_file(argv[1]);
		break;
	default:
		usage(name);
	}

	if (!part_name)
		part_name = stralloc("_");

	reporter = report_to_stderr;
	if (!instantiate())
		return 1;
//	inst_debug();
	error = gui_main();
	if (error)
		return error;

//	dump(stdout);

	return 0;
}
