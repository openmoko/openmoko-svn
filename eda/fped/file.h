/*
 * file.h - File handling
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef FILE_H
#define FILE_H

#include <stdio.h>


/*
 * Returns -1 on error.
 */
int file_exists(const char *name);

char *set_extension(const char *name, const char *ext);
int save_to(const char *name, int (*fn)(FILE *file));
void save_with_backup(const char *name, int (*fn)(FILE *file));

void save_fpd(void);
void write_kicad(void);
void write_ps(void);
void write_ps_fullpage(void);

#endif /* !FILE_H */
