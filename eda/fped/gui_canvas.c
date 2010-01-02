/*
 * gui_canvas.c - GUI, canvas
 *
 * Written 2009, 2010 by Werner Almesberger
 * Copyright 2009, 2010 by Werner Almesberger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "obj.h"
#include "delete.h"
#include "inst.h"
#include "gui_util.h"
#include "gui_inst.h"
#include "gui_style.h"
#include "gui_status.h"
#include "gui_tool.h"
#include "gui.h"
#include "gui_canvas.h"


#if 0
#define	DPRINTF(fmt, ...)	fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#else
#define	DPRINTF(fmt, ...)
#endif


void (*highlight)(void) = NULL;

static struct coord curr_pos; /* canvas coordinates ! */
static struct coord user_origin = { 0, 0 };

static int dragging = 0;
static int drag_escaped = 0; /* 1 once we've made is out of the drag radius */
static struct coord drag_start;


/* ----- status display ---------------------------------------------------- */


static void update_zoom(void)
{
	status_set_zoom("x%d", draw_ctx.scale);
}


static void update_pos(struct coord pos)
{
	struct coord user;
	unit_type diag;

	set_with_units(status_set_sys_x, "X ", pos.x);
	set_with_units(status_set_sys_y, "Y ", pos.y);

	user.x = pos.x-user_origin.x;
	user.y = pos.y-user_origin.y;
	set_with_units(status_set_user_x, "x ", user.x);
	set_with_units(status_set_user_y, "y ", user.y);

	if (!selected_inst) {
		diag = hypot(user.x, user.y);
		set_with_units(status_set_r, "r = ", diag);
		status_set_angle_xy(user);
	}
}


void refresh_pos(void)
{
	update_pos(canvas_to_coord(curr_pos.x, curr_pos.y));
}


/* ----- coordinate system ------------------------------------------------- */


static void center(const struct bbox *this_bbox)
{
	struct bbox bbox;

	bbox = this_bbox ? *this_bbox : inst_get_bbox();
	draw_ctx.center.x = (bbox.min.x+bbox.max.x)/2;
	draw_ctx.center.y = (bbox.min.y+bbox.max.y)/2;
}


static void auto_scale(const struct bbox *this_bbox)
{
	struct bbox bbox;
	unit_type h, w;
	int sx, sy;
	float aw, ah;

	bbox = this_bbox ? *this_bbox : inst_get_bbox();
	aw = draw_ctx.widget->allocation.width;
	ah = draw_ctx.widget->allocation.height;
	h = bbox.max.x-bbox.min.x;
	w = bbox.max.y-bbox.min.y;
	aw -= 2*CANVAS_CLEARANCE;
	ah -= 2*CANVAS_CLEARANCE;
	if (aw < 1)
		aw = 1;
	if (ah < 1)
		ah = 1;
	sx = ceil(h/aw);
	sy = ceil(w/ah);
	draw_ctx.scale = sx > sy ? sx : sy > 0 ? sy : 1;

	update_zoom();
}


/* ----- drawing ----------------------------------------------------------- */


void redraw(void)
{
	float aw, ah;

	aw = draw_ctx.widget->allocation.width;
	ah = draw_ctx.widget->allocation.height;
	gdk_draw_rectangle(draw_ctx.widget->window,
	    instantiation_error ? gc_bg_error : gc_bg, TRUE, 0, 0, aw, ah);

	DPRINTF("--- redraw: inst_draw ---");
	inst_draw();
	if (highlight)
		highlight();
	DPRINTF("--- redraw: tool_redraw ---");
	tool_redraw();
	DPRINTF("--- redraw: done ---");
}


/* ----- drag -------------------------------------------------------------- */


static void drag_left(struct coord pos)
{
	if (!dragging)
		return;
	if (!drag_escaped &&
	    hypot(pos.x-drag_start.x, pos.y-drag_start.y)/draw_ctx.scale <
	    DRAG_MIN_R)
		return;
	drag_escaped = 1;
	tool_drag(pos);
}


static void drag_middle(struct coord pos)
{
}


static gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event,
    gpointer data)
{
	struct coord pos = canvas_to_coord(event->x, event->y);

	DPRINTF("--- motion ---");
	curr_pos.x = event->x;
	curr_pos.y = event->y;
	tool_hover(pos);
	if (event->state & GDK_BUTTON1_MASK)
		drag_left(pos);
	if (event->state & GDK_BUTTON2_MASK)
		drag_middle(pos);
	update_pos(pos);
	return TRUE;
}


