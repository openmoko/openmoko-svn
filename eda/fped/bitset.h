/*
 * bitset.h - Arbitrary-length bit sets
 *
 * Written 2010 by Werner Almesberger
 * Copyright 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef BITSET_H
#define BITSET_H

struct bitset;

struct bitset *bitset_new(int n);
struct bitset *bitset_clone(const struct bitset *old);
void bitset_free(struct bitset *set);

void bitset_set(struct bitset *set, int n);
void bitset_clear(struct bitset *set, int n);
int bitset_pick(const struct bitset *set, int n);

int bitset_is_empty(const struct bitset *set);
void bitset_zero(struct bitset *a);

void bitset_and(struct bitset *a, const struct bitset *b);
void bitset_or(struct bitset *a, const struct bitset *b);

int bitset_ge(const struct bitset *a, const struct bitset *b);

#endif /* !BITSET_H */
