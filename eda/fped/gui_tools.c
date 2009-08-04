/*
 * gui_tools.c - GUI, tool bar
 *
 * Written 2009 by Werner Almesberger
 * Copyright 2009 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdlib.h>
#include <assert.h>
#include <gtk/gtk.h>

#include "util.h"
#include "inst.h"
#include "obj.h"
#include "gui_util.h"
#include "gui_style.h"
#include "gui_inst.h"
#include "gui_tools.h"


#include "icons/circ.xpm"
#include "icons/frame.xpm"
#include "icons/line.xpm"
#include "icons/meas.xpm"
#include "icons/pad.xpm"
#include "icons/point.xpm"
#include "icons/rect.xpm"
#include "icons/vec.xpm"


#define DA	GDK_DRAWABLE(ctx->widget->window)


struct tool_ops {
	struct pix_buf *(*drag_new)(struct draw_ctx *ctx, struct inst *from,
	     struct coord to);
	int (*end_new)(struct draw_ctx *ctx, struct inst *from,
	    struct inst *to);
};


static GtkWidget *ev_point;
static GtkWidget *active_tool;
static struct tool_ops *active_ops = NULL;
static struct inst *hover_inst = NULL;

static struct drag_state {
	struct inst *new; /* non-NULL if dragging a new object */
	int anchors_n; /* number of anchors, 0 if no moving */
	int anchor_i; /* current anchor */
	struct vec **anchors[3];
} drag = {
	.new = NULL,
	.anchors_n = 0,
};

static struct pix_buf *pix_buf;

static struct tool_ops vec_ops;
static struct tool_ops frame_ops;
static struct tool_ops pad_ops;
static struct tool_ops circ_ops;
static struct tool_ops meas_ops;


static struct obj *new_obj(enum obj_type type, struct inst *base)
{
	struct obj *obj, **walk;

	obj = alloc_type(struct obj);
	obj->type = type;
	obj->base = inst_get_ref(base);
	obj->next = NULL;
	obj->lineno = 0;
	for (walk = &active_frame->objs; *walk; walk = &(*walk)->next);
	*walk = obj;
	return obj;
}


/* ----- line -------------------------------------------------------------- */


static struct pix_buf *drag_new_line(struct draw_ctx *ctx,
    struct inst *from, struct coord to)
{
	struct coord pos;
	struct pix_buf *buf;

	pos = translate(ctx, inst_get_point(from));
	to = translate(ctx, to);
	buf = save_pix_buf(DA, pos.x, pos.y, to.x, to.y, 1);
	gdk_draw_line(DA, gc_drag, pos.x, pos.y, to.x, to.y);
	return buf;
}


static int end_new_line(struct draw_ctx *ctx,
    struct inst *from, struct inst *to)
{
	struct obj *obj;

	if (from == to)
		return 0;
	obj = new_obj(ot_line, from);
	obj->u.line.other = inst_get_ref(to);
	obj->u.line.width = NULL;
	return 1;
}


static struct tool_ops line_ops = {
	.drag_new	= drag_new_line,
	.end_new	= end_new_line,
};


/* ----- rect -------------------------------------------------------------- */


static void swap_coord(unit_type *a, unit_type *b)
{
	unit_type tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}


static struct pix_buf *drag_new_rect(struct draw_ctx *ctx,
    struct inst *from, struct coord to)
{
	struct coord pos;
	struct pix_buf *buf;

	pos = translate(ctx, inst_get_point(from));
	to = translate(ctx, to);
	if (pos.x > to.x)
		swap_coord(&pos.x, &to.x);
	if (pos.y > to.y)
		swap_coord(&pos.y, &to.y);
	buf = save_pix_buf(DA, pos.x, pos.y, to.x, to.y, 1);
	gdk_draw_rectangle(DA, gc_drag, FALSE,
	    pos.x, pos.y, to.x-pos.x, to.y-pos.y);
	return buf;
}


static int end_new_rect(struct draw_ctx *ctx,
    struct inst *from, struct inst *to)
{
	struct obj *obj;

	if (from == to)
		return 0;
	obj = new_obj(ot_rect, from);
	obj->u.rect.other = inst_get_ref(to);
	obj->u.rect.width = NULL;
	return 1;
}


