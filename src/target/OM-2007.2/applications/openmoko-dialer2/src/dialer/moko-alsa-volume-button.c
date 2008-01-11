
#include "moko-alsa-volume-button.h"
#include <gtk/gtk.h>
#include <alsa/asoundlib.h>

G_DEFINE_TYPE (MokoAlsaVolumeButton, moko_alsa_volume_button, \
	GTK_TYPE_SCALE_BUTTON)

#define ALSA_VOLUME_BUTTON_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_ALSA_VOLUME_BUTTON, \
	 MokoAlsaVolumeButtonPrivate))

typedef struct _MokoAlsaVolumeButtonPrivate MokoAlsaVolumeButtonPrivate;

struct _MokoAlsaVolumeButtonPrivate {
	gchar *device;
	snd_mixer_selem_id_t *element;
	
	snd_mixer_t *mixer_handle;
	snd_mixer_elem_t *mixer_elem;
	gint control_type;
	
	glong min;
	glong max;
};

enum {
	PROP_DEVICE = 1,
	PROP_ELEMENT,
};

enum {
	PLAYBACK,
	CAPTURE,
	CONTROL
};

static gboolean
io_func (GIOChannel *source, GIOCondition condition, MokoAlsaVolumeButton *self)
{
	switch (condition) {
	    case G_IO_IN : {
		MokoAlsaVolumeButtonPrivate *priv =
			ALSA_VOLUME_BUTTON_PRIVATE (self);
		snd_mixer_handle_events (priv->mixer_handle);
		
		break;
	    }
	    case G_IO_ERR :
	    case G_IO_NVAL :
		g_warning ("Encountered an error, stopping IO watch");
		return FALSE;
	    default :
		g_warning ("Unhandled IO condition");
		break;
	}
	
	return TRUE;
}

static void
update_adjustment (MokoAlsaVolumeButton *button)
{
	long volume, old_volume;
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (button);
	
	/* TODO: Average out volume across channels? */
	
	switch (priv->control_type) {
	    case PLAYBACK :
		snd_mixer_selem_get_playback_volume (
			priv->mixer_elem, 0, &volume);
		break;
	    case CAPTURE :
		snd_mixer_selem_get_capture_volume (
			priv->mixer_elem, 0, &volume);
		break;
	    case CONTROL :
	    default :
		/* TODO: Handle switches? */
		g_warning ("Unhandled control type");
		return;
	}
	
	old_volume = (long)((gtk_scale_button_get_value (
		GTK_SCALE_BUTTON (button)) / 100.0) *
		(gdouble)(priv->max-priv->min)) + priv->min;
	if (volume != old_volume)
		gtk_scale_button_set_value (GTK_SCALE_BUTTON (button),
			((gdouble)(volume-priv->min)) /
				((gdouble)priv->max-priv->min) * 100.0);
}

static int
mixer_event_cb (snd_mixer_t *mixer, unsigned int mask, snd_mixer_elem_t *elem)
{
	/*MokoAlsaVolumeButton *button = MOKO_ALSA_VOLUME_BUTTON (
		snd_mixer_get_callback_private (mixer));
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (button);*/
	
	return 0;
}

static int
mixer_elem_event_cb (snd_mixer_elem_t *elem, unsigned int mask)
{
	MokoAlsaVolumeButton *button = MOKO_ALSA_VOLUME_BUTTON (
		snd_mixer_elem_get_callback_private (elem));

	update_adjustment (button);

	return 0;
}

static void
open_mixer (MokoAlsaVolumeButton *self)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (self);
	
	if (snd_mixer_open (&priv->mixer_handle, 0) != 0) {
		g_warning ("Failed to get mixer handle");
		priv->mixer_handle = NULL;
		return;
	}
	
	snd_mixer_set_callback (priv->mixer_handle, mixer_event_cb);
	snd_mixer_set_callback_private (priv->mixer_handle, self);
	
	g_debug ("Opened mixer");
}

static void
start_polling (MokoAlsaVolumeButton *self)
{
	struct pollfd *fds;
	gint i, nfds;

	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (self);
	
	if ((nfds = snd_mixer_poll_descriptors_count (priv->mixer_handle)) <= 0){
		g_warning ("No poll descriptors on mixer?");
		return;
	}
	
	fds = g_new0 (struct pollfd, nfds);
	if (snd_mixer_poll_descriptors (priv->mixer_handle, fds, nfds) < 0) {
		g_warning ("Error getting polling descriptors for sound mixer");
		g_free (fds);
		return;
	}
	
	for (i = 0; i < nfds; i++) {
		GIOChannel *channel = g_io_channel_unix_new (fds[i].fd);
		g_debug ("Adding IO watch (IN: %d, OUT: %d)",
			fds[i].events & POLLIN, fds[i].events & POLLOUT);
		g_io_add_watch (channel,
			((fds[i].events & POLLIN) ? G_IO_IN : 0) |
			((fds[i].events & POLLOUT) ? G_IO_OUT : 0) |
			((fds[i].events & POLLPRI) ? G_IO_PRI : 0) |
			((fds[i].events & POLLERR) ? G_IO_ERR : 0) |
			((fds[i].events & POLLHUP) ? G_IO_HUP : 0) |
			((fds[i].events & POLLNVAL) ? G_IO_NVAL : 0),
			(GIOFunc)io_func, self);
	}
	g_free (fds);
	
	g_debug ("Polling for events...");
}

