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

struct inst_ops;
struct draw_ctx;

struct inst {
	const struct inst_ops *ops;
	struct coord base;
//	struct inst *base_inst; /* frame or vector leading to this item */
	struct bbox bbox;
	struct vec *vec; /* undefined if not vector */
	struct obj *obj; /* undefined if not object */
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


extern struct inst *selected_inst;
extern struct bbox active_frame_bbox;


void inst_select_outside(void *item, void (*deselect)(void *item));
int inst_select(const struct draw_ctx *ctx, struct coord pos);
void inst_deselect(void);

struct inst *inst_find_point(const struct draw_ctx *ctx, struct coord pos);
struct coord inst_get_point(const struct inst *inst);
int inst_anchors(struct inst *inst, struct vec ***anchors);
struct vec *inst_get_ref(const struct inst *inst);
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

void inst_begin_frame(const struct frame *frame, struct coord base,
    int active, int is_active_frame);
void inst_end_frame(const struct frame *frame);

struct bbox inst_get_bbox(void);

void inst_start(void);
void inst_commit(void);
void inst_revert(void);

void inst_draw(struct draw_ctx *ctx);
void inst_hover(struct inst *inst, struct draw_ctx *ctx, int on);
void inst_debug(void);

#endif /* !INST_H */
