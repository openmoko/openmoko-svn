
#include "moko-alsa-volume-button.h"

G_DEFINE_TYPE (MokoAlsaVolumeButton, moko_alsa_volume_button, \
	GTK_TYPE_SCALE_BUTTON)

#define ALSA_VOLUME_BUTTON_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_ALSA_VOLUME_BUTTON, \
	 MokoAlsaVolumeButtonPrivate))

typedef struct _MokoAlsaVolumeButtonPrivate MokoAlsaVolumeButtonPrivate;

struct _MokoAlsaVolumeButtonPrivate {
	MokoAlsaVolumeControl *control;
};

enum {
	PROP_CONTROL = 1,
};

static void
moko_alsa_volume_button_get_property (GObject *object, guint property_id,
				      GValue *value, GParamSpec *pspec)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (object);
	
	switch (property_id) {
	    case PROP_CONTROL :
		g_value_set_object (value, priv->control);
		break;
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_alsa_volume_button_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	    case PROP_CONTROL :
		moko_alsa_volume_button_set_control (
			MOKO_ALSA_VOLUME_BUTTON (object),
			g_value_get_object (value));
		break;
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_alsa_volume_button_finalize (GObject *object)
{
	MokoAlsaVolumeButton *button = MOKO_ALSA_VOLUME_BUTTON (object);
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (button);
	
	if (priv->control) g_object_unref (priv->control);
	
	G_OBJECT_CLASS (moko_alsa_volume_button_parent_class)->
		finalize (object);
}

static void
moko_alsa_volume_button_value_changed (GtkScaleButton *button, gdouble value)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (button);
	
	if (GTK_SCALE_BUTTON_CLASS (moko_alsa_volume_button_parent_class)->
	    value_changed)
		GTK_SCALE_BUTTON_CLASS (moko_alsa_volume_button_parent_class)->
			value_changed (button, value);
	
	moko_alsa_volume_control_set_volume (priv->control,
		gtk_scale_button_get_value (button) / 100.0);
}

static void
moko_alsa_volume_button_class_init (MokoAlsaVolumeButtonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkScaleButtonClass *button_class = GTK_SCALE_BUTTON_CLASS (klass);

	g_type_class_add_private (klass, sizeof (MokoAlsaVolumeButtonPrivate));

	object_class->get_property = moko_alsa_volume_button_get_property;
	object_class->set_property = moko_alsa_volume_button_set_property;
	object_class->finalize = moko_alsa_volume_button_finalize;
	
	button_class->value_changed = moko_alsa_volume_button_value_changed;

	g_object_class_install_property (
		object_class,
		PROP_CONTROL,
		g_param_spec_object (
			"control",
			"MokoAlsaVolumeControl",
			"The volume control object to hook onto.",
			MOKO_TYPE_ALSA_VOLUME_CONTROL,
			G_PARAM_READWRITE));
}

static void
moko_alsa_volume_button_init (MokoAlsaVolumeButton *self)
{
}

GtkWidget *
moko_alsa_volume_button_new (void)
{
	return GTK_WIDGET (g_object_new (MOKO_TYPE_ALSA_VOLUME_BUTTON, NULL));
}

static void
volume_changed_cb (MokoAlsaVolumeControl *control, gdouble volume,
		   MokoAlsaVolumeButton *button)
{
	gtk_scale_button_set_value (GTK_SCALE_BUTTON (button), volume * 100.0);
}

void
moko_alsa_volume_button_set_control (MokoAlsaVolumeButton *button,
				     MokoAlsaVolumeControl *control)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (button);
	
	if (priv->control) {
		g_signal_handlers_disconnect_by_func (priv->control,
			volume_changed_cb, button);
		g_object_unref (priv->control);
		priv->control = NULL;
	}
	
	if (control) {
		priv->control = g_object_ref (control);
		g_signal_connect (priv->control, "volume_changed",
			G_CALLBACK (volume_changed_cb), button);
		gtk_scale_button_set_value (GTK_SCALE_BUTTON (button),
			moko_alsa_volume_control_get_volume (control) * 100.0);
	}
}

MokoAlsaVolumeControl *
moko_alsa_volume_button_get_control (MokoAlsaVolumeButton *button)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (button);
	
	return priv->control;
}

