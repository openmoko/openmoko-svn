/*
 *  libmokoui -- OpenMoko Application Framework UI Library
 *
 *  Authored by Chris Lord <chris@openedhand.com>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

/**
 * SECTION: moko-finger-scroll
 * @short_description: A scrolling widget designed for touch screens
 * @see_also: #GtkScrolledWindow
 *
 * #MokoFingerScroll implements a scrolled window designed to be used with a
 * touch screen interface. The user scrolls the child widget by activating the
 * pointing device and dragging it over the widget.
 *
 */

/* TODO:
 * - Scroll policies
 * - Delay click mode (only send synthetic clicks on mouse-up, as in previous
 *   versions.
 * - 'Physical' mode for acceleration scrolling
 */

#include <gdk/gdkx.h>
#include "moko-finger-scroll.h"

G_DEFINE_TYPE (MokoFingerScroll, moko_finger_scroll, GTK_TYPE_EVENT_BOX)
#define FINGER_SCROLL_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_SCROLL, \
	MokoFingerScrollPrivate))
typedef struct _MokoFingerScrollPrivate MokoFingerScrollPrivate;

struct _MokoFingerScrollPrivate {
	MokoFingerScrollMode mode;
	gdouble x;	/* Used to store mouse co-ordinates of the first or */
	gdouble y;	/* previous events in a press-motion pair */
	gdouble ex;	/* Used to store mouse co-ordinates of the last */
	gdouble ey;	/* motion event in acceleration mode */
	gboolean enabled;
	gboolean clicked;
	GdkEventType last_type;	/* Last event type and time, to stop */
	guint32 last_time;	/* infinite loops */
	gboolean moved;
	GTimeVal click_start;
	gdouble vmin;
	gdouble vmax;
	gdouble decel;
	guint sps;
	gdouble vel_x;
	gdouble vel_y;
	GdkWindow *child;
	gint ix;	/* Initial click mouse co-ordinates */
	gint iy;
	gint cx;	/* Initial click child window mouse co-ordinates */
	gint cy;
	guint idle_id;
	
	GtkWidget *align;
	gboolean hscroll;
	gboolean vscroll;
	GdkRectangle hscroll_rect;
	GdkRectangle vscroll_rect;
	guint scroll_width;

	GtkAdjustment *hadjust;
	GtkAdjustment *vadjust;
};

enum {
	PROP_ENABLED = 1,
	PROP_MODE,
	PROP_VELOCITY_MIN,
	PROP_VELOCITY_MAX,
	PROP_DECELERATION,
	PROP_SPS,
};

/* Following function inherited from libhildondesktop */
static GList *
get_ordered_children (GdkWindow *window)
{
	Window      *children, root, parent;
	guint        i, n_children = 0;
	GList        *ret = NULL;

	gdk_error_trap_push ();
	XQueryTree (GDK_DISPLAY (), GDK_WINDOW_XID (window), &root,
		&parent, &children, &n_children);

	if (gdk_error_trap_pop ()) return NULL;

	for (i = 0; i < n_children; i++) {
		GdkWindow *window = gdk_window_lookup (children[i]);
		if (window) ret = g_list_append (ret, window);
	}

	XFree (children);

	return ret;
}

static GdkWindow *
moko_finger_scroll_get_topmost (GdkWindow *window, gint x, gint y,
				gint *tx, gint *ty)
{
	/* Find the GdkWindow at the given point, by recursing from a given
	 * parent GdkWindow. Optionally return the co-ordinates transformed
	 * relative to the child window.
	 */
	gint width, height;
	
	gdk_drawable_get_size (GDK_DRAWABLE (window), &width, &height);
	if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) return NULL;
	
	/*g_debug ("Finding window at (%d, %d) in %p", x, y, window);*/
	
	while (window) {
		gint child_x = 0, child_y = 0;
		GList *c, *children = get_ordered_children (window);
		GdkWindow *old_window = window;
		
		for (c = children; c; c = c->next) {
			GdkWindow *child = (GdkWindow *)c->data;
			gint wx, wy;
			
			gdk_window_get_geometry (child, &wx, &wy,
				&width, &height, NULL);
			/*g_debug ("Child: %p, (%dx%d+%d,%d)", child,
				width, height, wx, wy);*/
			
			if ((x >= wx) && (x < (wx + width)) &&
			    (y >= wy) && (y < (wy + height))) {
				child_x = x - wx; child_y = y - wy;
				window = child;
			}
		}
		
		g_list_free (children);
		
		/*g_debug ("\\|/");*/
		if (window == old_window) break;
		
		x = child_x;
		y = child_y;
	}
	
	if (tx) *tx = x;
	if (ty) *ty = y;

	/*g_debug ("Returning: %p", window);*/

	return window;
}

