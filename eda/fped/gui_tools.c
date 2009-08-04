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
	struct pix_buf *(*drag)(struct draw_ctx *ctx, struct inst *from,
	     struct coord to);
	int (*end)(struct draw_ctx *ctx, struct inst *from, struct inst *to);
};


static GtkWidget *ev_point;
static GtkWidget *active_tool;
static struct tool_ops *active_ops = NULL;
static struct inst *hover_inst = NULL;

static struct inst *drag;
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
	.drag	= drag_new_line,
	.end	= end_new_line,
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
	.drag	= drag_new_rect,
	.end	= end_new_rect,
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
	.drag	= drag_new_circ,
	.end	= end_new_circ,
};


/* ----- mouse actions ----------------------------------------------------- */


void tool_dehover(struct draw_ctx *ctx)
{
	if (hover_inst)
		inst_hover(hover_inst, ctx, 0);
	hover_inst = NULL;
}


void tool_hover(struct draw_ctx *ctx, struct coord pos)
{
	struct inst *inst;

	if (!active_ops)
		return;
	inst = inst_find_point(ctx, pos);
	if (inst != hover_inst)
		tool_dehover(ctx);
	if (inst) {
		inst_hover(inst, ctx, 1);
		hover_inst = inst;
	}
}


int tool_consider_drag(struct draw_ctx *ctx, struct coord pos)
{
	if (!active_ops)
		return 0;
	drag = inst_find_point(ctx, pos);
	if (!drag)
		return 0;
	pix_buf = NULL;
	return 1;
}


void tool_drag(struct draw_ctx *ctx, struct coord to)
{
	if (pix_buf)
		restore_pix_buf(pix_buf);
	tool_hover(ctx, to);
	pix_buf = active_ops->drag(ctx, drag, to);
}


void tool_cancel_drag(struct draw_ctx *ctx)
{
	tool_dehover(ctx);
	tool_reset();
	if (pix_buf)
		restore_pix_buf(pix_buf);
	drag = NULL;
	active_ops = NULL;
}


int tool_end_drag(struct draw_ctx *ctx, struct coord to)
{
	struct inst *from = drag;
	struct inst *end;
	struct tool_ops *ops = active_ops;

	tool_cancel_drag(ctx);
	end = inst_find_point(ctx, to);
	if (end)
		return ops->end(ctx, from, end);
	return 0;
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
