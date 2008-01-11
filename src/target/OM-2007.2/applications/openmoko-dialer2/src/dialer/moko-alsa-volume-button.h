#ifndef _MOKO_ALSA_VOLUME_BUTTON
#define _MOKO_ALSA_VOLUME_BUTTON

#include <glib-object.h>
#include <gtk/gtk.h>
#include <alsa/asoundlib.h>

G_BEGIN_DECLS

#define MOKO_TYPE_ALSA_VOLUME_BUTTON moko_alsa_volume_button_get_type()

#define MOKO_ALSA_VOLUME_BUTTON(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	MOKO_TYPE_ALSA_VOLUME_BUTTON, MokoAlsaVolumeButton))

#define MOKO_ALSA_VOLUME_BUTTON_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
	MOKO_TYPE_ALSA_VOLUME_BUTTON, MokoAlsaVolumeButtonClass))

#define MOKO_IS_ALSA_VOLUME_BUTTON(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	MOKO_TYPE_ALSA_VOLUME_BUTTON))

#define MOKO_IS_ALSA_VOLUME_BUTTON_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	MOKO_TYPE_ALSA_VOLUME_BUTTON))

#define MOKO_ALSA_VOLUME_BUTTON_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
	MOKO_TYPE_ALSA_VOLUME_BUTTON, MokoAlsaVolumeButtonClass))

typedef struct {
	GtkScaleButton parent;
} MokoAlsaVolumeButton;

typedef struct {
	GtkScaleButtonClass parent_class;
} MokoAlsaVolumeButtonClass;

GType moko_alsa_volume_button_get_type (void);

GtkWidget *moko_alsa_volume_button_new (void);

void moko_alsa_volume_button_set_device (MokoAlsaVolumeButton *button,
					 const gchar *device);

void moko_alsa_volume_button_set_element (MokoAlsaVolumeButton *button,
					  snd_mixer_selem_id_t *element);

void moko_alsa_volume_button_set_device_from_card_number (
			MokoAlsaVolumeButton *button, gint number);

void moko_alsa_volume_button_set_device_from_name (MokoAlsaVolumeButton *button,
						   const gchar *name);

void moko_alsa_volume_button_set_element_from_name (
			MokoAlsaVolumeButton *button, const gchar *name);

G_END_DECLS

#endif /* _MOKO_ALSA_VOLUME_BUTTON */