static void
synth_crossing (GdkWindow *child, gint x, gint y, gint x_root, gint y_root,
		guint32 time, gboolean in)
{
	GdkEventCrossing *crossing_event;
	GdkEventType type = in ? GDK_ENTER_NOTIFY : GDK_LEAVE_NOTIFY;

	/* Send synthetic enter event */
	crossing_event = (GdkEventCrossing *)gdk_event_new (type);
	((GdkEventAny *)crossing_event)->type = type;
	((GdkEventAny *)crossing_event)->window = g_object_ref (child);
	((GdkEventAny *)crossing_event)->send_event = FALSE;
	crossing_event->subwindow = g_object_ref (child);
	crossing_event->time = time;
	crossing_event->x = x;
	crossing_event->y = y;
	crossing_event->x_root = x_root;
	crossing_event->y_root = y_root;
	crossing_event->mode = GDK_CROSSING_NORMAL;
	crossing_event->detail = GDK_NOTIFY_UNKNOWN;
	crossing_event->focus = FALSE;
	crossing_event->state = 0;
	gdk_event_put ((GdkEvent *)crossing_event);
	gdk_event_free ((GdkEvent *)crossing_event);
}

static gboolean
moko_finger_scroll_button_press_cb (MokoFingerScroll *scroll,
				    GdkEventButton *event,
				    gpointer user_data)
{
	gint x, y;

	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);

	if ((!priv->enabled) || (event->button != 1) ||
	    ((event->time == priv->last_time) &&
	     (event->type == priv->last_type))) return TRUE;

	if (priv->clicked && priv->child) {
		/* Widget stole focus on last click, send crossing-out event */
		synth_crossing (priv->child, 0, 0, event->x_root, event->y_root,
			event->time, FALSE);
	}
	
	g_get_current_time (&priv->click_start);
	priv->last_type = event->type;
	priv->last_time = event->time;
	priv->x = event->x;
	priv->y = event->y;
	priv->ix = priv->x;
	priv->iy = priv->y;
	/* Don't allow a click if we're still moving fast, where fast is
	 * defined as a quarter of our top possible speed.
	 * TODO: Make 'fast' configurable?
	 */
	if ((ABS (priv->vel_x) < (priv->vmax * 0.25)) &&
	    (ABS (priv->vel_y) < (priv->vmax * 0.25)))
		priv->child = moko_finger_scroll_get_topmost (
			GTK_BIN (priv->align)->child->window,
			event->x, event->y, &x, &y);
	else
		priv->child = NULL;

	priv->clicked = TRUE;
	/* Stop scrolling on mouse-down (so you can flick, then hold to stop) */
	priv->vel_x = 0;
	priv->vel_y = 0;
	
	if ((priv->child) && (priv->child != GTK_BIN (
	     priv->align)->child->window)) {
		
		g_object_add_weak_pointer ((GObject *)priv->child,
			&priv->child);
		
		event = (GdkEventButton *)gdk_event_copy ((GdkEvent *)event);
		event->x = x;
		event->y = y;
		priv->cx = x;
		priv->cy = y;

		synth_crossing (priv->child, x, y, event->x_root,
			event->y_root, event->time, TRUE);
	
		/* Send synthetic click (button press/release) event */
		((GdkEventAny *)event)->window = g_object_ref (priv->child);
		gdk_event_put ((GdkEvent *)event);
		gdk_event_free ((GdkEvent *)event);
	} else
		priv->child = NULL;

	return TRUE;
}

