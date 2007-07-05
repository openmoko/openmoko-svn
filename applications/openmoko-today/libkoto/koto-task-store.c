/*
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>
#include <string.h>
#include <libecal/e-cal.h>
#include "koto-task.h"
#include "koto-task-store.h"
#include "ical-util.h"

G_DEFINE_TYPE (KotoTaskStore, koto_task_store, GTK_TYPE_LIST_STORE);

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), KOTO_TYPE_TASK_STORE, KotoTaskStorePrivate))

typedef struct _KotoTaskStorePrivate KotoTaskStorePrivate;

struct _KotoTaskStorePrivate
{
  ECal *cal;
  ECalView *view;
  guint sig_added, sig_modified, sig_removed;
  GHashTable *uid_hash;
};

enum {
  PROP_0,
  PROP_CALVIEW,
};

/*
 * Private methods
 */

static gint
sorter_cb (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
  int res;
  gboolean done_a, done_b;
  int weight_a, weight_b;
  char *summary_a, *summary_b;

  gtk_tree_model_get (model, a,
                      COLUMN_DONE, &done_a,
                      COLUMN_WEIGHT, &weight_a,
                      COLUMN_SUMMARY, &summary_a,
                      -1);
  gtk_tree_model_get (model, b,
                      COLUMN_DONE, &done_b,
                      COLUMN_WEIGHT, &weight_b,
                      COLUMN_SUMMARY, &summary_b,
                      -1);

  if (done_a != done_b) {
    res = done_a < done_b ? -1 : 1;
    goto done;
  }
  
  if (weight_a != weight_b) {
    res = weight_a < weight_b ? -1 : 1;
    goto done;
  }
  
  res = g_utf8_collate (summary_a ?: "", summary_b ?: "");
  
 done:
  g_free (summary_a);
  g_free (summary_b);

  return res;
}

static int
get_weight (int priority, struct icaltimetype due) {

  struct icaltimetype today;

  if (priority == PRIORITY_NONE)
    priority = PRIORITY_MEDIUM;

  if (icaltime_is_null_time (due)) {
    return priority;
  }

  today = icaltime_today ();

  /* If we're due in the past */
  if (icaltime_compare_date_only (due, today) < 0)
    return priority - 10;

  /* If it's due today */
  if (icaltime_compare_date_only(due, today) == 0)
    return priority - 5;

  /* If it's due in the next three days */
  icaltime_adjust(&today, 3, 0, 0, 0);
  if (icaltime_compare_date_only(due, today) <= 0)
    return priority - 2;

  /* If its due later than a fortnight away */
  icaltime_adjust(&today, -3 + 14, 0, 0, 0);
  if (icaltime_compare_date_only(due, today) > 0)
    return priority + 2;

  return priority;
}

/*
 * Utility method called by objects_added and objects_modified that inserts a
 * new row (@insert is #TRUE) or updates a row (@insert is #FALSE) in @store
 * pointed to by @iter with data extracted from @ical.
 */
static void
update_row (KotoTaskStore *store, icalcomponent *ical, gboolean insert, GtkTreeIter *iter)
{
  icalproperty_status status;
  icaltimetype due_time;
  GDate *due_date = NULL;
  KotoTask *task;

  g_assert (KOTO_IS_TASK_STORE (store));
  g_assert (ical);
  g_assert (iter);
  
  status = icalcomponent_get_status (ical);
  due_time = icalcomponent_get_due (ical);
  if (!icaltime_is_null_time (due_time)) {
    due_date = g_date_new ();
    g_date_set_time_t (due_date,
                       icaltime_as_timet_with_zone (due_time,
                                                    icaltimezone_get_utc_timezone ()));
  }
  
  task = koto_task_new (icalcomponent_new_clone (ical));

  if (insert) {
    gtk_list_store_insert_with_values (GTK_LIST_STORE (store), iter, 0,
                                       COLUMN_ICAL, task,
                                       COLUMN_DONE, status == ICAL_STATUS_COMPLETED,
                                       COLUMN_PRIORITY, ical_util_get_priority (ical),
                                       COLUMN_WEIGHT, get_weight (ical_util_get_priority (ical), due_time),
                                       COLUMN_DUE, due_date,
                                       COLUMN_SUMMARY, icalcomponent_get_summary (ical),
                                       COLUMN_URL, ical_util_get_url (ical),
                                       -1);
  } else {
    gtk_list_store_set (GTK_LIST_STORE (store), iter,
                        COLUMN_ICAL, task,
                        COLUMN_DONE, status == ICAL_STATUS_COMPLETED,
                        COLUMN_PRIORITY, ical_util_get_priority (ical),
                        COLUMN_WEIGHT, get_weight (ical_util_get_priority (ical), due_time),
                        COLUMN_DUE, due_date,
                        COLUMN_SUMMARY, icalcomponent_get_summary (ical),
                        COLUMN_URL, ical_util_get_url (ical),
                        -1);
  }
  
  if (due_date)
    g_date_free (due_date);
  koto_task_unref (task);
}

