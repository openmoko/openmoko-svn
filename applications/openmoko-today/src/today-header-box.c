
#include <math.h>
#include <gtk/gtk.h>
#include "today-header-box.h"

G_DEFINE_TYPE (TodayHeaderBox, today_header_box, GTK_TYPE_VBOX)

#define HEADER_BOX_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TODAY_TYPE_HEADER_BOX, TodayHeaderBoxPrivate))

typedef struct _TodayHeaderBoxPrivate TodayHeaderBoxPrivate;

struct _TodayHeaderBoxPrivate
{
	GtkWidget *ebox;
	gchar *markup;
	PangoLayout *layout;
	guint padding;
	guint curve_radius;
	GdkColor dark_mid;
	GdkColor dark_dark;
};

enum {
	PROP_MARKUP = 1,
	PROP_PADDING,
	PROP_RADIUS,
};

static GdkColor
today_header_box_pixel_blend (GdkColor c, GdkColor c2, gfloat weight)
{
	GdkColor c3;

	c3.pixel = 0;
	c3.red = (guint16)((weight * (gfloat)c.red) +
		((1 - weight) * (gfloat)c2.red));
	c3.green = (guint16)((weight * (gfloat)c.green) +
		((1 - weight) * (gfloat)c2.green));
	c3.blue = (guint16)((weight * (gfloat)c.blue) +
		((1 - weight) * (gfloat)c2.blue));

	return c3;
}

static gboolean
today_header_box_expose_cb (GtkWidget *widget, GdkEventExpose *event,
			    TodayHeaderBox *header_box)
{
	gint width, height, line;
	GdkGC *gc;
	TodayHeaderBoxPrivate *priv = HEADER_BOX_PRIVATE (header_box);

	if (!priv->layout) return FALSE;
	
	gc = gdk_gc_new (widget->window);
	gdk_drawable_get_size (GDK_DRAWABLE (widget->window), &width, &height);
	for (line = 0; line < height; line++) {
		gint offset;
		GdkColor color;
		
		if (line <= priv->curve_radius) {
			offset = priv->curve_radius - (gint)sqrt (
				pow (priv->curve_radius,2) -
				pow (priv->curve_radius - line, 2));
			color = today_header_box_pixel_blend (
				priv->dark_mid,
				widget->style->mid[GTK_STATE_NORMAL],
				(gfloat)line/priv->curve_radius);
		} else {
			offset = 0;
			color = today_header_box_pixel_blend (
				priv->dark_dark,
				widget->style->dark[GTK_STATE_NORMAL],
				(gfloat)(line-priv->curve_radius)/
					(height-priv->curve_radius));
		}
		
		gdk_colormap_alloc_color (gtk_widget_get_colormap (widget),
			&color, FALSE, TRUE);

		gdk_gc_set_foreground (gc, &color);
		gdk_draw_line (widget->window, gc, offset - 1, line,
			width - offset + 1, line);
	}
	gdk_gc_set_foreground (gc, &widget->style->light[GTK_STATE_NORMAL]);
	gdk_draw_layout (widget->window, gc, priv->padding, priv->padding,
		priv->layout);

	return FALSE;
}

static void
today_header_box_layout_refresh (TodayHeaderBox *header_box)
{
	TodayHeaderBoxPrivate *priv = HEADER_BOX_PRIVATE (header_box);

	if (priv->layout) {
		g_object_unref (priv->layout);
		priv->layout = NULL;
	}

	if (priv->markup) {
		gint width, height;
		priv->layout = gtk_widget_create_pango_layout (
			priv->ebox, NULL);
		pango_layout_set_markup (priv->layout, priv->markup, -1);
		pango_layout_get_pixel_size (priv->layout, &width, &height);
		gtk_widget_set_size_request (priv->ebox,
			-1, height + (priv->padding * 2));
		pango_layout_set_ellipsize (priv->layout, PANGO_ELLIPSIZE_END);
	}
	
	gtk_widget_queue_draw (priv->ebox);
}

static void
today_header_box_style_set_cb (GtkWidget *widget, GtkStyle *prev,
							   gpointer user_data)
{
	TodayHeaderBoxPrivate *priv = HEADER_BOX_PRIVATE (user_data);

	/* Force style to be allocated (required on first fire of signal) */
	gtk_widget_realize (widget);
	
	priv->dark_mid = today_header_box_pixel_blend (
		widget->style->mid[GTK_STATE_NORMAL],
		widget->style->dark[GTK_STATE_NORMAL], 0.25);
	priv->dark_dark = today_header_box_pixel_blend (
		widget->style->dark[GTK_STATE_NORMAL],
		widget->style->black, 0.25);
	gdk_colormap_alloc_color (gtk_widget_get_colormap (widget),
		&priv->dark_mid, FALSE, TRUE);
	gdk_colormap_alloc_color (gtk_widget_get_colormap (widget),
		&priv->dark_dark, FALSE, TRUE);
	
	today_header_box_layout_refresh (TODAY_HEADER_BOX (user_data));
}

