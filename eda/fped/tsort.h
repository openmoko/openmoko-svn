/*
 * tsort.h - Topological sort
 *
 * Written 2010 by Werner Almesberger
 * Copyright 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef TSORT_H
#define TSORT_H

struct node;
struct tsort;

struct node *add_node(struct tsort *tsort, void *user, int decay);
void add_edge(struct node *from, struct node *to, int priority);

struct tsort *begin_tsort(void);
void **end_tsort(struct tsort *tsort);

#endif /* !TSORT_H */