static void
moko_finger_scroll_redraw (MokoFingerScroll *scroll)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);

	/* Redraw scroll indicators */
	if (priv->hscroll) {
		if (GTK_WIDGET (scroll)->window) {
			gdk_window_invalidate_rect (GTK_WIDGET (scroll)->window,
				&priv->hscroll_rect, FALSE);
		}
	}
	if (priv->vscroll) {
		if (GTK_WIDGET (scroll)->window) {
			gdk_window_invalidate_rect (GTK_WIDGET (scroll)->window,
				&priv->vscroll_rect, FALSE);
		}
	}
}

static void
moko_finger_scroll_refresh (MokoFingerScroll *scroll)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	GtkAllocation *allocation = &GTK_WIDGET (scroll)->allocation;
	GtkWidget *widget = GTK_BIN (priv->align)->child;
	gboolean vscroll, hscroll;
	GtkRequisition req;
	guint border;
	
	if (!widget) return;
	
	/* Calculate if we need scroll indicators */
	border = gtk_container_get_border_width (GTK_CONTAINER (scroll));
	gtk_widget_size_request (widget, &req);
	if (req.width + (border * 2) > allocation->width) hscroll = TRUE;
	else hscroll = FALSE;
	if (req.height + (border * 2) > allocation->height) vscroll = TRUE;
	else vscroll = FALSE;
	
	/* TODO: Read ltr settings to decide which corner gets scroll
	 * indicators?
	 */
	if ((priv->vscroll != vscroll) || (priv->hscroll != hscroll)) {
		gtk_alignment_set_padding (GTK_ALIGNMENT (priv->align), 0,
			hscroll ? priv->scroll_width : 0, 0,
			vscroll ? priv->scroll_width : 0);
	}
	
	/* Store the vscroll/hscroll areas for redrawing */
	if (vscroll) {
		GtkAllocation *allocation = &GTK_WIDGET (scroll)->allocation;
		priv->vscroll_rect.x = allocation->x + allocation->width -
			priv->scroll_width;
		priv->vscroll_rect.y = allocation->y;
		priv->vscroll_rect.width = priv->scroll_width;
		priv->vscroll_rect.height = allocation->height -
			(hscroll ? priv->scroll_width : 0);
	}
	if (hscroll) {
		GtkAllocation *allocation = &GTK_WIDGET (scroll)->allocation;
		priv->hscroll_rect.y = allocation->y + allocation->height -
			priv->scroll_width;
		priv->hscroll_rect.x = allocation->x;
		priv->hscroll_rect.height = priv->scroll_width;
		priv->hscroll_rect.width = allocation->width -
			(vscroll ? priv->scroll_width : 0);
	}
	
	priv->vscroll = vscroll;
	priv->hscroll = hscroll;
	
	moko_finger_scroll_redraw (scroll);
}

static void
moko_finger_scroll_scroll (MokoFingerScroll *scroll, gdouble x, gdouble y,
			   gboolean *sx, gboolean *sy)
{
	/* Scroll by a particular amount (in pixels). Optionally, return if
	 * the scroll on a particular axis was successful.
	 */
	gdouble h, v;
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	
	if (!GTK_BIN (priv->align)->child) return;
	
	if (priv->hadjust) {
		h = gtk_adjustment_get_value (priv->hadjust) - x;
		if (h > priv->hadjust->upper - priv->hadjust->page_size) {
			if (sx) *sx = FALSE;
			h = priv->hadjust->upper - priv->hadjust->page_size;
		} else if (h < priv->hadjust->lower) {
			if (sx) *sx = FALSE;
			h = priv->hadjust->lower;
		} else if (sx)
			*sx = TRUE;
		gtk_adjustment_set_value (priv->hadjust, h);
	}
	
	if (priv->vadjust) {
		v = gtk_adjustment_get_value (priv->vadjust) - y;
		if (v > priv->vadjust->upper - priv->vadjust->page_size) {
			if (sy) *sy = FALSE;
			v = priv->vadjust->upper - priv->vadjust->page_size;
		} else if (v < priv->vadjust->lower) {
			if (sy) *sy = FALSE;
			v = priv->vadjust->lower;
		} else if (sy)
			*sy = TRUE;
		gtk_adjustment_set_value (priv->vadjust, v);
	}

	moko_finger_scroll_redraw (scroll);
}

