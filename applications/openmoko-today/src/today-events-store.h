#ifndef _TODAY_EVENTS_STORE
#define _TODAY_EVENTS_STORE

#include <gtk/gtk.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define TODAY_TYPE_EVENTS_STORE today_events_store_get_type()

#define TODAY_EVENTS_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  TODAY_TYPE_EVENTS_STORE, TodayEventsStore))

#define TODAY_EVENTS_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  TODAY_TYPE_EVENTS_STORE, TodayEventsStoreClass))

#define TODAY_IS_EVENTS_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  TODAY_TYPE_EVENTS_STORE))

#define TODAY_IS_EVENTS_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  TODAY_TYPE_EVENTS_STORE))

#define TODAY_EVENTS_STORE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  TODAY_TYPE_EVENTS_STORE, TodayEventsStoreClass))

typedef struct {
  GtkListStore parent;
} TodayEventsStore;

typedef struct {
  GtkListStoreClass parent_class;
} TodayEventsStoreClass;

enum {
	TODAY_EVENTS_STORE_COL_SUMMARY,
	TODAY_EVENTS_STORE_COL_UID,
};

GType today_events_store_get_type (void);

TodayEventsStore* today_events_store_new (void);

G_END_DECLS

#endif /* _TODAY_EVENTS_STORE */