/*
 * Callback when objects in the calendar view are added.
 */
static void
on_objects_added (ECalView *view, GList *objects, KotoTaskStore *store)
{
  KotoTaskStorePrivate *priv = GET_PRIVATE (store);

  for (;objects; objects = g_list_next (objects)) {
    icalcomponent *ical;
    GtkTreeIter iter;

    ical = objects->data;

    update_row (store, ical, TRUE, &iter);

    g_hash_table_insert (priv->uid_hash,
                         g_strdup (icalcomponent_get_uid (ical)),
                         gtk_tree_iter_copy (&iter));
    
  }
}

/*
 * Callback when objects in the calendar view are modified.
 */
static void
on_objects_modified (ECalView *view, GList *objects, KotoTaskStore *store)
{
  KotoTaskStorePrivate *priv = GET_PRIVATE (store);

  for (;objects; objects = g_list_next (objects)) {
    icalcomponent *ical;
    GtkTreeIter *iter;

    ical = objects->data;

    iter = g_hash_table_lookup (priv->uid_hash, icalcomponent_get_uid (ical));

    update_row (store, ical, FALSE, iter);
  }
}

/*
 * Callback when objects in the calendar view are removed.
 */
static void
on_objects_removed (ECalView *view, GList *uids, KotoTaskStore *store) {
  KotoTaskStorePrivate *priv = GET_PRIVATE (store);

  for (; uids; uids = g_list_next (uids)) {
    GtkTreeIter *iter;
    const char *uid;
#if HAVE_ECALCOMPONENTID
    ECalComponentId *id = uids->data;
    /* TODO: check uid/rid to handle recurrant tasks? */
    uid = id->uid;
#else
    uid = uids->data;
#endif

    iter = g_hash_table_lookup (priv->uid_hash, uid);
    if (iter) {
      gtk_list_store_remove (GTK_LIST_STORE (store), iter);
      g_hash_table_remove (priv->uid_hash, uid);
    } else {
      g_warning ("Cannot find iter for removed UID %s", uid);
    }
  }
}

/*
 * Object methods
 */

static void
koto_task_store_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  KotoTaskStorePrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
  case PROP_CALVIEW:
    g_value_set_object (value, priv->view);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_task_store_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_CALVIEW:
    koto_task_store_set_view (KOTO_TASK_STORE (object),
                               g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_task_store_dispose (GObject *object)
{
  koto_task_store_set_view (KOTO_TASK_STORE (object), NULL);

  if (G_OBJECT_CLASS (koto_task_store_parent_class)->dispose)
    G_OBJECT_CLASS (koto_task_store_parent_class)->dispose (object);
}

static void
koto_task_store_finalize (GObject *object)
{
  KotoTaskStorePrivate *priv = GET_PRIVATE (object);

  g_hash_table_destroy (priv->uid_hash);
  
  G_OBJECT_CLASS (koto_task_store_parent_class)->finalize (object);
}

static void
koto_task_store_class_init (KotoTaskStoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (KotoTaskStorePrivate));

  object_class->get_property = koto_task_store_get_property;
  object_class->set_property = koto_task_store_set_property;
  object_class->dispose = koto_task_store_dispose;
  object_class->finalize = koto_task_store_finalize;

  g_object_class_install_property (object_class, PROP_CALVIEW,
                                   g_param_spec_object ("calview", "calview", NULL,
                                                        E_TYPE_CAL_VIEW,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK));
}

