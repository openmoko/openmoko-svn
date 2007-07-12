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

#include "moko-finger-scroll.h"

G_DEFINE_TYPE (MokoFingerScroll, moko_finger_scroll, GTK_TYPE_EVENT_BOX)
#define FINGER_SCROLL_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_SCROLL, MokoFingerScrollPrivate))
typedef struct _MokoFingerScrollPrivate MokoFingerScrollPrivate;

#define SCROLL_WIDTH 6

struct _MokoFingerScrollPrivate {
	MokoFingerScrollMode mode;
	gdouble x;
	gdouble y;
	gdouble ex;
	gdouble ey;
	gboolean enabled;
	gboolean clicked;
	gboolean moved;
	GTimeVal click_start;
	gdouble vmin;
	gdouble vmax;
	gdouble decel;
	guint sps;
	gdouble vel_x;
	gdouble vel_y;
	
	GtkWidget *align;
	gboolean hscroll;
	gboolean vscroll;
	GdkRectangle hscroll_rect;
	GdkRectangle vscroll_rect;
};

enum {
	PROP_ENABLED = 1,
	PROP_MODE,
	PROP_VELOCITY_MIN,
	PROP_VELOCITY_MAX,
	PROP_DECELERATION,
	PROP_SPS,
};

static gboolean
moko_finger_scroll_button_press_cb (MokoFingerScroll *scroll,
				    GdkEventButton *event,
				    gpointer user_data)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);

	if ((!priv->enabled) || (priv->clicked) || (event->button != 1))
		return FALSE;

	g_get_current_time (&priv->click_start);
	priv->x = event->x;
	priv->y = event->y;
	/* Don't allow a click if we're still moving fast, where fast is
	 * defined as a quarter of our top possible speed.
	 * TODO: Make 'fast' configurable?
	 */
	if ((ABS (priv->vel_x) < (priv->vmax * 0.25)) &&
	    (ABS (priv->vel_y) < (priv->vmax * 0.25)))
		priv->moved = FALSE;
	priv->clicked = TRUE;
	/* Stop scrolling on mouse-down (so you can flick, then hold to stop) */
	priv->vel_x = 0;
	priv->vel_y = 0;
	
	return TRUE;
}

static void
moko_finger_scroll_scroll (MokoFingerScroll *scroll, gdouble x, gdouble y,
			   gboolean *sx, gboolean *sy)
{
	/* Scroll by a particular amount (in pixels). Optionally, return if
	 * the scroll on a particular axis was successful.
	 */
	gdouble h, v;
	GtkAdjustment *hadjust, *vadjust;
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	
	if (!GTK_BIN (priv->align)->child) return;
	
	g_object_get (G_OBJECT (GTK_BIN (priv->align)->child), "hadjustment",
		&hadjust, "vadjustment", &vadjust, NULL);
		
	if (hadjust) {
		h = gtk_adjustment_get_value (hadjust) - x;
		if (h > hadjust->upper - hadjust->page_size) {
			if (sx) *sx = FALSE;
			h = hadjust->upper - hadjust->page_size;
		} else if (h < hadjust->lower) {
			if (sx) *sx = FALSE;
			h = hadjust->lower;
		} else if (sx)
			*sx = TRUE;
		gtk_adjustment_set_value (hadjust, h);
	}
	
	if (vadjust) {
		v = gtk_adjustment_get_value (vadjust) - y;
		if (v > vadjust->upper - vadjust->page_size) {
			if (sy) *sy = FALSE;
			v = vadjust->upper - vadjust->page_size;
		} else if (v < vadjust->lower) {
			if (sy) *sy = FALSE;
			v = vadjust->lower;
		} else if (sy)
			*sy = TRUE;
		gtk_adjustment_set_value (vadjust, v);
	}

	/* Redraw scroll indicators */
	if (priv->hscroll) {
		gdk_window_invalidate_rect (GTK_WIDGET (scroll)->window,
			&priv->hscroll_rect, FALSE);
	}
	if (priv->vscroll) {
		gdk_window_invalidate_rect (GTK_WIDGET (scroll)->window,
			&priv->vscroll_rect, FALSE);
	}
}