static struct tool_ops rect_ops = {
	.drag_new	= drag_new_rect,
	.end_new	= end_new_rect,
};


/* ----- circ -------------------------------------------------------------- */


static struct pix_buf *drag_new_circ(struct draw_ctx *ctx,
    struct inst *from, struct coord to)
{
	struct coord pos;
	struct pix_buf *buf;
	double r;

	pos = translate(ctx, inst_get_point(from));
	to = translate(ctx, to);
	r = hypot(to.x-pos.x, to.y-pos.y);
	buf = save_pix_buf(DA, pos.x-r, pos.y-r, pos.x+r, pos.y+r, 1);
	draw_circle(DA, gc_drag, FALSE, pos.x, pos.y, r);
	return buf;
}


static int end_new_circ(struct draw_ctx *ctx,
    struct inst *from, struct inst *to)
{
	struct obj *obj;

	if (from == to)
		return 0;
	obj = new_obj(ot_arc, from);
	obj->u.arc.start = inst_get_ref(to);
	obj->u.arc.end = inst_get_ref(to);
	obj->u.arc.width = NULL;
	return 1;
}


static struct tool_ops circ_ops = {
	.drag_new	= drag_new_circ,
	.end_new	= end_new_circ,
};


/* ----- moving references ------------------------------------------------- */


static int may_move(struct inst *curr)
{
	if (!selected_inst)
		return 0;
	if (drag.anchors_n)
		return 0; /* already moving something else */
	drag.anchors_n = inst_anchors(selected_inst, drag.anchors);
	for (drag.anchor_i = 0; drag.anchor_i != drag.anchors_n;
	    drag.anchor_i++)
		if (*drag.anchors[drag.anchor_i] == inst_get_vec(curr))
			return 1;
	drag.anchors_n = 0;
	return 0;
}


static int would_be_equal(const struct drag_state *state,
    int a, int b, struct inst *curr)
{
	const struct vec *va;
	const struct vec *vb;

	va = a == state->anchor_i ? inst_get_vec(curr) : *state->anchors[a];
	vb = b == state->anchor_i ? inst_get_vec(curr) : *state->anchors[b];
	return va == vb;
}


static int may_move_to(const struct drag_state *state, struct inst *curr)
{
	assert(selected_inst);
	assert(state->anchors_n);
	switch (state->anchors_n) {
	case 3:
		if (would_be_equal(state, 0, 2, curr))
			return 0;
		/* fall through */
	case 2:
		if (would_be_equal(state, 0, 1, curr))
			return 0;
		/* fall through */
	case 1:
		return 1;
	default:
		abort();
	}
}


static void do_move_to(struct drag_state *state, struct inst *curr)
{
	struct vec *old;

	assert(may_move_to(state, curr));
	old = *state->anchors[state->anchor_i];
	if (old)
		old->n_refs--;
	*state->anchors[state->anchor_i] = inst_get_ref(curr);
	state->anchors_n = 0;
}


/* ----- hover ------------------------------------------------------------- */


void tool_dehover(struct draw_ctx *ctx)
{
	if (hover_inst)
		inst_hover(hover_inst, ctx, 0);
	hover_inst = NULL;
}


void tool_hover(struct draw_ctx *ctx, struct coord pos)
{
	struct inst *curr;

	curr = inst_find_point(ctx, pos);
	if (curr && !active_ops) {
		if (drag.anchors_n) {
			if (!may_move_to(&drag, curr))
				curr = NULL;
		} else {
			if (!may_move(curr))
				curr = NULL;
			drag.anchors_n = 0;
		}
	}
	if (curr != hover_inst)
		tool_dehover(ctx);
	if (curr) {
		inst_hover(curr, ctx, 1);
		hover_inst = curr;
	}
}


/* ----- mouse actions ----------------------------------------------------- */


int tool_consider_drag(struct draw_ctx *ctx, struct coord pos)
{
	struct inst *curr;

	assert(!drag.new);
	assert(!drag.anchors_n);
	curr = inst_find_point(ctx, pos);
	if (!curr)
		return 0;
	pix_buf = NULL;
	if (active_ops) {
		drag.new = curr;
		return 1;
	}
	return may_move(curr);
}


