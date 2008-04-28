/*
 * env.h - Environment access and file handling
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


#ifndef ENV_H
#define ENV_H

extern unsigned long env_size;

void read_env(const char *name, int warn_crc);
void parse_env(void);
void print_env(const char *name);
void write_env(const char *name);
void set_env(const char *var);
void edit_env(const char *name, int cpp);

#endif /* ENV_H */
