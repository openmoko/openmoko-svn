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



#include "cpp.h"
#include "error.h"
#include "obj.h"
#include "inst.h"
#include "gui.h"


extern void scan_empty(void);
extern int yyparse(void);


static void load_file(const char *name)
{
	reporter = report_parse_error;
	run_cpp_on_file(name);
	(void) yyparse();
}


int main(int argc, char **argv)
{
	int error;

	error = gui_init(&argc, &argv);
	if (error)
		return error;
	if (argc == 1) {
		scan_empty();
		(void) yyparse();
	} else {
		load_file(argv[1]);
		argc--;
		argv++;
	}
	reporter = report_to_stderr;
	if (!instantiate())
		return 1;
//	inst_debug();
	error = gui_main(argc, argv);
	if (error)
		return error;
	return 0;
}