static gboolean
moko_finger_scroll_timeout (MokoFingerScroll *scroll)
{
	gboolean sx, sy;
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	
	if ((!priv->enabled) ||
	    (priv->mode != MOKO_FINGER_SCROLL_MODE_ACCEL)) return FALSE;
	if (!priv->clicked) {
		/* Decelerate gradually when pointer is raised */
		priv->vel_x *= priv->decel;
		priv->vel_y *= priv->decel;
		if ((ABS (priv->vel_x) < 1.0) && (ABS (priv->vel_y) < 1.0))
			return FALSE;
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

	if ((!priv->enabled) || (!priv->clicked)) return FALSE;

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
			g_timeout_add ((gint)(1000.0/(gdouble)priv->sps),
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
	
	gdk_window_get_pointer (GTK_WIDGET (scroll)->window, NULL, NULL, 0);
	
	return TRUE;
}

static GdkWindow *
moko_finger_scroll_get_topmost (GdkWindow *window, gint x, gint y,
				gint *x2, gint *y2)
{
	/* Find the GdkWindow at the given point, by recursing from a given
	 * parent GdkWindow. Optionally return the co-ordinates transformed
	 * relative to the child window.
	 */
	while (window) {
		GList *c, *children = gdk_window_peek_children (window);
		GdkWindow *old_window = window;
		for (c = children; c; c = c->next) {
			GdkWindow *child = (GdkWindow *)c->data;
			gint width, height, wx, wy;
			
			gdk_drawable_get_size (child, &width, &height);
			gdk_window_get_position (child, &wx, &wy);
			
			if ((x >= wx) && (x < (wx + width)) &&
			    (y >= wy) && (y < (wy + height))) {
				x -= wx; y -= wy;
				window = child;
				break;
			}
		}
		if (window == old_window) break;
	}
	
	if (x2) *x2 = x;
	if (y2) *y2 = y;

	return window;
}

static gboolean
moko_finger_scroll_button_release_cb (MokoFingerScroll *scroll,
				      GdkEventButton *event,
				      gpointer user_data)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	GTimeVal current;
		
	if ((!priv->clicked) || (!priv->enabled) || (event->button != 1))
		return FALSE;

	g_get_current_time (&current);

	priv->clicked = FALSE;
	if ((!priv->moved) &&
	    ((current.tv_sec > priv->click_start.tv_sec) ||
	    (current.tv_usec - priv->click_start.tv_usec > 500))) {
		GtkAdjustment *hadjust, *vadjust;
		gint x, y;
		GdkWindow *child;
		
		g_object_get (G_OBJECT (GTK_BIN (priv->align)->child), "hadjustment",
			&hadjust, "vadjustment", &vadjust, NULL);

		child = moko_finger_scroll_get_topmost (
			GTK_BIN (priv->align)->child->window,
			event->x, event->y, &x, &y);

		event->x = x;
		event->y = y;
		
		/* Set velocity to zero, most widgets don't expect to be
		 * moving while being clicked.
		 */
		priv->vel_x = 0;
		priv->vel_y = 0;

		/* Send synthetic click event */
		((GdkEvent *)event)->any.window = g_object_ref (child);
		((GdkEvent *)event)->type = GDK_BUTTON_PRESS;
		gdk_event_put ((GdkEvent *)event);
		((GdkEvent *)event)->any.window = g_object_ref (child);
		((GdkEvent *)event)->type = GDK_BUTTON_RELEASE;
		gdk_event_put ((GdkEvent *)event);
	}

	return TRUE;
}

static void
moko_finger_scroll_size_allocate_cb (GtkWidget *widget,
				     GtkAllocation *allocation,
				     MokoFingerScroll *scroll)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	GtkRequisition req;
	gboolean vscroll, hscroll;
	
	/* Decide if we need scroll indicators */	
	gtk_widget_size_request (widget, &req);
	if (req.width > allocation->width) hscroll = TRUE;
	else hscroll = FALSE;
	if (req.height > allocation->height) vscroll = TRUE;
	else vscroll = FALSE;
	
	/* TODO: Read ltr settings to decide which corner gets scroll
	 * indicators?
	 */
	if ((priv->vscroll != vscroll) || (priv->hscroll != hscroll)) {
		gtk_alignment_set_padding (GTK_ALIGNMENT (priv->align), 0,
			hscroll ? SCROLL_WIDTH : 0, 0,
			vscroll ? SCROLL_WIDTH : 0);
	}
	
	/* Store the vscroll/hscroll areas for redrawing */
	if (vscroll) {
		GtkAllocation *allocation = &GTK_WIDGET (scroll)->allocation;
		priv->vscroll_rect.x = allocation->x + allocation->width -
			SCROLL_WIDTH;
		priv->vscroll_rect.y = allocation->y;
		priv->vscroll_rect.width = SCROLL_WIDTH;
		priv->vscroll_rect.height = allocation->height -
			(hscroll ? SCROLL_WIDTH : 0);
	}
	if (hscroll) {
		GtkAllocation *allocation = &GTK_WIDGET (scroll)->allocation;
		priv->hscroll_rect.y = allocation->y + allocation->height -
			SCROLL_WIDTH;
		priv->hscroll_rect.x = allocation->x;
		priv->hscroll_rect.height = SCROLL_WIDTH;
		priv->hscroll_rect.width = allocation->width -
			(vscroll ? SCROLL_WIDTH : 0);
	}
	
	priv->vscroll = vscroll;
	priv->hscroll = hscroll;
}

