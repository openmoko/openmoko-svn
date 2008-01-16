#ifndef _MOKO_ALSA_VOLUME_CONTROL
#define _MOKO_ALSA_VOLUME_CONTROL

#include <glib.h>
#include <glib-object.h>
#include <alsa/asoundlib.h>

G_BEGIN_DECLS

#define MOKO_TYPE_ALSA_VOLUME_CONTROL moko_alsa_volume_control_get_type()

#define MOKO_ALSA_VOLUME_CONTROL(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	MOKO_TYPE_ALSA_VOLUME_CONTROL, MokoAlsaVolumeControl))

#define MOKO_ALSA_VOLUME_CONTROL_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
	MOKO_TYPE_ALSA_VOLUME_CONTROL, MokoAlsaVolumeControlClass))

#define MOKO_IS_ALSA_VOLUME_CONTROL(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	MOKO_TYPE_ALSA_VOLUME_CONTROL))

#define MOKO_IS_ALSA_VOLUME_CONTROL_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	MOKO_TYPE_ALSA_VOLUME_CONTROL))

#define MOKO_ALSA_VOLUME_CONTROL_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
	MOKO_TYPE_ALSA_VOLUME_CONTROL, MokoAlsaVolumeControlClass))

typedef struct {
	GObject parent;
} MokoAlsaVolumeControl;

typedef struct {
	GObjectClass parent_class;
	
	/* Signals */
	void	(*volume_changed)	(MokoAlsaVolumeControl *control,
					 gdouble volume);
} MokoAlsaVolumeControlClass;

GType moko_alsa_volume_control_get_type (void);

MokoAlsaVolumeControl *moko_alsa_volume_control_new (void);

void moko_alsa_volume_control_set_device (MokoAlsaVolumeControl *control,
					 const gchar *device);

void moko_alsa_volume_control_set_element (MokoAlsaVolumeControl *control,
					  snd_mixer_selem_id_t *element);

void moko_alsa_volume_control_set_device_from_card_number (
			MokoAlsaVolumeControl *control,
			gint number);

void moko_alsa_volume_control_set_device_from_name (
			MokoAlsaVolumeControl *control,
			const gchar *name);

void moko_alsa_volume_control_set_element_from_name (
			MokoAlsaVolumeControl *control,
			const gchar *name);

gdouble moko_alsa_volume_control_get_volume (MokoAlsaVolumeControl *control);

void moko_alsa_volume_control_set_volume (MokoAlsaVolumeControl *control,
					  gdouble volume);

G_END_DECLS

#endif /* _MOKO_ALSA_VOLUME_CONTROL */