/* ----- button press and release ------------------------------------------ */


static gboolean button_press_event(GtkWidget *widget, GdkEventButton *event,
    gpointer data)
{
	struct coord pos = canvas_to_coord(event->x, event->y);
	const struct inst *prev;
	int res;

	DPRINTF("--- button press ---");
	gtk_widget_grab_focus(widget);
	switch (event->button) {
	case 1:
		if (dragging) {
			fprintf(stderr, "HUH ?!?\n");
			tool_cancel_drag();
			dragging = 0;
		}
		res = tool_consider_drag(pos);
		/* tool doesn't do drag */
		if (res < 0) {
			change_world();
			inst_deselect();
			break;
		}
		if (res) {
			inst_deselect();
			redraw();
			dragging = 1;
			drag_escaped = 0;
			drag_start = pos;
			break;
		}
		prev = selected_inst;
		inst_select(pos);
		if (prev != selected_inst)
			redraw();
		break;
	case 2:
		tool_dehover();
		draw_ctx.center = pos;
		redraw();
		tool_hover(canvas_to_coord(event->x, event->y));
		break;
	}
	return TRUE;
}


static gboolean button_release_event(GtkWidget *widget, GdkEventButton *event,
    gpointer data)
{
	struct coord pos = canvas_to_coord(event->x, event->y);

	DPRINTF("--- button release ---");
	switch (event->button) {
	case 1:
		if (!dragging)
			break;
		dragging = 0;
		if (hypot(pos.x-drag_start.x,
		    pos.y-drag_start.y)/draw_ctx.scale < DRAG_MIN_R)
			tool_cancel_drag();
		else {
			if (tool_end_drag(pos))
				change_world();
		}
		break;
	}
	return TRUE;
}


/* ----- zoom control ------------------------------------------------------ */


static void zoom_in(struct coord pos)
{
	if (draw_ctx.scale < 2)
		return;
	tool_dehover();
	draw_ctx.scale /= 2;
	draw_ctx.center.x = (draw_ctx.center.x+pos.x)/2;
	draw_ctx.center.y = (draw_ctx.center.y+pos.y)/2;
	update_zoom();
	redraw();
	tool_hover(pos);
}


static void zoom_out(struct coord pos)
{
	struct bbox bbox;

	bbox = inst_get_bbox();
	bbox.min = translate(bbox.min);
	bbox.max = translate(bbox.max);
	if (bbox.min.x >= ZOOM_STOP_BORDER &&
	    bbox.max.y >= ZOOM_STOP_BORDER &&
	    bbox.max.x < draw_ctx.widget->allocation.width-ZOOM_STOP_BORDER &&
	    bbox.min.y < draw_ctx.widget->allocation.height-ZOOM_STOP_BORDER)
		return;
	tool_dehover();
	draw_ctx.scale *= 2;
	draw_ctx.center.x = 2*draw_ctx.center.x-pos.x;
	draw_ctx.center.y = 2*draw_ctx.center.y-pos.y;
	update_zoom();
	redraw();
	tool_hover(pos);
}


void zoom_in_center(void)
{
	zoom_in(draw_ctx.center);
}


void zoom_out_center(void)
{
	zoom_out(draw_ctx.center);
}


void zoom_to_frame(void)
{
	tool_dehover();
	center(&active_frame_bbox);
	auto_scale(&active_frame_bbox);
	redraw();
	tool_hover(canvas_to_coord(curr_pos.x, curr_pos.y));
}


void zoom_to_extents(void)
{
	tool_dehover();
	center(NULL);
	auto_scale(NULL);
	redraw();
	tool_hover(canvas_to_coord(curr_pos.x, curr_pos.y));
}


static gboolean scroll_event(GtkWidget *widget, GdkEventScroll *event,
    gpointer data)
{
	struct coord pos = canvas_to_coord(event->x, event->y);

	gtk_widget_grab_focus(widget);
	switch (event->direction) {
	case GDK_SCROLL_UP:
		zoom_in(pos);
		break;
	case GDK_SCROLL_DOWN:
		zoom_out(pos);
		break;
	default:
		/* ignore */;
	}
	return TRUE;
}


/* ----- keys -------------------------------------------------------------- */