static void
koto_task_store_init (KotoTaskStore *self)
{
  KotoTaskStorePrivate *priv = GET_PRIVATE (self);

  const GType column_types[] = {
    KOTO_TYPE_TASK, /* icalcomponent */
    G_TYPE_BOOLEAN, /* done */
    G_TYPE_INT, /* weight */
    G_TYPE_INT, /* priority */
    G_TYPE_DATE, /* due */
    G_TYPE_STRING, /* summary */
    G_TYPE_STRING, /* URL */
  };

  gtk_list_store_set_column_types (GTK_LIST_STORE (self),
                                   G_N_ELEMENTS (column_types),
                                   (GType *) column_types);

  gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (self),
                                           sorter_cb, NULL, NULL);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (self),
                                        GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
                                        GTK_SORT_ASCENDING);

  priv->uid_hash = g_hash_table_new_full (g_str_hash, g_str_equal,
                                          g_free, (GDestroyNotify)gtk_tree_iter_free);
}


/*
 * Public methods
 */

GtkTreeModel*
koto_task_store_new (ECalView *view)
{
  return g_object_new (KOTO_TYPE_TASK_STORE,
                       "calview", view,
                       NULL);
}

void
koto_task_store_set_view (KotoTaskStore *store, ECalView *view)
{
  KotoTaskStorePrivate *priv;

  g_return_if_fail (KOTO_IS_TASK_STORE (store));

  priv = GET_PRIVATE (store);

  if (priv->view) {
    g_signal_handler_disconnect (priv->view, priv->sig_added);
    g_signal_handler_disconnect (priv->view, priv->sig_modified);
    g_signal_handler_disconnect (priv->view, priv->sig_removed);
    priv->sig_added = priv->sig_modified = priv->sig_removed = 0;
    
    g_object_unref (priv->view);
    g_object_unref (priv->cal);
    priv->view = NULL;
    priv->cal = NULL;
  }
  
  if (view) {
    priv->view = g_object_ref (view);
    priv->cal = g_object_get_data (G_OBJECT (view), "koto-ecal");
    if (priv->cal)
      g_object_ref (priv->cal);
    priv->sig_added = g_signal_connect (priv->view, "objects-added", G_CALLBACK (on_objects_added), store);
    priv->sig_modified = g_signal_connect (priv->view, "objects-modified", G_CALLBACK (on_objects_modified), store);
    priv->sig_removed = g_signal_connect (priv->view, "objects-removed", G_CALLBACK (on_objects_removed), store);
  }
}

void
koto_task_store_set_done (KotoTaskStore *store, GtkTreeIter *iter, gboolean done)
{
  KotoTaskStorePrivate *priv;
  GError *error = NULL;
  KotoTask *task;

  g_return_if_fail (KOTO_IS_TASK_STORE (store));
  g_return_if_fail (iter);

  priv = GET_PRIVATE (store);
  
  gtk_tree_model_get (GTK_TREE_MODEL (store), iter, COLUMN_ICAL, &task, -1);
  
  icalcomponent_set_status (task->comp, done ? ICAL_STATUS_COMPLETED : ICAL_STATUS_NONE);
  
  gtk_list_store_set (GTK_LIST_STORE (store), iter, COLUMN_DONE, done, -1);

  koto_task_unref (task);
  
  if (priv->cal) {
    if (!e_cal_modify_object (priv->cal, task->comp, CALOBJ_MOD_THIS, &error)) {
      g_warning (G_STRLOC ": cannot modify object: %s", error->message);
      g_error_free (error);
    }
  }
}

gboolean
koto_task_store_get_iter_for_uid (KotoTaskStore *store, const char *uid, GtkTreeIter *iter)
{
  KotoTaskStorePrivate *priv;
  GtkTreeIter *it;

  g_return_val_if_fail (KOTO_IS_TASK_STORE (store), FALSE);
  g_return_val_if_fail (uid, FALSE);
  g_return_val_if_fail (iter, FALSE);

  priv = GET_PRIVATE (store);

  if (g_hash_table_lookup_extended (priv->uid_hash, uid, NULL, (gpointer*)(void*)&it)) {
    *iter = *it;
    return TRUE;
  } else {
    return FALSE;
  }
}
