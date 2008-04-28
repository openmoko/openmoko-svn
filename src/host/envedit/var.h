/*
 * var.h - Environment variable repository
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


#ifndef VAR_H
#define VAR_H

void set_var(const char *name, const char *value);
void del_var(const char *name);
void reset_var(void);
const char *next_var(const char const **value);

#endif /* VAR_H */