void tool_drag(struct draw_ctx *ctx, struct coord to)
{
	if (pix_buf)
		restore_pix_buf(pix_buf);
	tool_hover(ctx, to);
	pix_buf = drag.new ? active_ops->drag_new(ctx, drag.new, to) : NULL;
}


void tool_cancel_drag(struct draw_ctx *ctx)
{
	tool_dehover(ctx);
	tool_reset();
	if (pix_buf)
		restore_pix_buf(pix_buf);
	drag.new = NULL;
	active_ops = NULL;
	drag.anchors_n = 0;
}


int tool_end_drag(struct draw_ctx *ctx, struct coord to)
{
	struct drag_state state = drag;
	struct inst *end;
	struct tool_ops *ops = active_ops;

	tool_cancel_drag(ctx);
	end = inst_find_point(ctx, to);
	if (!end)
		return 0;
	if (state.new)
		return ops->end_new(ctx, state.new, end);
	if (!may_move_to(&state, end))
		return 0;
	do_move_to(&state, end);
	return 1;
}


/* ----- tool bar creation ------------------------------------------------- */


static void tool_select(GtkWidget *evbox, struct tool_ops *ops)
{
	GdkColor col;

	if (active_tool) {
		col = get_color(TOOL_UNSELECTED);
		gtk_widget_modify_bg(active_tool, GTK_STATE_NORMAL, &col);
		active_tool = NULL;
	}
	col = get_color(TOOL_SELECTED);
	gtk_widget_modify_bg(evbox, GTK_STATE_NORMAL, &col);
	active_tool = evbox;
	active_ops = ops;
}


void tool_reset(void)
{
	tool_select(ev_point, NULL);
}


static gboolean tool_button_press_event(GtkWidget *widget,
    GdkEventButton *event, gpointer data)
{
	tool_select(widget, data);
	return TRUE;
}


static GtkWidget *tool_button(GtkWidget *bar, GdkDrawable *drawable,
    char **xpm, GtkWidget *last_evbox, struct tool_ops *ops)
{
	GdkPixmap *pixmap;
	GtkWidget *image, *evbox;	
	GtkToolItem *item;
	GtkToolItem *last = NULL;

	if (last_evbox)
		last = GTK_TOOL_ITEM(gtk_widget_get_ancestor(last_evbox,
		    GTK_TYPE_TOOL_ITEM));
	pixmap = gdk_pixmap_create_from_xpm_d(drawable, NULL, NULL, xpm);
	image = gtk_image_new_from_pixmap(pixmap, NULL);

/*
 * gtk_radio_tool_button_new_from_widget is *huge*. we try to do things in a
 * more compact way.
 */
#if 0
	if (last)
		item = gtk_radio_tool_button_new_from_widget(
		    GTK_RADIO_TOOL_BUTTON(last));
	else
		item = gtk_radio_tool_button_new(NULL);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(item), image);
#else
	evbox = gtk_event_box_new();
	gtk_misc_set_padding(GTK_MISC(image), 1, 1);
	gtk_container_add(GTK_CONTAINER(evbox), image);
	g_signal_connect(G_OBJECT(evbox), "button_press_event",
            G_CALLBACK(tool_button_press_event), ops);

	item = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(item), evbox);

	gtk_container_set_border_width(GTK_CONTAINER(item), 0);
#endif

	gtk_toolbar_insert(GTK_TOOLBAR(bar), item, -1);

	return evbox;
}


GtkWidget *gui_setup_tools(GdkDrawable *drawable)
{
	GtkWidget *bar;
	GtkWidget *last;

	bar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(bar), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_orientation(GTK_TOOLBAR(bar),
	    GTK_ORIENTATION_VERTICAL);

	ev_point = tool_button(bar, drawable, xpm_point, NULL, NULL);
	last = tool_button(bar, drawable, xpm_vec, ev_point, &vec_ops);
	last = tool_button(bar, drawable, xpm_frame, last, &frame_ops);
	last = tool_button(bar, drawable, xpm_pad, last, &pad_ops);
	last = tool_button(bar, drawable, xpm_line, last, &line_ops);
	last = tool_button(bar, drawable, xpm_rect, last, &rect_ops);
	last = tool_button(bar, drawable, xpm_circ, last, &circ_ops);
	last = tool_button(bar, drawable, xpm_meas, last, &meas_ops);

	tool_reset();

	return bar;
}
