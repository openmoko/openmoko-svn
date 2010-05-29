/*
 * bitset.c - Arbitrary-length bit sets
 *
 * Written 2010 by Werner Almesberger
 * Copyright 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "util.h"
#include "bitset.h"


struct bitset {
	int v_n;
	unsigned *v;
};

#define BITS	(sizeof(unsigned)*8)


struct bitset *bitset_new(int n)
{
	struct bitset *new;

	new = alloc_type(struct bitset);
	new->v_n = (n+BITS-1) & ~(BITS-1);
	new->v = zalloc_size(sizeof(unsigned)*new->v_n);
	return new;
}


struct bitset *bitset_clone(const struct bitset *old)
{
	struct bitset *new;
	size_t bytes;

	new = alloc_type(struct bitset);
	bytes = sizeof(unsigned)*old->v_n;
	new->v_n = old->v_n;
	new->v = alloc_size(bytes);
	memcpy(new->v, old->v, bytes);
	return new;
}


void bitset_free(struct bitset *set)
{
	free(set->v);
	free(set);
}


void bitset_set(struct bitset *set, int n)
{
	assert(n < set->v_n*BITS);
	set->v[n/BITS] |= 1U << (n % BITS);
}


void bitset_clear(struct bitset *set, int n)
{
	assert(n < set->v_n*BITS);
	set->v[n/BITS] &= ~(1U << (n % BITS));
}


int bitset_pick(const struct bitset *set, int n)
{
	assert(n < set->v_n*BITS);
	return !!(set->v[n/BITS] & (1U << (n % BITS)));
}


int bitset_is_empty(const struct bitset *set)
{
	int i;

	for (i = 0; i != set->v_n; i++)
		if (set->v[i])
			return 0;
	return 1;
}


void bitset_zero(struct bitset *a)
{
	int i;

	for (i = 0; i != a->v_n; i++)
		a->v[i] = 0;
}


void bitset_and(struct bitset *a, const struct bitset *b)
{
	int i;

	assert(a->v_n == b->v_n);
	for (i = 0; i != a->v_n; i++)
		a->v[i] &= b->v[i];
}


void bitset_or(struct bitset *a, const struct bitset *b)
{
	int i;

	assert(a->v_n == b->v_n);
	for (i = 0; i != a->v_n; i++)
		a->v[i] |= b->v[i];
}


int bitset_ge(const struct bitset *a, const struct bitset *b)
{
	int i;

	assert(a->v_n == b->v_n);
	for (i = 0; i != a->v_n; i++)
		if (~a->v[i] & b->v[i])
			return 0;
	return 1;
}
