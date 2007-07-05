
#include <gtk/gtk.h>
#include <libecal/e-cal.h>
#include <libecal/e-cal-view.h>
#include <libecal/e-cal-component.h>
#include <libecal/e-cal-time-util.h>
#include <libical/icalcomponent.h>
#include "today-tasks-store.h"

G_DEFINE_TYPE (TodayTasksStore, today_tasks_store, KOTO_TYPE_TASK_STORE)

#define TASKS_STORE_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TODAY_TYPE_TASKS_STORE, TodayTasksStorePrivate))

typedef struct _TodayTasksStorePrivate TodayTasksStorePrivate;

struct _TodayTasksStorePrivate
{
	ECalView *tasks_view;
	ECal *tasks_ecal;
};

static gboolean
today_tasks_store_start (gpointer data)
{
	TodayTasksStore *store = TODAY_TASKS_STORE (data);
	TodayTasksStorePrivate *priv = TASKS_STORE_PRIVATE (store);
	
	if ((priv->tasks_ecal = e_cal_new_system_tasks ())) {
		GError *error = NULL;
		if (e_cal_open (priv->tasks_ecal, FALSE, &error)) {
			time_t t;
			time (&t);
			gchar *isodate = isodate_from_time_t (t);
			gchar *query = g_strdup_printf (
				/*"((!(is-completed?)) or ((is-completed?) and "
				"(!(completed-before? "
				"(time-day-begin (make-time %s))))))",
				isodate*/
				"#t");
			if (e_cal_get_query (priv->tasks_ecal,
			     query, &priv->tasks_view, &error)) {
				koto_task_store_set_view (
					KOTO_TASK_STORE (store),
					priv->tasks_view);
				e_cal_view_start (priv->tasks_view);
			} else {
				g_warning ("Unable to get tasks query\n"
					"\"%s\"\nError: %s",
					query, error->message);
				g_error_free (error);
				error = NULL;
			}
			g_free (query);
			g_free (isodate);
		} else {
			g_warning ("Unable to open system tasks: %s",
				error->message);
			g_error_free (error);
			error = NULL;
		}
	} else {
		g_warning ("Unable to retrieve system tasks");
	}
	
	return FALSE;
}

static void
today_tasks_store_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
today_tasks_store_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
today_tasks_store_dispose (GObject *object)
{
	if (G_OBJECT_CLASS (today_tasks_store_parent_class)->dispose)
		G_OBJECT_CLASS (today_tasks_store_parent_class)->dispose (
				object);
}

static void
today_tasks_store_finalize (GObject *object)
{
	G_OBJECT_CLASS (today_tasks_store_parent_class)->finalize (object);
}

static void
today_tasks_store_class_init (TodayTasksStoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (TodayTasksStorePrivate));

	object_class->get_property = today_tasks_store_get_property;
	object_class->set_property = today_tasks_store_set_property;
	object_class->dispose = today_tasks_store_dispose;
	object_class->finalize = today_tasks_store_finalize;
}

static void
today_tasks_store_init (TodayTasksStore *self)
{
	TodayTasksStorePrivate *priv = TASKS_STORE_PRIVATE (self);

	priv->tasks_ecal = NULL;
	priv->tasks_view = NULL;

	g_idle_add (today_tasks_store_start, self);
}

TodayTasksStore*
today_tasks_store_new (void)
{
	return g_object_new (TODAY_TYPE_TASKS_STORE, NULL);
}