static gboolean
moko_finger_scroll_timeout (MokoFingerScroll *scroll)
{
	gboolean sx, sy;
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	
	if ((!priv->enabled) ||
	    (priv->mode != MOKO_FINGER_SCROLL_MODE_ACCEL)) {
		priv->idle_id = 0;
		return FALSE;
	}
	if (!priv->clicked) {
		/* Decelerate gradually when pointer is raised */
		priv->vel_x *= priv->decel;
		priv->vel_y *= priv->decel;
		if ((ABS (priv->vel_x) < 1.0) && (ABS (priv->vel_y) < 1.0)) {
			priv->idle_id = 0;
			return FALSE;
		}
	}
	
	moko_finger_scroll_scroll (scroll, priv->vel_x, priv->vel_y, &sx, &sy);
	/* If the scroll on a particular axis wasn't succesful, reset the
	 * initial scroll position to the new mouse co-ordinate. This means
	 * when you get to the top of the page, dragging down works immediately.
	 */
	if (!sx) priv->x = priv->ex;
	if (!sy) priv->y = priv->ey;
	
	return TRUE;
}

static gboolean
moko_finger_scroll_motion_notify_cb (MokoFingerScroll *scroll,
				     GdkEventMotion *event,
				     gpointer user_data)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	gint dnd_threshold;
	gdouble x, y;

	gdk_window_get_pointer (GTK_WIDGET (scroll)->window, NULL, NULL, 0);
	
	if ((!priv->enabled) || (!priv->clicked) ||
	    ((event->time == priv->last_time) &&
	     (event->type == priv->last_type))) return TRUE;
	
	/* Only start the scroll if the mouse cursor passes beyond the
	 * DnD threshold for dragging.
	 */
	g_object_get (G_OBJECT (gtk_settings_get_default ()),
		"gtk-dnd-drag-threshold", &dnd_threshold, NULL);
	x = event->x - priv->x;
	y = event->y - priv->y;
	
	if ((!priv->moved) && (
	     (ABS (x) > dnd_threshold) || (ABS (y) > dnd_threshold))) {
		priv->moved = TRUE;
		if (priv->mode == MOKO_FINGER_SCROLL_MODE_ACCEL) {
			priv->idle_id = g_timeout_add (
				(gint)(1000.0/(gdouble)priv->sps),
				(GSourceFunc)moko_finger_scroll_timeout,
				scroll);
		}
	}
	
	if (priv->moved) {
		switch (priv->mode) {
		    case MOKO_FINGER_SCROLL_MODE_PUSH :
			/* Scroll by the amount of pixels the cursor has moved
			 * since the last motion event.
			 */
			moko_finger_scroll_scroll (scroll, x, y, NULL, NULL);
			priv->x = event->x;
			priv->y = event->y;
			break;
		    case MOKO_FINGER_SCROLL_MODE_ACCEL :
			/* Set acceleration relative to the initial click */
			priv->ex = event->x;
			priv->ey = event->y;
			priv->vel_x = ((x > 0) ? 1 : -1) *
				(((ABS (x) /
				 (gdouble)GTK_WIDGET (scroll)->
				 allocation.width) *
				(priv->vmax-priv->vmin)) + priv->vmin);
			priv->vel_y = ((y > 0) ? 1 : -1) *
				(((ABS (y) /
				 (gdouble)GTK_WIDGET (scroll)->
				 allocation.height) *
				(priv->vmax-priv->vmin)) + priv->vmin);
			break;
		    default :
			break;
		}
	}
	
	if (priv->child) {
		/* Send motion notify to child */
		priv->last_type = event->type;
		priv->last_time = event->time;
		event = (GdkEventMotion *)gdk_event_copy ((GdkEvent *)event);
		event->x = priv->cx + (event->x - priv->ix);
		event->y = priv->cy + (event->y - priv->iy);
		event->window = g_object_ref (priv->child);
		gdk_event_put ((GdkEvent *)event);
		gdk_event_free ((GdkEvent *)event);
	}

	return TRUE;
}