static void
close_mixer (MokoAlsaVolumeButton *self)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (self);
	
	if (!priv->mixer_handle) return;
	
	snd_mixer_close (priv->mixer_handle);
	priv->mixer_handle = NULL;
	g_debug ("Closed mixer");
}

static void
detach_mixer (MokoAlsaVolumeButton *self)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (self);
	
	if (priv->mixer_handle && priv->device &&
	    priv->element && priv->mixer_elem) {
		snd_mixer_detach (priv->mixer_handle, priv->device);
		priv->mixer_elem = NULL;
		g_debug ("Detached from mixer");
		close_mixer (self);
	}
}

static void
attach_mixer (MokoAlsaVolumeButton *self)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (self);
	
	g_debug ("Trying to attach... %p, %s, %p", priv->mixer_handle,
		priv->device, priv->element);
	
	open_mixer (self);
	
	if (priv->mixer_handle && priv->device && priv->element &&
	    (snd_mixer_attach (priv->mixer_handle, priv->device) == 0) &&
	    (snd_mixer_selem_register (priv->mixer_handle, NULL, NULL) == 0) &&
	    (snd_mixer_load (priv->mixer_handle) == 0)) {
		priv->mixer_elem = snd_mixer_find_selem (
			priv->mixer_handle, priv->element);
		if (!priv->mixer_elem) {
			g_warning ("Unable to find mixer element");
			snd_mixer_detach (priv->mixer_handle, priv->device);
			close_mixer (self);
		} else {
			g_debug ("Attached to mixer");
			
			if (snd_mixer_selem_has_playback_volume (
			    priv->mixer_elem)) {
				priv->control_type = PLAYBACK;
				snd_mixer_selem_get_playback_volume_range (
					priv->mixer_elem,
					&priv->min, &priv->max);
			} else if (snd_mixer_selem_has_capture_volume (
				 priv->mixer_elem)) {
				priv->control_type = CAPTURE;
				snd_mixer_selem_get_capture_volume_range (
					priv->mixer_elem,
					&priv->min, &priv->max);
			} else
				priv->control_type = CONTROL;
			
			snd_mixer_elem_set_callback (
				priv->mixer_elem, mixer_elem_event_cb);
			snd_mixer_elem_set_callback_private (
				priv->mixer_elem, self);
			
			start_polling (self);
			update_adjustment (self);
		}
	} else {
		close_mixer (self);
	}
}

static void
moko_alsa_volume_button_get_property (GObject *object, guint property_id,
				      GValue *value, GParamSpec *pspec)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (object);
	
	switch (property_id) {
	    case PROP_DEVICE :
		g_value_set_string (value, priv->device);
		break;
	    case PROP_ELEMENT :
		g_value_set_pointer (value, priv->element);
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
	    case PROP_DEVICE :
		moko_alsa_volume_button_set_device (
			MOKO_ALSA_VOLUME_BUTTON (object),
			g_value_get_string (value));
		break;
		
	    case PROP_ELEMENT :
		moko_alsa_volume_button_set_element (
			MOKO_ALSA_VOLUME_BUTTON (object),
			(snd_mixer_selem_id_t *)g_value_get_pointer (value));
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
	
	detach_mixer (button);
	
	g_free (priv->device);
	if (priv->element) {
		snd_mixer_selem_id_free (priv->element);
	}
	
	G_OBJECT_CLASS (moko_alsa_volume_button_parent_class)->
		finalize (object);
}

static void
moko_alsa_volume_button_value_changed (GtkScaleButton *button, gdouble value)
{
	long volume;
	
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (button);
	
	if (!priv->mixer_elem) return;
	
	if (GTK_SCALE_BUTTON_CLASS (moko_alsa_volume_button_parent_class)->
	    value_changed)
		GTK_SCALE_BUTTON_CLASS (moko_alsa_volume_button_parent_class)->
			value_changed (button, value);
	
	volume = (long)((gtk_scale_button_get_value (
		GTK_SCALE_BUTTON (button)) / 100.0) *
		(gdouble)(priv->max-priv->min)) + priv->min;

	switch (priv->control_type) {
	    case PLAYBACK :
		snd_mixer_selem_set_playback_volume_all (
			priv->mixer_elem, volume);
		break;
	    case CAPTURE :
		snd_mixer_selem_set_capture_volume_all (
			priv->mixer_elem, volume);
		break;
	    default :
		g_warning ("Unhandled control type");
	}
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
		PROP_DEVICE,
		g_param_spec_string (
			"device",
			"gchar *",
			"The alsa device name.",
			"default",
			G_PARAM_READWRITE));

	g_object_class_install_property (
		object_class,
		PROP_ELEMENT,
		g_param_spec_pointer (
			"element",
			"snd_mixer_selem_id_t",
			"The alsa simple mixer element ID.",
			G_PARAM_READWRITE));
}

