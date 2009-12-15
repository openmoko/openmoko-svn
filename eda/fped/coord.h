/*
 * coord.h - Coordinate representation and basic operations
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef COORD_H
#define COORD_H

#include <stdint.h>


#define MICRON_UNITS	10
#define	MIL_UNITS	(25.4*MICRON_UNITS)
#define	MM_UNITS	(1000.0*MICRON_UNITS)
#define	KICAD_UNIT	(MIL_UNITS/10.0)

#define	MIL_IN_MM	0.0254


typedef int32_t unit_type;


#define	UNIT_ERROR	((unit_type) 1 << (sizeof(unit_type)*8-1))


struct coord {
	unit_type x, y;
};


static inline unit_type mil_to_units(double mil)
{
	return mil*MIL_UNITS;
}


static inline unit_type mm_to_units(double mm)
{
	return mm*MM_UNITS;
}


static inline double units_to_mm(unit_type u)
{
	return (double) u/MM_UNITS;
}


static inline double units_to_mil(unit_type u)
{
	return (double) u/MIL_UNITS;
}


static inline int units_to_kicad(unit_type u)
{
	return (double) u/KICAD_UNIT;
}


static inline int coord_eq(struct coord a, struct coord b)
{
	return a.x == b.x && a.y == b.y;
}


double mm_to_mil(double mm, int exponent);
double mil_to_mm(double mil, int exponent);

double units_to_best(unit_type u, int *mm);

struct coord normalize(struct coord v, unit_type len);
struct coord rotate(struct coord v, double angle);
struct coord add_vec(struct coord a, struct coord b);
struct coord sub_vec(struct coord a, struct coord b);
struct coord neg_vec(struct coord v);

struct coord rotate_r(struct coord c, unit_type r, double angle);
double theta_vec(struct coord v);
double theta(struct coord c, struct coord p);

void swap_coord(unit_type *a, unit_type *b);
void sort_coord(struct coord *min, struct coord *max);

unit_type dist_point(struct coord a, struct coord b);
unit_type dist_line(struct coord p, struct coord a, struct coord b);
unit_type dist_rect(struct coord p, struct coord a, struct coord b);
int inside_rect(struct coord p, struct coord a, struct coord b);
unit_type dist_circle(struct coord p, struct coord c, unit_type r);

#endif /* !COORD_H */
