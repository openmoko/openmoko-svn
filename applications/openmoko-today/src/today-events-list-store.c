
#include <gtk/gtk.h>
#include <libecal/e-cal.h>
#include <libecal/e-cal-view.h>
#include <libecal/e-cal-component.h>
#include <libecal/e-cal-time-util.h>
#include <libical/icalcomponent.h>
#include "today-events-list-store.h"
#include "config.h"

G_DEFINE_TYPE (TodayEventsListStore, today_events_list_store, GTK_TYPE_LIST_STORE)

#define EVENTS_LIST_STORE_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), TODAY_TYPE_EVENTS_LIST_STORE, TodayEventsListStorePrivate))

typedef struct _TodayEventsListStorePrivate TodayEventsListStorePrivate;

struct _TodayEventsListStorePrivate
{
	ECalView *events_view;
	ECal *events_ecal;
	GHashTable *hash_table;
};

static void
today_events_list_store_get_property (GObject *object, guint property_id,
				      GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
today_events_list_store_set_property (GObject *object, guint property_id,
				      const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
today_events_list_store_dispose (GObject *object)
{
	if (G_OBJECT_CLASS (today_events_list_store_parent_class)->dispose)
		G_OBJECT_CLASS (today_events_list_store_parent_class)->dispose (
			object);
}

static void
today_events_list_store_finalize (GObject *object)
{
	G_OBJECT_CLASS (today_events_list_store_parent_class)->
		finalize (object);
	
	TodayEventsListStorePrivate *priv = EVENTS_LIST_STORE_PRIVATE (object);
	
	g_hash_table_destroy (priv->hash_table);
	if (priv->events_view) g_object_unref (priv->events_view);
	if (priv->events_view) g_object_unref (priv->events_ecal);
	priv->events_view = NULL;
	priv->events_ecal = NULL;
}

static void
today_events_list_store_class_init (TodayEventsListStoreClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	g_type_class_add_private (klass, sizeof (TodayEventsListStorePrivate));
	
	object_class->get_property = today_events_list_store_get_property;
	object_class->set_property = today_events_list_store_set_property;
	object_class->dispose = today_events_list_store_dispose;
	object_class->finalize = today_events_list_store_finalize;
}

static void
today_events_list_store_objects_added (ECalView *ecalview, GList *objects,
				       TodayEventsListStore *store)
{
	TodayEventsListStorePrivate *priv = EVENTS_LIST_STORE_PRIVATE (store);

	for (; objects; objects = objects->next) {
		struct tm start_tm;
		icaltimetype start;
		char time_string[64];
		gchar *string;
		GtkTreeIter *iter;
		
		icalcomponent *comp = (icalcomponent *)objects->data;
		gchar *uid = (gchar *)icalcomponent_get_uid (comp);
		
		if (!uid) continue;
		uid = g_strdup (uid);
		
		start = icalcomponent_get_dtstart (comp);
		start_tm = icaltimetype_to_tm (&start);
		
		strftime (time_string, sizeof (time_string), "%I:%M%p",
			&start_tm);
		string = (gchar *)icalcomponent_get_description (comp);
		string = g_strdup_printf ("(%s) %s", time_string,
			string ? string : "New event");
		iter = g_new0 (GtkTreeIter, 1);
		gtk_list_store_insert_with_values (GTK_LIST_STORE (store),
			iter, 0,
			TODAY_EVENTS_LIST_STORE_COL_STRING, string,
			TODAY_EVENTS_LIST_STORE_COL_UID, uid,
			-1);
		g_hash_table_insert (priv->hash_table, (gpointer)uid,
			(gpointer)iter);
	}
}

static void
today_events_list_store_objects_modified (ECalView *ecalview, GList *objects,
					  TodayEventsListStore *store)
{
}

static void
today_events_list_store_objects_removed (ECalView *ecalview, GList *uids,
					 TodayEventsListStore *store)
{
	TodayEventsListStorePrivate *priv = EVENTS_LIST_STORE_PRIVATE (store);

	for (; uids; uids = uids->next) {
		const gchar *uid;
		gchar *string;
		GtkTreeIter *iter;
		
#ifdef HAVE_CID_TYPE
		ECalComponentId *id = uids->data;
		/* FIXME: What happens with uid/rid here? */
		uid = id->uid;
#else
		uid = uids->data;
#endif

		iter = (GtkTreeIter *)
			g_hash_table_lookup (priv->hash_table, uid);
		if (!iter) continue;
		
		gtk_tree_model_get (GTK_TREE_MODEL (store), iter,
			TODAY_EVENTS_LIST_STORE_COL_STRING, &string, -1);
		gtk_list_store_remove (GTK_LIST_STORE (store), iter);
		g_hash_table_remove (priv->hash_table, uid);
		g_free (string);
	}
}

static gboolean
today_events_list_store_start (gpointer data)
{
	TodayEventsListStore *store = TODAY_EVENTS_LIST_STORE (data);
	TodayEventsListStorePrivate *priv = EVENTS_LIST_STORE_PRIVATE (store);
	
	if ((priv->events_ecal = e_cal_new_system_calendar ())) {
		GError *error = NULL;
		if (e_cal_open (priv->events_ecal, FALSE, &error)) {
			time_t t;
			time (&t);
			gchar *isodate = isodate_from_time_t (t);
			gchar *query = g_strdup_printf (
				"(occur-in-time-range? (time-day-begin "
					"(make-time \"%s\")) "
				"(time-add-day (time-day-begin "
					"(make-time \"%s\")) 7))",
				isodate, isodate);
			if (e_cal_get_query (priv->events_ecal,
			     query, &priv->events_view, &error)) {
				g_signal_connect (G_OBJECT (priv->events_view),
					"objects-added",
					G_CALLBACK (
					today_events_list_store_objects_added),
					store);
				g_signal_connect (G_OBJECT (priv->events_view),
					"objects-modified",
					G_CALLBACK (
					today_events_list_store_objects_modified
					),
					store);
				g_signal_connect (G_OBJECT (priv->events_view),
					"objects-removed",
					G_CALLBACK (
					today_events_list_store_objects_removed
					),
					store);
				e_cal_view_start (priv->events_view);
			} else {
				g_warning ("Unable to get calendar query: %s",
					error->message);
				g_error_free (error);
				error = NULL;
			}
			g_free (query);
			g_free (isodate);
		} else {
			g_warning ("Unable to open system calendar: %s",
				error->message);
			g_error_free (error);
			error = NULL;
		}
	} else {
		g_warning ("Unable to retrieve system calendar");
	}
	
	return FALSE;
}

static void
today_events_list_store_init (TodayEventsListStore *self)
{
	TodayEventsListStorePrivate *priv = EVENTS_LIST_STORE_PRIVATE (self);

	priv->events_ecal = NULL;
	priv->events_view = NULL;
	priv->hash_table = g_hash_table_new_full (g_str_hash, g_str_equal,
		g_free, g_free);
	
	gtk_list_store_set_column_types (GTK_LIST_STORE (self), 2,
		(GType []){ G_TYPE_STRING, G_TYPE_STRING });
	
	g_idle_add (today_events_list_store_start, self);
}

TodayEventsListStore *
today_events_list_store_new (void)
{
	return g_object_new (TODAY_TYPE_EVENTS_LIST_STORE, NULL);
}
