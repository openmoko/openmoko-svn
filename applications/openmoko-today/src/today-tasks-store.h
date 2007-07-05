#ifndef _TODAY_TASKS_STORE
#define _TODAY_TASKS_STORE

#include <libkoto/koto-task-store.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define TODAY_TYPE_TASKS_STORE today_tasks_store_get_type()

#define TODAY_TASKS_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  TODAY_TYPE_TASKS_STORE, TodayTasksStore))

#define TODAY_TASKS_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  TODAY_TYPE_TASKS_STORE, TodayTasksStoreClass))

#define TODAY_IS_TASKS_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  TODAY_TYPE_TASKS_STORE))

#define TODAY_IS_TASKS_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  TODAY_TYPE_TASKS_STORE))

#define TODAY_TASKS_STORE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  TODAY_TYPE_TASKS_STORE, TodayTasksStoreClass))

typedef struct {
  KotoTaskStore parent;
} TodayTasksStore;

typedef struct {
  KotoTaskStoreClass parent_class;
} TodayTasksStoreClass;

GType today_tasks_store_get_type (void);

TodayTasksStore* today_tasks_store_new (void);

G_END_DECLS

#endif /* _TODAY_TASKS_STORE */
