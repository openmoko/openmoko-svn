/*
 * cpp.h - CPP subprocess
 *
 * Written 2002, 2003, 2008 by Werner Almesberger
 * Copyright 2002, 2003 Caltech Netlab FAST project
 * Copyright 2008 by OpenMoko, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CPP_H
#define CPP_H


extern const char *cpp_command;

void add_cpp_arg(const char *arg);
void add_cpp_Wp(const char *arg);
void run_cpp_on_file(const char *name); /* NULL for stdin */
void run_cpp_on_string(const char *str);
void reap_cpp(void);

#endif /* CPP_H */