static gboolean key_press_event(GtkWidget *widget, GdkEventKey *event,
    gpointer data)
{
	struct coord pos = canvas_to_coord(curr_pos.x, curr_pos.y);

	DPRINTF("--- key press ---");
	switch (event->keyval) {
	case ' ':
		user_origin = pos;
		update_pos(pos);
		break;
	case '+':
	case '=':
		zoom_in(pos);
		break;
	case '-':
		zoom_out(pos);
		break;
	case '*':
		zoom_to_extents();
		break;
	case '#':
		zoom_to_frame();
		break;
	case '.':
		tool_dehover();
		draw_ctx.center = pos;
		redraw();
		tool_hover(canvas_to_coord(curr_pos.x, curr_pos.y));
		break;
	case GDK_BackSpace:
	case GDK_Delete:
#if 0
	case GDK_KP_Delete:
		if (selected_inst) {
			inst_delete(selected_inst);
			tool_frame_update();
			change_world();
		}
		break;
#endif
	case 'u':
		if (undelete())
			change_world();
		break;
	case '/':
{
/* @@@ find a better place for this */
extern int show_vars;
		show_vars = !show_vars;
change_world();
}
	}
	return TRUE;
}


/* ----- expose event ------------------------------------------------------ */


static gboolean expose_event(GtkWidget *widget, GdkEventExpose *event,
    gpointer data)
{
	static int first = 1;

	DPRINTF("--- expose ---");
	if (first) {
		init_canvas();
		first = 0;
	}
	tool_dehover();
	redraw();
	return TRUE;
}


/* ----- enter/leave ------------------------------------------------------- */


static gboolean enter_notify_event(GtkWidget *widget, GdkEventCrossing *event,
    gpointer data)
{
	gtk_widget_grab_focus(widget);
	return TRUE;
}


static gboolean leave_notify_event(GtkWidget *widget, GdkEventCrossing *event,
    gpointer data)
{
	DPRINTF("--- leave ---");
	if (dragging)
		tool_cancel_drag();
	tool_dehover();
	dragging = 0;
	return TRUE;
}


/* ----- tooltip ----------------------------------------------------------- */


static gboolean canvas_tooltip(GtkWidget *widget, gint x, gint y,
    gboolean keyboard_mode, GtkTooltip *tooltip, gpointer user_data)
{
	struct coord pos = canvas_to_coord(x, y);
	const char *res;

	res = tool_tip(pos);
	if (!res)
		return FALSE;
	gtk_tooltip_set_markup(tooltip, res);
	return TRUE;
}


/* ----- canvas setup ------------------------------------------------------ */


/*
 * Note that we call init_canvas twice: first to make sure we'll make it safely
 * through select_frame, and the second time to set the geometry for the actual
 * screen.
 */

void init_canvas(void)
{
	center(NULL);
	auto_scale(NULL);
}


GtkWidget *make_canvas(void)
{
	GtkWidget *canvas;
	GdkColor black = { 0, 0, 0, 0 };

	/* Canvas */

	canvas = gtk_drawing_area_new();
	gtk_widget_modify_bg(canvas, GTK_STATE_NORMAL, &black);

	g_signal_connect(G_OBJECT(canvas), "motion_notify_event",
	    G_CALLBACK(motion_notify_event), NULL);
	g_signal_connect(G_OBJECT(canvas), "button_press_event",
	    G_CALLBACK(button_press_event), NULL);
	g_signal_connect(G_OBJECT(canvas), "button_release_event",
	    G_CALLBACK(button_release_event), NULL);
	g_signal_connect(G_OBJECT(canvas), "scroll_event",
	    G_CALLBACK(scroll_event), NULL);

	GTK_WIDGET_SET_FLAGS(canvas, GTK_CAN_FOCUS);

	g_signal_connect(G_OBJECT(canvas), "key_press_event",
	    G_CALLBACK(key_press_event), NULL);

	g_signal_connect(G_OBJECT(canvas), "expose_event",
	    G_CALLBACK(expose_event), NULL);
	g_signal_connect(G_OBJECT(canvas), "enter_notify_event",
	    G_CALLBACK(enter_notify_event), NULL);
	g_signal_connect(G_OBJECT(canvas), "leave_notify_event",
	    G_CALLBACK(leave_notify_event), NULL);

	gtk_widget_set(canvas, "has-tooltip", TRUE, NULL);
	g_signal_connect(G_OBJECT(canvas), "query_tooltip",
	    G_CALLBACK(canvas_tooltip), NULL);

	gtk_widget_set_events(canvas,
	    GDK_EXPOSE | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
	    GDK_KEY_PRESS_MASK |
	    GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
	    GDK_SCROLL |
	    GDK_POINTER_MOTION_MASK);

	gtk_widget_set_double_buffered(canvas, FALSE);

	draw_ctx.widget = canvas;

	return canvas;
}