static gboolean
moko_finger_scroll_button_release_cb (MokoFingerScroll *scroll,
				      GdkEventButton *event,
				      gpointer user_data)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	GTimeVal current;
	gint x, y;
	GdkWindow *child;
	
	if ((!priv->clicked) || (!priv->enabled) || (event->button != 1) ||
	    ((event->time == priv->last_time) &&
	     (event->type == priv->last_type)))
		return TRUE;

	priv->last_type = event->type;
	priv->last_time = event->time;
	g_get_current_time (&current);

	priv->clicked = FALSE;
		
	child = moko_finger_scroll_get_topmost (
		GTK_BIN (priv->align)->child->window,
		event->x, event->y, &x, &y);

	if (!priv->child) {
		priv->moved = FALSE;
		return TRUE;
	}
	
	event = (GdkEventButton *)gdk_event_copy ((GdkEvent *)event);
	event->x = x;
	event->y = y;

	/* Leave the widget if we've moved - This doesn't break selection,
	 * but stops buttons from being clicked.
	 */
	if ((child != priv->child) || (priv->moved)) {
		/* Send synthetic leave event */
		synth_crossing (priv->child, x, y, event->x_root,
			event->y_root, event->time, FALSE);
		/* Send synthetic button release event */
		((GdkEventAny *)event)->window = g_object_ref (priv->child);
		gdk_event_put ((GdkEvent *)event);
	} else {
		/* Send synthetic button release event */
		((GdkEventAny *)event)->window = g_object_ref (child);
		gdk_event_put ((GdkEvent *)event);
		/* Send synthetic leave event */
		synth_crossing (priv->child, x, y, event->x_root,
			event->y_root, event->time, FALSE);
	}
	g_object_remove_weak_pointer ((GObject *)priv->child,
		&priv->child);

	priv->moved = FALSE;
	gdk_event_free ((GdkEvent *)event);
	
	return TRUE;
}

static gboolean
moko_finger_scroll_expose_event (GtkWidget *widget, GdkEventExpose *event)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (widget);
	
	if (GTK_BIN (priv->align)->child) {
		if (priv->vscroll) {
			gint y, height;
			gdk_draw_rectangle (widget->window,
				widget->style->fg_gc[GTK_STATE_INSENSITIVE],
				TRUE,
				priv->vscroll_rect.x, priv->vscroll_rect.y,
				priv->vscroll_rect.width,
				priv->vscroll_rect.height);
			
			y = widget->allocation.y +
				((priv->vadjust->value/priv->vadjust->upper)*
				 (widget->allocation.height -
				  (priv->hscroll ? priv->scroll_width : 0)));
			height = (widget->allocation.y +
				(((priv->vadjust->value +
				   priv->vadjust->page_size)/
				  priv->vadjust->upper)*
				 (widget->allocation.height -
				  (priv->hscroll ? priv->scroll_width : 0)))) -
				  y;
			
			gdk_draw_rectangle (widget->window,
				widget->style->base_gc[GTK_STATE_SELECTED],
				TRUE, priv->vscroll_rect.x, y,
				priv->vscroll_rect.width, height);
		}
		
		if (priv->hscroll) {
			gint x, width;
			gdk_draw_rectangle (widget->window,
				widget->style->fg_gc[GTK_STATE_INSENSITIVE],
				TRUE,
				priv->hscroll_rect.x, priv->hscroll_rect.y,
				priv->hscroll_rect.width,
				priv->hscroll_rect.height);

			x = widget->allocation.x +
				((priv->hadjust->value/priv->hadjust->upper)*
				 (widget->allocation.width  -
				  (priv->vscroll ? priv->scroll_width : 0)));
			width = (widget->allocation.x +
				(((priv->hadjust->value +
				   priv->hadjust->page_size)/
				  priv->hadjust->upper)*
				 (widget->allocation.width -
				  (priv->vscroll ? priv->scroll_width : 0)))) -
				  x;

			gdk_draw_rectangle (widget->window,
				widget->style->base_gc[GTK_STATE_SELECTED],
				TRUE, x, priv->hscroll_rect.y, width,
				priv->hscroll_rect.height);
		}
	}
	
	return GTK_WIDGET_CLASS (
		moko_finger_scroll_parent_class)->expose_event (widget, event);
}