static void
moko_alsa_volume_button_init (MokoAlsaVolumeButton *self)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (self);
	
	priv->device = g_strdup ("default");
}

GtkWidget *
moko_alsa_volume_button_new (void)
{
	return GTK_WIDGET (g_object_new (MOKO_TYPE_ALSA_VOLUME_BUTTON, NULL));
}

void
moko_alsa_volume_button_set_device (MokoAlsaVolumeButton *button,
				    const gchar *device)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (button);

	detach_mixer (button);
	g_free (priv->device);
	priv->device = g_strdup (device);
	g_debug ("Device set: %s", device);
	attach_mixer (button);
}

void
moko_alsa_volume_button_set_element (MokoAlsaVolumeButton *button,
				     snd_mixer_selem_id_t *element)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (button);

	detach_mixer (button);
	if (priv->element) {
		snd_mixer_selem_id_free (priv->element);
		priv->element = NULL;
	}
	
	if (snd_mixer_selem_id_malloc (&priv->element) != 0) {
		g_warning ("Unable to allocate mixer element id");
	} else if (element) {
		snd_mixer_selem_id_copy (priv->element, element);
		g_debug ("Element set");
		attach_mixer (button);
	}
}

void
moko_alsa_volume_button_set_device_from_card_number (
	MokoAlsaVolumeButton *button, gint number)
{
	void **hints;
	
	if (snd_device_name_hint (number, "pcm", &hints) == 0) {
		gchar *device = strdup (snd_device_name_get_hint (
			hints[0], "NAME"));
		snd_device_name_free_hint (hints);
		strchr (device, ':')[0] = '\0';
		
		moko_alsa_volume_button_set_device (button, device);
		g_free (device);
	} else
		g_warning ("Unable to find card number %d", number);
}

void
moko_alsa_volume_button_set_device_from_name (MokoAlsaVolumeButton *button,
					      const gchar *name)
{
	gint i = -1;
	
	if (!name) {
		moko_alsa_volume_button_set_device (button, NULL);
		return;
	}

	while (snd_card_next (&i) == 0) {
		void **hints;
	
		if (snd_device_name_hint (i, "pcm", &hints) == 0) {
			gchar *device = strdup (snd_device_name_get_hint (
				hints[0], "NAME"));
			snd_device_name_free_hint (hints);
			strchr (device, ':')[0] = '\0';
			
			if (strcmp (device, name) == 0) {
				moko_alsa_volume_button_set_device (
					button, device);
				g_free (device);
				return;
			}
			g_free (device);
		}
	}
	
	g_warning ("Card '%s' not found", name);
}

void
moko_alsa_volume_button_set_element_from_name (MokoAlsaVolumeButton *button,
					       const gchar *name)
{
	MokoAlsaVolumeButtonPrivate *priv = ALSA_VOLUME_BUTTON_PRIVATE (button);

	if (!priv->device) return;
	
	detach_mixer (button);
	
	if (!name) {
		moko_alsa_volume_button_set_element (button, NULL);
		return;
	}
	
	open_mixer (button);
	if ((snd_mixer_attach (priv->mixer_handle, priv->device) == 0) &&
	    (snd_mixer_selem_register (priv->mixer_handle, NULL, NULL) == 0) &&
	    (snd_mixer_load (priv->mixer_handle) == 0)) {
		snd_mixer_elem_t *elem;
		
		elem = snd_mixer_first_elem (priv->mixer_handle);
		while (elem) {
			const char *elem_name = snd_mixer_selem_get_name (elem);
			if (strcmp (elem_name, name) == 0)
				break;
			elem = snd_mixer_elem_next (elem);
		}
		
		if (!elem) {
			snd_mixer_detach (priv->mixer_handle, priv->device);
			close_mixer (button);
			g_warning ("Mixer element '%s' not found", name);
			attach_mixer (button);
		} else {
			snd_mixer_selem_id_t *id;
			if (snd_mixer_selem_id_malloc (&id) != 0) {
				g_warning ("Unable to allocate element id");
				snd_mixer_detach (
					priv->mixer_handle, priv->device);
				close_mixer (button);
			} else {
				snd_mixer_selem_get_id (elem, id);
				snd_mixer_detach (
					priv->mixer_handle, priv->device);
				close_mixer (button);
				g_debug ("Setting element ID");
				moko_alsa_volume_button_set_element (
					button, id);
				snd_mixer_selem_id_free (id);
			}
		}
	} else
		g_warning ("Unable to open mixer on card '%s'", priv->device);
}

