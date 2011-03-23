/*
 * kicad.h - Dump objects in the KiCad board/module format
 *
 * Written 2009, 2011 by Werner Almesberger
 * Copyright 2009, 2011 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef KICAD_H
#define KICAD_H

#include <stdio.h>


int kicad(FILE *file, const char *one);

#endif /* !KICAD_H */
