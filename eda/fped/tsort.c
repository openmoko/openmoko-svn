/*
 * tsort.c - Topological sort
 *
 * Written 2010 by Werner Almesberger
 * Copyright 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * We use a slight variation of Kahn's algorithm. The difference is that we add
 * a priority. Edges with the highest priority get selected before edges with
 * lower priority.
 *
 * We maintain the initial list of nodes in the order in which they were added.
 * Therefore, the first node with inbound edges will always be sorted first.
 * E.g., the root frame.
 *
 * add_node and add_edge can be invoked multiple times with the same
 * parameters. In the case of add_node, simply the existing node is returned.
 * In the case of add_edge, the new edge's priority is added to the priority of
 * the previous edges.
 *
 * Priority is accumulated in a node until the node is output. If a node has
 * the "decay" flag set, it resets the priorities of all other nodes when
 * output. E.g., when outputting a vector, all priorities accumulated from
 * previous vectors (towards referencing them with ".") lose their effect.
 *
 * Last but not least, the algorithm is stable: a pre-existing order that
 * conflicts neither with the partial order nor the priorities is preserved.
 *
 * Thus, we have the following sorting criteria, in decreasing importance:
 * - the destination if an edge never precedes its origin
 * - higher priority comes before lower priority
 * - earlier add_node comes before later
 */


#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "util.h"
#include "tsort.h"


struct edge {
	struct node *to;
	int priority;		/* edge priority */
	struct edge *next;
};

struct node {
	void *user;
	struct edge *edges;	/* outbound edges */
	int incoming;		/* number of incoming edges */
	int priority;		/* cumulative node priority */
	int decay;		/* all node prio. decay after issuing this */
	struct node *next;
};

struct tsort {
	struct node *nodes;
	struct node **next_node;
	int n_nodes;
};


void add_edge(struct node *from, struct node *to, int priority)
{
	struct edge **edge;

	for (edge = &from->edges; *edge; edge = &(*edge)->next)
		if ((*edge)->to == to) {
			(*edge)->priority += priority;
			return;
		}
	*edge = alloc_type(struct edge);
	(*edge)->to = to;
	(*edge)->priority = priority;
	(*edge)->next = NULL;
	to->incoming++;
}


struct node *add_node(struct tsort *tsort, void *user, int decay)
{
	struct node *node;

	for (node = tsort->nodes; node; node = node->next)
		if (node->user == user)
			return node;
	node = alloc_type(struct node);
	node->user = user;
	node->edges = NULL;
	node->incoming = 0;
	node->priority = 0;
	node->decay = decay;
	node->next = NULL;
	*tsort->next_node = node;
	tsort->next_node = &node->next;
	tsort->n_nodes++;
	return node;
}


struct tsort *begin_tsort(void)
{
	struct tsort *tsort;

	tsort = alloc_type(struct tsort);
	tsort->nodes = NULL;
	tsort->next_node = &tsort->nodes;
	tsort->n_nodes = 0;
	return tsort;
}


void **end_tsort(struct tsort *tsort)
{
	struct node **walk, **first, *node;
	struct edge *edge;
	void **res;
	int n = 0;

	res = alloc_size(sizeof(void *)*(tsort->n_nodes+1));
	while (1) {
		first = NULL;
		for (walk = &tsort->nodes; *walk; walk = &(*walk)->next) {
			if ((*walk)->incoming)
				continue;
			if (!first || (*first)->priority < (*walk)->priority)
				first = walk;
		}
		if (!first)
			break;
		if ((*first)->decay)
			for (node = tsort->nodes; node; node = node->next)
				node->priority = 0;
		node = *first;
		*first = node->next;
		res[n++] = node->user;
		while (node->edges) {
			edge = node->edges;
			edge->to->incoming--;
			edge->to->priority += edge->priority;
			node->edges = edge->next;
			free(edge);
		}
		free(node);
	}
	if (tsort->nodes) /* we have at least one cycle */
		abort();
	free(tsort);
	res[n] = NULL;
	return res;
}
