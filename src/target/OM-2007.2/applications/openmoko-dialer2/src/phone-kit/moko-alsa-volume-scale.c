
#include "moko-alsa-volume-scale.h"

G_DEFINE_TYPE (MokoAlsaVolumeScale, moko_alsa_volume_scale, \
	GTK_TYPE_EVENT_BOX)

#define ALSA_VOLUME_SCALE_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_ALSA_VOLUME_SCALE, \
	 MokoAlsaVolumeScalePrivate))

typedef struct _MokoAlsaVolumeScalePrivate MokoAlsaVolumeScalePrivate;

struct _MokoAlsaVolumeScalePrivate {
	MokoAlsaVolumeControl *control;
	GtkWidget             *scale;
};

enum {
	PROP_CONTROL = 1,
	PROP_ORIENTATION,
};

static void
scale_value_changed_cb (GtkRange *range, MokoAlsaVolumeScale *scale)
{
	gdouble value = gtk_range_get_value (range);
	MokoAlsaVolumeScalePrivate *priv = ALSA_VOLUME_SCALE_PRIVATE (scale);
	
	moko_alsa_volume_control_set_volume (priv->control, value);
}


static void
create_widgets (MokoAlsaVolumeScale *scale, GtkOrientation orientation)
{
	GtkBox *box;
	GtkWidget *image;
	
	MokoAlsaVolumeScalePrivate *priv = ALSA_VOLUME_SCALE_PRIVATE (scale);
	
	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		box = GTK_BOX (gtk_hbox_new (FALSE, 6));
	else box = GTK_BOX (gtk_vbox_new (FALSE, 6));
	
	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		image = gtk_image_new_from_icon_name ("moko-volume-down",
			GTK_ICON_SIZE_SMALL_TOOLBAR);
	else image = gtk_image_new_from_icon_name ("moko-volume-up",
		GTK_ICON_SIZE_SMALL_TOOLBAR);
	
	gtk_box_pack_start (box, image, FALSE, TRUE, 0);
	
	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		image = gtk_image_new_from_icon_name ("moko-volume-up",
			GTK_ICON_SIZE_SMALL_TOOLBAR);
	else image = gtk_image_new_from_icon_name ("moko-volume-down",
		GTK_ICON_SIZE_SMALL_TOOLBAR);
	
	gtk_box_pack_end (box, image, FALSE, TRUE, 0);
	
	if (orientation == GTK_ORIENTATION_HORIZONTAL) {
		priv->scale = gtk_hscale_new_with_range (0.0, 1.0, 0.02);
	} else {
		priv->scale = gtk_vscale_new_with_range (0.0, 1.0, 0.02);
		gtk_range_set_inverted (GTK_RANGE (priv->scale), TRUE);
	}
	gtk_scale_set_draw_value (GTK_SCALE (priv->scale), FALSE);

	gtk_box_pack_start (box, priv->scale, TRUE, TRUE, 0);
	
	g_signal_connect (priv->scale, "value-changed",
		G_CALLBACK (scale_value_changed_cb), scale);
	
	gtk_container_add (GTK_CONTAINER (scale), GTK_WIDGET (box));
	gtk_widget_show_all (GTK_WIDGET (box));
	gtk_widget_set_no_show_all (GTK_WIDGET (box), TRUE);
}

static void
moko_alsa_volume_scale_get_property (GObject *object, guint property_id,
				     GValue *value, GParamSpec *pspec)
{
	MokoAlsaVolumeScalePrivate *priv = ALSA_VOLUME_SCALE_PRIVATE (object);
	
	switch (property_id) {
	    case PROP_CONTROL :
		g_value_set_object (value, priv->control);
		break;
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_alsa_volume_scale_set_property (GObject *object, guint property_id,
				     const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	    case PROP_CONTROL :
		moko_alsa_volume_scale_set_control (
			MOKO_ALSA_VOLUME_SCALE (object),
			g_value_get_object (value));
		break;
	    case PROP_ORIENTATION :
		create_widgets (MOKO_ALSA_VOLUME_SCALE (object),
			g_value_get_enum (value));
		break;
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_alsa_volume_scale_finalize (GObject *object)
{
	MokoAlsaVolumeScale *scale = MOKO_ALSA_VOLUME_SCALE (object);
	MokoAlsaVolumeScalePrivate *priv = ALSA_VOLUME_SCALE_PRIVATE (scale);
	
	if (priv->control) g_object_unref (priv->control);
	
	G_OBJECT_CLASS (moko_alsa_volume_scale_parent_class)->
		finalize (object);
}

static void
moko_alsa_volume_scale_class_init (MokoAlsaVolumeScaleClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (MokoAlsaVolumeScalePrivate));

	object_class->get_property = moko_alsa_volume_scale_get_property;
	object_class->set_property = moko_alsa_volume_scale_set_property;
	object_class->finalize = moko_alsa_volume_scale_finalize;

	g_object_class_install_property (
		object_class,
		PROP_CONTROL,
		g_param_spec_object (
			"control",
			"MokoAlsaVolumeControl",
			"The volume control object to hook onto.",
			MOKO_TYPE_ALSA_VOLUME_CONTROL,
			G_PARAM_READWRITE));

	g_object_class_install_property (
		object_class,
		PROP_ORIENTATION,
		g_param_spec_enum (
			"orientation",
			"GtkOrientation",
			"The orientation of the scale.",
			GTK_TYPE_ORIENTATION,
			GTK_ORIENTATION_HORIZONTAL,
			G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
moko_alsa_volume_scale_init (MokoAlsaVolumeScale *self)
{
}

GtkWidget *
moko_alsa_volume_scale_new (GtkOrientation orientation)
{
	return GTK_WIDGET (g_object_new (MOKO_TYPE_ALSA_VOLUME_SCALE,
		"orientation", orientation, NULL));
}

static void
volume_changed_cb (MokoAlsaVolumeControl *control, gdouble volume,
		   MokoAlsaVolumeScale *scale)
{
	MokoAlsaVolumeScalePrivate *priv = ALSA_VOLUME_SCALE_PRIVATE (scale);
	gtk_range_set_value (GTK_RANGE (priv->scale), volume);
}

void
moko_alsa_volume_scale_set_control (MokoAlsaVolumeScale *scale,
				     MokoAlsaVolumeControl *control)
{
	MokoAlsaVolumeScalePrivate *priv = ALSA_VOLUME_SCALE_PRIVATE (scale);
	
	if (priv->control) {
		g_signal_handlers_disconnect_by_func (priv->control,
			volume_changed_cb, scale);
		g_object_unref (priv->control);
		priv->control = NULL;
	}
	
	if (control) {
		priv->control = g_object_ref (control);
		g_signal_connect (priv->control, "volume_changed",
			G_CALLBACK (volume_changed_cb), scale);
		gtk_range_set_value (GTK_RANGE (priv->scale),
			moko_alsa_volume_control_get_volume (control));
	}
}

MokoAlsaVolumeControl *
moko_alsa_volume_scale_get_control (MokoAlsaVolumeScale *scale)
{
	MokoAlsaVolumeScalePrivate *priv = ALSA_VOLUME_SCALE_PRIVATE (scale);
	
	return priv->control;
}

