
#ifndef _TODAY_HEADER_BOX
#define _TODAY_HEADER_BOX

#include <gtk/gtk.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define TODAY_TYPE_HEADER_BOX today_header_box_get_type()

#define TODAY_HEADER_BOX(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  TODAY_TYPE_HEADER_BOX, TodayHeaderBox))

#define TODAY_HEADER_BOX_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  TODAY_TYPE_HEADER_BOX, TodayHeaderBoxClass))

#define TODAY_IS_HEADER_BOX(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  TODAY_TYPE_HEADER_BOX))

#define TODAY_IS_HEADER_BOX_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  TODAY_TYPE_HEADER_BOX))

#define TODAY_HEADER_BOX_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  TODAY_TYPE_HEADER_BOX, TodayHeaderBoxClass))

typedef struct {
  GtkVBox parent;
} TodayHeaderBox;

typedef struct {
  GtkVBoxClass parent_class;
} TodayHeaderBoxClass;

GType today_header_box_get_type (void);

GtkWidget *today_header_box_new (void);
GtkWidget *today_header_box_new_with_markup (const gchar *markup);

G_END_DECLS

#endif /* _TODAY_HEADER_BOX */