static void
moko_finger_scroll_destroy (GtkObject *object)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (object); 

	if (priv->hadjust) {
		g_object_unref (G_OBJECT (priv->hadjust));
		priv->hadjust = NULL;
	}

	if (priv->vadjust) {
		g_object_unref (G_OBJECT (priv->vadjust));
		priv->vadjust = NULL;
	}

	GTK_OBJECT_CLASS (moko_finger_scroll_parent_class)->destroy (object);
}

static void
moko_finger_scroll_remove_cb (GtkContainer *container,
			      GtkWidget    *child,
			      MokoFingerScroll *scroll)
{
	g_signal_handlers_disconnect_by_func (child,
		moko_finger_scroll_refresh, scroll);
	g_signal_handlers_disconnect_by_func (child,
		gtk_widget_queue_resize, scroll);
}

static void
moko_finger_scroll_add (GtkContainer *container,
			GtkWidget    *child)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (container);
	
	gtk_container_add (GTK_CONTAINER (priv->align), child);
	g_signal_connect_swapped (G_OBJECT (child), "size-allocate",
		G_CALLBACK (moko_finger_scroll_refresh), container);
	g_signal_connect_swapped (G_OBJECT (child), "size-request",
		G_CALLBACK (gtk_widget_queue_resize), container);

	if (!gtk_widget_set_scroll_adjustments (
	     child, priv->hadjust, priv->vadjust))
		g_warning("%s: cannot add non scrollable widget, "
			"wrap it in a viewport", __FUNCTION__);
}

static void
moko_finger_scroll_get_property (GObject * object, guint property_id,
				 GValue * value, GParamSpec * pspec)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (object);

	switch (property_id) {
	    case PROP_ENABLED :
		g_value_set_boolean (value, priv->enabled);
		break;
	    case PROP_MODE :
		g_value_set_int (value, priv->mode);
		break;
	    case PROP_VELOCITY_MIN :
		g_value_set_double (value, priv->vmin);
		break;
	    case PROP_VELOCITY_MAX :
		g_value_set_double (value, priv->vmax);
		break;
	    case PROP_DECELERATION :
		g_value_set_double (value, priv->decel);
		break;
	    case PROP_SPS :
		g_value_set_uint (value, priv->sps);
		break;
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_finger_scroll_set_property (GObject * object, guint property_id,
				 const GValue * value, GParamSpec * pspec)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (object);

	switch (property_id) {
	    case PROP_ENABLED :
		priv->enabled = g_value_get_boolean (value);
		gtk_event_box_set_above_child (
			GTK_EVENT_BOX (object), priv->enabled);
		break;
	    case PROP_MODE :
		priv->mode = g_value_get_int (value);
		break;
	    case PROP_VELOCITY_MIN :
		priv->vmin = g_value_get_double (value);
		break;
	    case PROP_VELOCITY_MAX :
		priv->vmax = g_value_get_double (value);
		break;
	    case PROP_DECELERATION :
		priv->decel = g_value_get_double (value);
		break;
	    case PROP_SPS :
		priv->sps = g_value_get_uint (value);
		break;
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_finger_scroll_dispose (GObject * object)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (object);
	
	if (priv->idle_id) {
		g_source_remove (priv->idle_id);
		priv->idle_id = 0;
	}
	
	if (G_OBJECT_CLASS (moko_finger_scroll_parent_class)->dispose)
		G_OBJECT_CLASS (moko_finger_scroll_parent_class)->
			dispose (object);
}

static void
moko_finger_scroll_finalize (GObject * object)
{
	G_OBJECT_CLASS (moko_finger_scroll_parent_class)->finalize (object);
}

static void
moko_finger_scroll_size_request (GtkWidget      *widget,
				 GtkRequisition *requisition)
{
	/* Request tiny size, seeing as we have no decoration of our own.
	 */
	requisition->width = 32;
	requisition->height = 32;
}

static void
moko_finger_scroll_style_set (GtkWidget *widget, GtkStyle *previous_style)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (widget);

	GTK_WIDGET_CLASS (moko_finger_scroll_parent_class)->
		style_set (widget, previous_style);
	
	gtk_widget_style_get (widget, "indicator-width", &priv->scroll_width,
		NULL);
}

