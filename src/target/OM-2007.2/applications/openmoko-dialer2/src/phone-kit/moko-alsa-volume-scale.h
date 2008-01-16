#ifndef _MOKO_ALSA_VOLUME_SCALE
#define _MOKO_ALSA_VOLUME_SCALE

#include <glib-object.h>
#include <gtk/gtk.h>
#include "moko-alsa-volume-control.h"

G_BEGIN_DECLS

#define MOKO_TYPE_ALSA_VOLUME_SCALE moko_alsa_volume_scale_get_type()

#define MOKO_ALSA_VOLUME_SCALE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	MOKO_TYPE_ALSA_VOLUME_SCALE, MokoAlsaVolumeScale))

#define MOKO_ALSA_VOLUME_SCALE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
	MOKO_TYPE_ALSA_VOLUME_SCALE, MokoAlsaVolumeScaleClass))

#define MOKO_IS_ALSA_VOLUME_SCALE(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	MOKO_TYPE_ALSA_VOLUME_SCALE))

#define MOKO_IS_ALSA_VOLUME_SCALE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	MOKO_TYPE_ALSA_VOLUME_SCALE))

#define MOKO_ALSA_VOLUME_SCALE_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
	MOKO_TYPE_ALSA_VOLUME_SCALE, MokoAlsaVolumeScaleClass))

typedef struct {
	GtkEventBox parent;
} MokoAlsaVolumeScale;

typedef struct {
	GtkEventBoxClass parent_class;
} MokoAlsaVolumeScaleClass;

GType moko_alsa_volume_scale_get_type (void);

GtkWidget *moko_alsa_volume_scale_new (GtkOrientation orientation);

void moko_alsa_volume_scale_set_control (MokoAlsaVolumeScale *scale,
					  MokoAlsaVolumeControl *control);

MokoAlsaVolumeControl *moko_alsa_volume_scale_get_control (
			MokoAlsaVolumeScale *scale);

G_END_DECLS

#endif /* _MOKO_ALSA_VOLUME_SCALE */

