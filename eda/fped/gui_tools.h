/*
 * gui_tools.h - GUI, tool bar
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef GUI_TOOLS_H
#define	GUI_TOOLS_H

#include <gtk/gtk.h>

#include "inst.h"


struct tool_ops {
	void (*tool_selected)(void);
	void (*tool_deselected)(void);
	struct inst *(*find_point)(struct coord pos);
	void (*click)(struct coord pos);
	void (*begin_drag_new)(struct inst *from);
// in inst
//	void (*begin_drag_move)struct inst *from, int anchor_i);
	struct pix_buf *(*drag_new)(struct inst *from, struct coord to);
	int (*end_new_raw)(struct inst *from, struct coord to);
	int (*end_new)(struct inst *from, struct inst *to);
};


struct pix_buf *draw_move_vec(struct inst *inst, struct coord pos, int i);
struct pix_buf *draw_move_line(struct inst *inst, struct coord pos, int i);
struct pix_buf *draw_move_rect(struct inst *inst, struct coord pos, int i);
struct pix_buf *draw_move_pad(struct inst *inst, struct coord pos, int i);
struct pix_buf *draw_move_arc(struct inst *inst, struct coord pos, int i);
struct pix_buf *draw_move_meas(struct inst *inst, struct coord pos, int i);
struct pix_buf *draw_move_frame(struct inst *inst, struct coord pos, int i);

struct pix_buf *gui_hover_vec(struct inst *self);
struct pix_buf *gui_hover_frame(struct inst *self);

void do_move_to_arc(struct inst *inst, struct vec *vec, int i);

void tool_dehover(void);
void tool_hover(struct coord pos);
int tool_consider_drag(struct coord pos);
void tool_drag(struct coord to);
void tool_cancel_drag(void);
int tool_end_drag(struct coord to);
void tool_redraw(void);

/*
 * The following functions are for measurements which are now in a separate
 * compilation unit.
 */

struct pix_buf *draw_move_line_common(struct inst *inst,
    struct coord end, struct coord pos, int i);
struct pix_buf *drag_new_line(struct inst *from, struct coord to);


/*
 * Cache the frame and track it.
 */

void tool_frame_update(void);
void tool_frame_deleted(const struct frame *frame);

void tool_reset(void);

GtkWidget *gui_setup_tools(GdkDrawable *drawable);

#endif /* !GUI_TOOLS_H */
