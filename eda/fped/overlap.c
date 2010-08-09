/*
 * overlap.c - Test for overlaps
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>

#include "coord.h"
#include "obj.h"
#include "inst.h"
#include "overlap.h"


/*
 * @@@ result may be too optimistic if "b" is a rounded pad
 */

int inside(const struct inst *a, const struct inst *b)
{
	struct coord min_a, max_a;
	struct coord min_b, max_b;

	min_a = a->base;
	switch (a->obj->type) {
	case ot_pad:
		max_a = a->u.pad.other;
		break;
	case ot_hole:
		max_a = a->u.hole.other;
		break;
	default:
		abort();
	}
	sort_coord(&min_a, &max_a);
	
	min_b = b->base;
	switch (b->obj->type) {
	case ot_pad:
		max_b = b->u.pad.other;
		break;
	case ot_hole:
		max_b = b->u.hole.other;
		break;
	default:
		abort();
	}
	sort_coord(&min_b, &max_b);

	return min_a.x >= min_b.x && max_a.x <= max_b.x &&
	    min_a.y >= min_b.y && max_a.y <= max_b.y;
}


/* ----- Overlap test for primitives --------------------------------------- */


struct shape {
	int circle;
	struct coord center;
	unit_type r;
	struct coord min, max;
};


static int circle_circle_overlap(struct coord c1, unit_type r1,
    struct coord c2, unit_type r2, enum allow_overlap allow)
{
	if (allow == ao_touch)
		return dist_point(c1, c2) < r1+r2;
	return dist_point(c1, c2) <= r1+r2;
}


static int circle_rect_overlap(struct coord c, unit_type r,
    struct coord min, struct coord max, enum allow_overlap allow)
{
	if (allow == ao_touch) {
		if (c.x <= min.x-r || c.x >= max.x+r)
			return 0;
		if (c.y <= min.y-r || c.y >= max.y+r)
			return 0;
	}
	if (c.x < min.x-r || c.x > max.x+r)
		return 0;
	if (c.y < min.y-r || c.y > max.y+r)
		return 0;
	return 1;
}


static int rect_rect_overlap(struct coord min1, struct coord max1,
    struct coord min2, struct coord max2, enum allow_overlap allow)
{
	if (allow == ao_touch) {
		if (max1.x <= min2.x || max2.x <= min1.x)
			return 0;
		if (max1.y <= min2.y || max2.y <= min1.y)
			return 0;
	}
	if (max1.x < min2.x || max2.x < min1.x)
		return 0;
	if (max1.y < min2.y || max2.y < min1.y)
		return 0;
	return 1;
}


static int shapes_overlap(const struct shape *a, const struct shape *b,
    enum allow_overlap allow)
{
	if (a->circle && !b->circle)
		return shapes_overlap(b, a, allow);
	if (a->circle) /* b must be circle, too */
		return circle_circle_overlap(a->center, a->r, b->center, b->r,
		    allow);
	if (b->circle) /* a must be rect */
		return circle_rect_overlap(b->center, b->r, a->min, a->max,
		    allow);
	return rect_rect_overlap(a->min, a->max, b->min, b->max, allow);
}


/* ----- Recursive overlap tester ------------------------------------------ */


static int test_overlap(const struct inst *a, const struct inst *b,
    const struct shape *other, enum allow_overlap allow);


static int do_circle(const struct inst *next, const struct shape *other,
    unit_type x, unit_type y, unit_type r, enum allow_overlap allow)
{
	struct shape shape = {
		.circle = 1,
		.center = {
			.x = x,
			.y = y,
		},
		.r = r,
	};

	if (next)
		return test_overlap(next, NULL, &shape, allow);
	return shapes_overlap(other, &shape, allow);
}


static int do_rect(const struct inst *next, const struct shape *other,
    unit_type x, unit_type y, unit_type w, unit_type h,
    enum allow_overlap allow)
{
	struct shape shape = {
		.circle = 0,
		.min = {
			.x = x,
			.y = y,
		},
		.max = {
			.x = x+w,
			.y = y+h,
		},
	};

	if (next)
		return test_overlap(next, NULL, &shape, allow);
	return shapes_overlap(other, &shape, allow);
}


static int test_overlap(const struct inst *a, const struct inst *b,
    const struct shape *other, enum allow_overlap allow)
{
	struct coord min, max;
	unit_type h, w, r;
	int rounded;

	min = a->base;
	switch (a->obj->type) {
	case ot_pad:
		max = a->u.pad.other;
		rounded = a->obj->u.pad.rounded;
		break;
	case ot_hole:
		max = a->u.hole.other;
		rounded = 1;
		break;
	default:
		abort();
	}
	sort_coord(&min, &max);

	h = max.y-min.y;
	w = max.x-min.x;

	if (!rounded)
		return do_rect(b, other, min.x, min.y, w, h, allow);

	if (h > w) {
		r = w/2;
		return do_circle(b, other, min.x+r, max.y-r, r, allow) ||
		    do_rect(b, other, min.x, min.y+r, w, h-2*r, allow) ||
		    do_circle(b, other, min.x+r, min.y+r, r, allow);
	} else {
		r = h/2;
		return do_circle(b, other, min.x+r, min.y+r, r, allow) ||
		    do_rect(b, other, min.x+r, min.y, w-2*r, h, allow) ||
		    do_circle(b, other, max.x-r, min.y+r, r, allow);
	}
}


int overlap(const struct inst *a, const struct inst *b,
    enum allow_overlap allow)
{
	if (allow == ao_any)
		return 0;
	return test_overlap(a, b, NULL, allow);
}