static void
moko_finger_scroll_class_init (MokoFingerScrollClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

	g_type_class_add_private (klass, sizeof (MokoFingerScrollPrivate));

	object_class->get_property = moko_finger_scroll_get_property;
	object_class->set_property = moko_finger_scroll_set_property;
	object_class->dispose = moko_finger_scroll_dispose;
	object_class->finalize = moko_finger_scroll_finalize;

	gtkobject_class->destroy = moko_finger_scroll_destroy;
	
	widget_class->size_request = moko_finger_scroll_size_request;
	widget_class->expose_event = moko_finger_scroll_expose_event;
	widget_class->style_set = moko_finger_scroll_style_set;
	
	container_class->add = moko_finger_scroll_add;

	g_object_class_install_property (
		object_class,
		PROP_ENABLED,
		g_param_spec_boolean (
			"enabled",
			"Enabled",
			"Enable or disable finger-scroll.",
			TRUE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (
		object_class,
		PROP_MODE,
		g_param_spec_int (
			"mode",
			"Scroll mode",
			"Change the finger-scrolling mode.",
			MOKO_FINGER_SCROLL_MODE_PUSH,
			MOKO_FINGER_SCROLL_MODE_ACCEL,
			MOKO_FINGER_SCROLL_MODE_ACCEL,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (
		object_class,
		PROP_VELOCITY_MIN,
		g_param_spec_double (
			"velocity_min",
			"Minimum scroll velocity",
			"Minimum distance the child widget should scroll "
				"per 'frame', in pixels.",
			0, G_MAXDOUBLE, 0,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (
		object_class,
		PROP_VELOCITY_MAX,
		g_param_spec_double (
			"velocity_max",
			"Maximum scroll velocity",
			"Maximum distance the child widget should scroll "
				"per 'frame', in pixels.",
			0, G_MAXDOUBLE, 48,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (
		object_class,
		PROP_DECELERATION,
		g_param_spec_double (
			"deceleration",
			"Deceleration multiplier",
			"The multiplier used when decelerating when in "
				"acceleration scrolling mode.",
			0, 1.0, 0.95,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	
	g_object_class_install_property (
		object_class,
		PROP_SPS,
		g_param_spec_uint (
			"sps",
			"Scrolls per second",
			"Amount of scroll events to generate per second.",
			0, G_MAXUINT, 15,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	
	gtk_widget_class_install_style_property (
		widget_class,
		g_param_spec_uint (
			"indicator-width",
			"Width of the scroll indicators",
			"Pixel width used to draw the scroll indicators.",
			0, G_MAXUINT, 6,
			G_PARAM_READWRITE));
}

static void
moko_finger_scroll_init (MokoFingerScroll * self)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (self);
	
	priv->moved = FALSE;
	priv->clicked = FALSE;
	priv->last_time = 0;
	priv->vscroll = TRUE;
	priv->hscroll = TRUE;
	priv->scroll_width = 6;

	gtk_event_box_set_above_child (GTK_EVENT_BOX (self), TRUE);
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (self), FALSE);
	
	priv->align = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
	GTK_CONTAINER_CLASS (moko_finger_scroll_parent_class)->add (
		GTK_CONTAINER (self), priv->align);
	gtk_alignment_set_padding (GTK_ALIGNMENT (priv->align),
		0, priv->scroll_width, 0, priv->scroll_width);
	gtk_widget_show (priv->align);
	
	gtk_widget_add_events (GTK_WIDGET (self), GDK_POINTER_MOTION_HINT_MASK);

	priv->hadjust = GTK_ADJUSTMENT (
		gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
	priv->vadjust = GTK_ADJUSTMENT (
		gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));

	g_object_ref_sink (G_OBJECT (priv->hadjust));
	g_object_ref_sink (G_OBJECT (priv->vadjust));
	
	g_signal_connect (G_OBJECT (self), "button-press-event",
		G_CALLBACK (moko_finger_scroll_button_press_cb), NULL);
	g_signal_connect (G_OBJECT (self), "button-release-event",
		G_CALLBACK (moko_finger_scroll_button_release_cb), NULL);
	g_signal_connect (G_OBJECT (self), "motion-notify-event",
		G_CALLBACK (moko_finger_scroll_motion_notify_cb), NULL);
	g_signal_connect (G_OBJECT (priv->align), "remove",
		G_CALLBACK (moko_finger_scroll_remove_cb), self);

	g_signal_connect_swapped (G_OBJECT (priv->hadjust), "changed",
		G_CALLBACK (moko_finger_scroll_refresh), self);
	g_signal_connect_swapped (G_OBJECT (priv->vadjust), "changed",
		G_CALLBACK (moko_finger_scroll_refresh), self);
	g_signal_connect_swapped (G_OBJECT (priv->hadjust), "value-changed",
		G_CALLBACK (moko_finger_scroll_redraw), self);
	g_signal_connect_swapped (G_OBJECT (priv->vadjust), "value-changed",
		G_CALLBACK (moko_finger_scroll_redraw), self);
}

/**
 * moko_finger_scroll_new:
 * 
 * Create a new finger scroll widget
 *
 * Returns: the newly created #MokoFingerScroll
 */

GtkWidget *
moko_finger_scroll_new (void)
{
	return g_object_new (MOKO_TYPE_FINGER_SCROLL, NULL);
}

/**
 * moko_finger_scroll_new_full:
 * @mode: #MokoFingerScrollMode
 * @enabled: Value for the enabled property
 * @vel_min: Value for the velocity-min property
 * @vel_max: Value for the velocity-max property
 * @decel: Value for the deceleration property
 * @sps: Value for the sps property
 *
 * Create a new #MokoFingerScroll widget and set various properties
 *
 * returns: the newly create #MokoFingerScrull
 */

GtkWidget *
moko_finger_scroll_new_full (gint mode, gboolean enabled,
			     gdouble vel_min, gdouble vel_max,
			     gdouble decel, guint sps)
{
	return g_object_new (MOKO_TYPE_FINGER_SCROLL,
			     "mode", mode,
			     "enabled", enabled,
			     "velocity_min", vel_min,
			     "velocity_max", vel_max,
			     "deceleration", decel,
			     "sps", sps,
			     NULL);
}

/**
 * moko_finger_scroll_add_with_viewport:
 * @scroll: A #MokoFingerScroll
 * @child: Child widget to add to the viewport
 *
 * Convenience function used to add a child to a #GtkViewport, and add the
 * viewport to the scrolled window.
 */

void
moko_finger_scroll_add_with_viewport (MokoFingerScroll *scroll,
				      GtkWidget *child)
{
	GtkWidget *viewport = gtk_viewport_new (NULL, NULL);
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (viewport), child);
	gtk_widget_show (viewport);
	gtk_container_add (GTK_CONTAINER (scroll), viewport);
}

