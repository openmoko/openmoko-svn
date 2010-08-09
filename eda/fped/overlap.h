/*
 * overlap.h - Test for overlaps
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef OVERLAP_H
#define OVERLAP_H


enum allow_overlap {
	ao_none,
	ao_touch,
	ao_any,
};


/*
 * Avoid inst.h -> layer.h -> overlay.h -> inst.h loop
 */

struct inst;


/*
 * "inside" returns 1 if "a" is completely enclosed by "b". If "a" == "b",
 * that also counts as "a" being inside "b".
 */

int inside(const struct inst *a, const struct inst *b);

/*
 * "overlap" returns 1 if "a" and "b" have at least one point in common.
 */

int overlap(const struct inst *a, const struct inst *b,
    enum allow_overlap allow);

#endif /* !OVERLAP_H */
