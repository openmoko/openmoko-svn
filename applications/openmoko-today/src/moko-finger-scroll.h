#ifndef _MOKO_FINGER_SCROLL
#define _MOKO_FINGER_SCROLL

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MOKO_TYPE_FINGER_SCROLL moko_finger_scroll_get_type()

#define MOKO_FINGER_SCROLL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MOKO_TYPE_FINGER_SCROLL, MokoFingerScroll))

#define MOKO_FINGER_SCROLL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MOKO_TYPE_FINGER_SCROLL, MokoFingerScrollClass))

#define MOKO_IS_FINGER_SCROLL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MOKO_TYPE_FINGER_SCROLL))

#define MOKO_IS_FINGER_SCROLL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MOKO_TYPE_FINGER_SCROLL))

#define MOKO_FINGER_SCROLL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MOKO_TYPE_FINGER_SCROLL, MokoFingerScrollClass))

typedef struct {
  GtkEventBox parent;
} MokoFingerScroll;

typedef struct {
  GtkEventBoxClass parent_class;
} MokoFingerScrollClass;

typedef enum {
	MOKO_FINGER_SCROLL_MODE_PUSH,
	MOKO_FINGER_SCROLL_MODE_ACCEL,
} MokoFingerScrollMode;

GType moko_finger_scroll_get_type (void);

GtkWidget* moko_finger_scroll_new (void);

G_END_DECLS

#endif /* _MOKO_FINGER_SCROLL */
