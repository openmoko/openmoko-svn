
#ifndef OPENMOKO_RSS_RFC_DATE_H
#define OPENMOKO_RSS_RFC_DATE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define RSS_TYPE_RFC_DATE            (rss_rfc_date_get_type())
#define RSS_RFC_DATE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), RSS_TYPE_RFC_DATE, RSSRFCDate))
#define RSS_RFC_DATE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((obj),    RSS_TYPE_RFC_DATE, RSSRFCDateClass))
#define RSS_IS_RFC_DATE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), RSS_TYPE_RFC_DATE))
#define RSS_IS_RFC_DATE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  RSS_TYPE_RFC_DATE))
#define RSS_RFC_DATE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  RSS_TYPE_RFC_DATE, RSSRFCDateClass))

typedef struct _RSSRFCDate RSSRFCDate;
typedef struct _RSSRFCDateClass RSSRFCDateClass;

struct _RSSRFCDate {
    GObject parent;
    gint64  date;
};

struct _RSSRFCDateClass {
    GObjectClass parent;
};

GType     rss_rfc_date_get_type ();
GObject*  rss_rfc_date_new      ();
void      rss_rfc_date_set      (RSSRFCDate* self, const gchar* rfc822_date);
gint      rss_rfc_date_compare  (RSSRFCDate* self, RSSRFCDate *other);
gchar*    rss_rfc_date_as_string(RSSRFCDate* self);


G_END_DECLS

#endif
