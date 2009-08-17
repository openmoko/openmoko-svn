/*
 * inst.h - Instance structures
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef INST_H
#define INST_H

#include <stdio.h>

#include "coord.h"
#include "obj.h"
#include "meas.h"


enum mode {
	mode_inactive,		/* on inactive frame */
	mode_inactive_in_path,	/* inactive but is in path to selected */
	mode_active,		/* on active frame */
	mode_active_in_path,	/* active and is in path to selected */
	mode_selected,		/* item is selected */
	mode_hover,		/* hovering over item's contact area */
	mode_n			/* number of modes */
};

struct bbox {
	struct coord min;
	struct coord max;
};


enum inst_prio {
	ip_frame,	/* frames have their own selection */
	ip_pad,		/* pads also accept clicks inside */
	ip_circ,	/* circles don't overlap easily */
	ip_arc,		/* arc are like circles, just shorter */
	ip_rect,	/* rectangles have plenty of sides */
	ip_meas,	/* mesurements are like lines but set a bit apart */
	ip_line,	/* lines are easly overlapped by other things */
	ip_vec,		/* vectors only have the end point */
	ip_n,		/* number of priorities */
};


struct inst;


struct inst_ops {
	void (*debug)(struct inst *self);
	void (*save)(FILE *file, struct inst *self);
	void (*draw)(struct inst *self);
	struct pix_buf *(*hover)(struct inst *self);
	unit_type (*distance)(struct inst *self, struct coord pos, 
	    unit_type scale);
	void (*select)(struct inst *self);
	int (*anchors)(struct inst *self, struct vec ***anchors);
	void (*begin_drag_move)(struct inst *from, int i);
	struct inst *(*find_point)(struct inst *self, struct coord pos);
	struct pix_buf *(*draw_move)(struct inst *inst,
	    struct coord pos, int i);
	void (*end_drag_move)(void);
	/* arcs and measurements need this special override */
	void (*do_move_to)(struct inst *inst, struct inst *to, int i);
};

struct inst {
	const struct inst_ops *ops;
	struct coord base;
//	struct inst *base_inst; /* frame or vector leading to this item */
	struct bbox bbox;
	struct vec *vec; /* NULL if not vector */
	struct obj *obj; /* NULL if not object */
	struct inst *outer; /* frame containing this item */
	int active;
	int in_path;
	union {
		struct {
			const struct frame *ref;
			int active;
		} frame;
		const char *name;
		struct {
			struct coord end;
			unit_type width;
		} rect;
		struct {
			const char *name;
			struct coord other;
		} pad;
		struct {
			unit_type r;
			double a1, a2;
			unit_type width;
		} arc;
		struct {
			struct coord end;
			double offset;
		} meas;
	} u;
	struct inst *next;
};


struct pkg {
	const char *name;	/* NULL if global package */
	struct inst *insts[ip_n];
	struct inst **next_inst[ip_n];
	struct sample **samples;
	struct pkg *next;
};


extern struct inst *selected_inst;
extern struct pkg *pkgs;	/* list of packages */
extern struct pkg *active_pkg;	/* package selected in GUI */
extern struct pkg *curr_pkg;	/* package currently being instantiated */
extern struct bbox active_frame_bbox;

/*
 * frame being instantiated - we need to export this one for meas.c, so that
 * measurement scan update the root frame's bounding box.
 */
extern	struct inst *curr_frame;

/*
 * @@@ Note that we over-generalize a bit here: the only item that ever ends up
 * in the global package is currently the root frame. However, we may later
 * allow other items shared by all packages be there as well.
 */

#define FOR_INST_PRIOS_UP(prio)					\
	for (prio = 0; prio != ip_n; prio++)

#define FOR_INST_PRIOS_DOWN(prio)				\
	for (prio = ip_n-1; prio != (enum inst_prio) -1; prio--)

#define	FOR_GLOBAL_INSTS(prio, inst)			\
	for (inst = pkgs->insts[prio]; inst; inst = inst->next)

#define	FOR_PKG_INSTS(prio, inst)				\
	for (inst = active_pkg->insts[prio]; inst; inst = inst->next)


void inst_select_outside(void *item, void (*deselect)(void *item));
int inst_select(struct coord pos);
void inst_deselect(void);

void inst_select_vec(struct vec *vec);
void inst_select_obj(struct obj *obj);

struct inst *inst_find_point(struct coord pos);
int inst_find_point_selected(struct coord pos, struct inst **res);
struct coord inst_get_point(const struct inst *inst);
int inst_anchors(struct inst *inst, struct vec ***anchors);
struct vec *inst_get_vec(const struct inst *inst);

int inst_vec(struct vec *vec, struct coord base);
int inst_line(struct obj *obj, struct coord a, struct coord b, unit_type width);
int inst_rect(struct obj *obj, struct coord a, struct coord b, unit_type width);
int inst_pad(struct obj *obj, const char *name, struct coord a, struct coord b);
int inst_arc(struct obj *obj, struct coord center, struct coord start,
    struct coord stop, unit_type width);
int inst_meas(struct obj *obj, struct coord from, struct coord to,
    unit_type offset);

void inst_begin_active(int active);
void inst_end_active(void);

void inst_begin_frame(struct obj *obj, const struct frame *frame,
    struct coord base, int active, int is_active_frame);
void inst_end_frame(const struct frame *frame);

void inst_select_pkg(const char *name);

struct bbox inst_get_bbox(void);

void inst_start(void);
void inst_commit(void);
void inst_revert(void);

void inst_draw(void);
void inst_highlight_vecs(int (*pick)(struct inst *inst, void *user),
     void *user);
struct inst *inst_find_vec(struct coord pos, 
    int (*pick)(struct inst *inst, void *user), void *user);
struct inst *insts_ip_vec(void);

struct pix_buf *inst_draw_move(struct inst *inst, struct coord pos, int i);
int inst_do_move_to(struct inst *inst, struct inst *to, int i);
struct pix_buf *inst_hover(struct inst *inst);
void inst_begin_drag_move(struct inst *inst, int i);
void inst_delete(struct inst *inst);

#endif /* !INST_H */