static void
today_header_box_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
	TodayHeaderBoxPrivate *priv = HEADER_BOX_PRIVATE (object);

	switch (property_id) {
	    case PROP_MARKUP :
		g_value_set_string (value, priv->markup);
		break;
	    case PROP_PADDING :
		g_value_set_uint (value, priv->padding);
		break;
	    case PROP_RADIUS :
		g_value_set_uint (value, priv->curve_radius);
		break;
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
today_header_box_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
	TodayHeaderBoxPrivate *priv = HEADER_BOX_PRIVATE (object);

	switch (property_id) {
	    case PROP_MARKUP : {
		const gchar *markup = g_value_get_string (value);
		if (priv->markup) {
			g_free (priv->markup);
			priv->markup = NULL;
		}
		if (markup) {
			priv->markup = g_strdup (markup);
		}
		today_header_box_layout_refresh (TODAY_HEADER_BOX (object));
		break;
	    }
	    case PROP_PADDING :
		priv->padding = g_value_get_uint (value);
		if (priv->layout) {
			gint width, height;
			pango_layout_get_pixel_size (
				priv->layout, &width, &height);
			gtk_widget_set_size_request (priv->ebox,
				-1, height + (priv->padding * 2));
		}
		gtk_widget_queue_draw (priv->ebox);
		break;
	    case PROP_RADIUS :
		priv->curve_radius = g_value_get_uint (value);
		gtk_widget_queue_draw (priv->ebox);
		break;
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
today_header_box_dispose (GObject *object)
{
	if (G_OBJECT_CLASS (today_header_box_parent_class)->dispose)
		G_OBJECT_CLASS (today_header_box_parent_class)->dispose (
			object);
}

static void
today_header_box_finalize (GObject *object)
{
	TodayHeaderBoxPrivate *priv = HEADER_BOX_PRIVATE (object);
	
	g_free (priv->markup);
	
	G_OBJECT_CLASS (today_header_box_parent_class)->finalize (object);
}

static void
today_header_box_class_init (TodayHeaderBoxClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (TodayHeaderBoxPrivate));

	object_class->get_property = today_header_box_get_property;
	object_class->set_property = today_header_box_set_property;
	object_class->dispose = today_header_box_dispose;
	object_class->finalize = today_header_box_finalize;

	g_object_class_install_property (
		object_class,
		PROP_MARKUP,
		g_param_spec_string (
			"markup",
			"Header label markup",
			"Pango markup to use when drawing the header label.",
			"<span size=\"large\"><b>Header</b></span>",
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (
		object_class,
		PROP_PADDING,
		g_param_spec_uint (
			"padding",
			"Header padding",
			"Padding to put around the header label, in pixels.",
			0, G_MAXUINT, 6,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (
		object_class,
		PROP_RADIUS,
		g_param_spec_uint (
			"curve_radius",
			"Header curve radius",
			"Radius of the curves on the header, in pixels.",
			0, G_MAXUINT, 12,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
today_header_box_init (TodayHeaderBox *self)
{
	TodayHeaderBoxPrivate *priv = HEADER_BOX_PRIVATE (self);
	
	priv->markup = NULL;
	priv->layout = NULL;
	priv->ebox = gtk_event_box_new ();
	gtk_box_pack_start (GTK_BOX (self), priv->ebox, FALSE, TRUE, 0);
	gtk_widget_set_app_paintable (priv->ebox, TRUE);
	gtk_widget_show (priv->ebox);
	
	g_signal_connect (G_OBJECT (priv->ebox), "expose-event",
			  G_CALLBACK (today_header_box_expose_cb), self);
	g_signal_connect (G_OBJECT (priv->ebox), "style-set",
			  G_CALLBACK (today_header_box_style_set_cb), self);
}

GtkWidget *
today_header_box_new (void)
{
	return GTK_WIDGET (g_object_new (TODAY_TYPE_HEADER_BOX, NULL));
}

GtkWidget *
today_header_box_new_with_markup (const gchar *markup)
{
	return GTK_WIDGET (g_object_new (
		TODAY_TYPE_HEADER_BOX, "markup", markup, NULL));
}
