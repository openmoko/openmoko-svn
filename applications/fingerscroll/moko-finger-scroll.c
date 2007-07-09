
#include "moko-finger-scroll.h"

G_DEFINE_TYPE (MokoFingerScroll, moko_finger_scroll, GTK_TYPE_SCROLLED_WINDOW)
#define FINGER_SCROLL_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_SCROLL, MokoFingerScrollPrivate))
typedef struct _MokoFingerScrollPrivate MokoFingerScrollPrivate;

struct _MokoFingerScrollPrivate {
	gint x;
	gint y;
	gboolean clicked;
	gboolean moved;
	GTimeVal click_start;
};

enum {
	PROP_ENABLED = 1,
	PROP_MODE,
};

static gboolean
moko_finger_scroll_button_press_cb (GtkWidget *widget, GdkEventButton *event,
				    MokoFingerScroll *scroll)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);

	if (priv->clicked) return FALSE;

	if (event->button != 1) return TRUE;

	g_get_current_time (&priv->click_start);
	priv->x = (gint)event->x_root;
	priv->y = (gint)event->y_root;
	priv->moved = FALSE;
	priv->clicked = TRUE;
	
	return TRUE;
}

static gboolean
moko_finger_scroll_motion_notify_cb (GtkWidget *widget, GdkEventMotion *event,
				     MokoFingerScroll *scroll)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	gint dnd_threshold;

	if (!priv->clicked) return FALSE;

	g_object_get (G_OBJECT (gtk_settings_get_default ()),
		"gtk-dnd-drag-threshold", &dnd_threshold, 0);
	
	if ((abs ((gint)event->x_root - priv->x) > dnd_threshold) ||
	    (abs ((gint)event->y_root - priv->y) > dnd_threshold)) {
		priv->moved = TRUE;
	}
	
	if (priv->moved) {
		gdouble h, v;
		GtkAdjustment *hadjust = gtk_scrolled_window_get_hadjustment (
			GTK_SCROLLED_WINDOW (scroll));
		GtkAdjustment *vadjust = gtk_scrolled_window_get_vadjustment (
			GTK_SCROLLED_WINDOW (scroll));
		h = gtk_adjustment_get_value (hadjust) -
			((event->x_root - priv->x));
		v = gtk_adjustment_get_value (vadjust) -
			((event->y_root - priv->y));
		priv->x = event->x_root;
		priv->y = event->y_root;
		if ((h >= hadjust->lower) &&
		    (h <= (hadjust->upper - hadjust->page_size)))
			gtk_adjustment_set_value (hadjust, h);
		if ((v >= vadjust->lower) &&
		    (v <= (vadjust->upper - vadjust->page_size)))
			gtk_adjustment_set_value (vadjust, v);
	}
	
	return TRUE;
}

static gboolean
moko_finger_scroll_button_release_cb (GtkWidget *widget, GdkEventButton *event,
				      MokoFingerScroll *scroll)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	GTimeVal current;
		
	if (!priv->clicked) return FALSE;
	if (event->button != 1) return TRUE;

	g_get_current_time (&current);

	if ((!priv->moved) &&
	    ((current.tv_sec > priv->click_start.tv_sec) ||
	    (current.tv_usec - priv->click_start.tv_usec > 500))) {
		/* Send synthetic click event */
		event->type = GDK_BUTTON_PRESS;
		gtk_widget_event (widget, (GdkEvent *)event);
		priv->clicked = FALSE;
		event->type = GDK_BUTTON_RELEASE;
		gtk_widget_event (widget, (GdkEvent *)event);
	}	
	priv->clicked = FALSE;

	return TRUE;
}

static void
moko_finger_scroll_add (GtkContainer *container,
			GtkWidget    *child)
{
	GTK_CONTAINER_CLASS (moko_finger_scroll_parent_class)->add (
		container, child);

	g_signal_connect (G_OBJECT (child), "button-press-event",
		G_CALLBACK (moko_finger_scroll_button_press_cb), container);
	g_signal_connect (G_OBJECT (child), "button-release-event",
		G_CALLBACK (moko_finger_scroll_button_release_cb), container);
	g_signal_connect (G_OBJECT (child), "motion-notify-event",
		G_CALLBACK (moko_finger_scroll_motion_notify_cb), container);
}

static void
moko_finger_scroll_remove (GtkContainer *container,
			   GtkWidget    *child)
{
	GTK_CONTAINER_CLASS (moko_finger_scroll_parent_class)->remove (
		container, child);
	
	g_signal_handlers_disconnect_by_func (child, 
		moko_finger_scroll_button_press_cb, container);
	g_signal_handlers_disconnect_by_func (child, 
		moko_finger_scroll_button_release_cb, container);
	g_signal_handlers_disconnect_by_func (child, 
		moko_finger_scroll_motion_notify_cb, container);
}

static void
moko_finger_scroll_get_property (GObject * object, guint property_id,
				 GValue * value, GParamSpec * pspec)
{
	switch (property_id) {
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_finger_scroll_set_property (GObject * object, guint property_id,
				 const GValue * value, GParamSpec * pspec)
{
	switch (property_id) {
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
moko_finger_scroll_class_init (MokoFingerScrollClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

	g_type_class_add_private (klass, sizeof (MokoFingerScrollPrivate));

	object_class->get_property = moko_finger_scroll_get_property;
	object_class->set_property = moko_finger_scroll_set_property;
	object_class->dispose = moko_finger_scroll_dispose;
	object_class->finalize = moko_finger_scroll_finalize;
	
	container_class->add = moko_finger_scroll_add;
	container_class->remove = moko_finger_scroll_remove;
}

static void
moko_finger_scroll_init (MokoFingerScroll * self)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (self);
	
	priv->x = 0;
	priv->y = 0;
	priv->moved = FALSE;
	priv->clicked = FALSE;	

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
}

GtkWidget *
moko_finger_scroll_new (void)
{
	return g_object_new (MOKO_TYPE_FINGER_SCROLL, NULL);
}