static gboolean
moko_finger_scroll_expose_event (GtkWidget *widget, GdkEventExpose *event)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (widget);
	
	if (GTK_BIN (priv->align)->child) {
		if (priv->vscroll) {
			gint y, height;
			GtkAdjustment *vadjust;
			g_object_get (G_OBJECT (GTK_BIN (priv->align)->child),
				"vadjustment", &vadjust, NULL);

			gdk_draw_rectangle (widget->window,
				widget->style->fg_gc[GTK_STATE_INSENSITIVE],
				TRUE,
				priv->vscroll_rect.x, priv->vscroll_rect.y,
				priv->vscroll_rect.width,
				priv->vscroll_rect.height);
			
			y = widget->allocation.y +
				((vadjust->value/vadjust->upper)*
				 (widget->allocation.height -
				  (priv->hscroll ? SCROLL_WIDTH : 0)));
			height = (widget->allocation.y +
				(((vadjust->value + vadjust->page_size)/
				  vadjust->upper)*
				 (widget->allocation.height -
				  (priv->hscroll ? SCROLL_WIDTH : 0)))) - y;
			
			gdk_draw_rectangle (widget->window,
				widget->style->base_gc[GTK_STATE_SELECTED],
				TRUE, priv->vscroll_rect.x, y,
				priv->vscroll_rect.width, height);
		}
		
		if (priv->hscroll) {
			gint x, width;
			GtkAdjustment *hadjust;
			g_object_get (G_OBJECT (GTK_BIN (priv->align)->child),
				"hadjustment", &hadjust, NULL);

			gdk_draw_rectangle (widget->window,
				widget->style->fg_gc[GTK_STATE_INSENSITIVE],
				TRUE,
				priv->hscroll_rect.x, priv->hscroll_rect.y,
				priv->hscroll_rect.width,
				priv->hscroll_rect.height);

			x = widget->allocation.x +
				((hadjust->value/hadjust->upper)*
				 (widget->allocation.width  -
				  (priv->vscroll ? SCROLL_WIDTH : 0)));
			width = (widget->allocation.x +
				(((hadjust->value + hadjust->page_size)/
				  hadjust->upper)*
				 (widget->allocation.width -
				  (priv->vscroll ? SCROLL_WIDTH : 0)))) - x;

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
moko_finger_scroll_add (GtkContainer *container,
			GtkWidget    *child)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (container);
	gtk_container_add (GTK_CONTAINER (priv->align), child);
	g_signal_connect (G_OBJECT (child), "size-allocate",
		G_CALLBACK (moko_finger_scroll_size_allocate_cb), container);
}

static void
moko_finger_scroll_remove (GtkContainer *container,
			   GtkWidget    *child)
{
	GTK_CONTAINER_CLASS (moko_finger_scroll_parent_class)->
		remove (container, child);
	g_signal_handlers_disconnect_by_func (child,
		moko_finger_scroll_size_allocate_cb, container);
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
moko_finger_scroll_class_init (MokoFingerScrollClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

	g_type_class_add_private (klass, sizeof (MokoFingerScrollPrivate));

	object_class->get_property = moko_finger_scroll_get_property;
	object_class->set_property = moko_finger_scroll_set_property;
	object_class->dispose = moko_finger_scroll_dispose;
	object_class->finalize = moko_finger_scroll_finalize;
	
	widget_class->size_request = moko_finger_scroll_size_request;
	widget_class->expose_event = moko_finger_scroll_expose_event;
	
	container_class->add = moko_finger_scroll_add;
	container_class->remove = moko_finger_scroll_remove;

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
			"Minimum distance the child widget should scroll "
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
}

static void
moko_finger_scroll_init (MokoFingerScroll * self)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (self);
	
	priv->moved = FALSE;
	priv->clicked = FALSE;
	priv->vscroll = TRUE;
	priv->hscroll = TRUE;

	gtk_event_box_set_above_child (GTK_EVENT_BOX (self), TRUE);
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (self), FALSE);
	
	priv->align = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
	GTK_CONTAINER_CLASS (moko_finger_scroll_parent_class)->add (
		GTK_CONTAINER (self), priv->align);
	gtk_alignment_set_padding (GTK_ALIGNMENT (priv->align),
		0, SCROLL_WIDTH, 0, SCROLL_WIDTH);
	gtk_widget_show (priv->align);
	
	gtk_widget_add_events (GTK_WIDGET (self), GDK_POINTER_MOTION_HINT_MASK);
	
	g_signal_connect (G_OBJECT (self), "button-press-event",
		G_CALLBACK (moko_finger_scroll_button_press_cb), NULL);
	g_signal_connect (G_OBJECT (self), "button-release-event",
		G_CALLBACK (moko_finger_scroll_button_release_cb), NULL);
	g_signal_connect (G_OBJECT (self), "motion-notify-event",
		G_CALLBACK (moko_finger_scroll_motion_notify_cb), NULL);
}

GtkWidget *
moko_finger_scroll_new (void)
{
	return g_object_new (MOKO_TYPE_FINGER_SCROLL, NULL);
}

GtkWidget *
moko_finger_scroll_new_full (gint mode, gboolean enabled,
			     gdouble vel_min, gdouble vel_max,
			     gdouble decel, guint sps)
{
	return g_object_new (MOKO_TYPE_FINGER_SCROLL,
			     "enabled", enabled,
			     "velocity_min", vel_min,
			     "velocity_max", vel_max,
			     "deceleration", decel,
			     "sps", sps,
			     NULL);
}

