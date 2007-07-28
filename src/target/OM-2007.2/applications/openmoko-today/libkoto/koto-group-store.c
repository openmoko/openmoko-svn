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
#include <glib/gi18n.h>
#include <libecal/e-cal-view.h>
#include <libedataserver/e-data-server-util.h>

#include "ical-util.h"
#include "koto-group-store.h"
#include "koto-group.h"
#include "koto-category-group.h"

G_DEFINE_TYPE (KotoGroupStore, koto_group_store, GTK_TYPE_LIST_STORE);

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), KOTO_TYPE_GROUP_STORE, KotoGroupStorePrivate))

typedef struct
{
  ECalView *view;
  /** A hash of group names to GtkTreeIter* */
  GHashTable *iter_hash;
  guint sig_added, sig_modified;
} KotoGroupStorePrivate;

enum {
  PROP_NULL,
  PROP_CALVIEW,
};

/*
 * Private methods.
 */

static void
on_objects_added (ECalView *view, GList *objects, KotoGroupStore *store)
{
  for (;objects; objects = g_list_next (objects)) {
    icalcomponent *comp = objects->data;
    char **categories, **l;
    
    categories = g_strsplit (ical_util_get_categories (comp) ?: "", ",", 0);

    for (l = categories; *l; l++) {
      koto_group_store_add_new_category (store, NULL, *l);
    }

    g_strfreev (categories);
  }
}

static gint
sorter_cb (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
  KotoGroup *group_a, *group_b;
  int res, weight_a, weight_b;

  gtk_tree_model_get (model, a, COL_GROUP, &group_a, -1);
  gtk_tree_model_get (model, b, COL_GROUP, &group_b, -1);

  weight_a = koto_group_get_weight (group_a);
  weight_b = koto_group_get_weight (group_b);

  if (weight_a != weight_b) {
    res = weight_a - weight_b;
    goto done;
  }
  
  res = g_utf8_collate (koto_group_get_name (group_a) ?: "",
                        koto_group_get_name (group_b) ?: "");

 done:
  g_object_unref (group_a);
  g_object_unref (group_b);

  return res;
}


/*
 * Object methods.
 */

static void
koto_group_store_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  KotoGroupStorePrivate *priv = GET_PRIVATE (object);

  switch (property_id) {
  case PROP_CALVIEW:
    g_value_set_object (value, priv->view);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_group_store_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_CALVIEW:
    koto_group_store_set_view (KOTO_GROUP_STORE (object),
                               g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_group_store_dispose (GObject *object)
{
  koto_group_store_set_view (KOTO_GROUP_STORE (object), NULL);

  if (G_OBJECT_CLASS (koto_group_store_parent_class)->dispose)
    G_OBJECT_CLASS (koto_group_store_parent_class)->dispose (object);
}

static void
koto_group_store_finalize (GObject *object)
{
  KotoGroupStorePrivate *priv = GET_PRIVATE (object);

  g_hash_table_destroy (priv->iter_hash);
  
  G_OBJECT_CLASS (koto_group_store_parent_class)->finalize (object);
}

static void
koto_group_store_class_init (KotoGroupStoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (KotoGroupStorePrivate));

  object_class->get_property = koto_group_store_get_property;
  object_class->set_property = koto_group_store_set_property;
  object_class->dispose = koto_group_store_dispose;
  object_class->finalize = koto_group_store_finalize;

  g_object_class_install_property (object_class, PROP_CALVIEW,
                                   g_param_spec_object ("calview", "calview", NULL,
                                                        E_TYPE_CAL_VIEW,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK));
}

static void
koto_group_store_init (KotoGroupStore *self)
{
  const GType column_types[] = { KOTO_TYPE_GROUP };
  KotoGroupStorePrivate *priv = GET_PRIVATE (self);

  gtk_list_store_set_column_types (GTK_LIST_STORE (self),
                                   G_N_ELEMENTS (column_types),
                                   (GType *) column_types);

  gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (self),
                                           sorter_cb, NULL, NULL);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (self),
                                        GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
                                        GTK_SORT_ASCENDING);
  
  priv->iter_hash = g_hash_table_new_full (g_str_hash, g_str_equal,
                                      g_free, (GDestroyNotify)gtk_tree_iter_free);
}


/*
 * Public methods.
 */

GtkTreeModel *
koto_group_store_new (ECalView *view)
{
  return g_object_new (KOTO_TYPE_GROUP_STORE, "calview", view, NULL);
}

void
koto_group_store_set_view (KotoGroupStore *store, ECalView *view)
{
  KotoGroupStorePrivate *priv;

  g_return_if_fail (KOTO_IS_GROUP_STORE (store));

  priv = GET_PRIVATE (store);

  if (priv->view) {
    g_signal_handler_disconnect (priv->view, priv->sig_added);
    g_signal_handler_disconnect (priv->view, priv->sig_modified);
    priv->sig_added = priv->sig_modified = 0;
    g_object_unref (priv->view);
    priv->view = NULL;
  }
  
  if (view) {
    priv->view = g_object_ref (view);
    priv->sig_added = g_signal_connect (priv->view, "objects-added", G_CALLBACK (on_objects_added), store);
    priv->sig_modified = g_signal_connect (priv->view, "objects-modified", G_CALLBACK (on_objects_added), store);
  }
}

gboolean
koto_group_store_get_iter_for_group (KotoGroupStore *store, const char *group, GtkTreeIter *iter)
{
  KotoGroupStorePrivate *priv;
  GtkTreeIter *it;

  g_return_val_if_fail (KOTO_IS_GROUP_STORE (store), FALSE);

  priv = GET_PRIVATE (store);

  /* Handle empty groups by returning the first row (All or None) */
  if (group == NULL || group[0] == '\0') {
    return gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), iter);
  }
  
  if (g_hash_table_lookup_extended (priv->iter_hash, group, NULL, (gpointer*)(void*)&it)) {
    *iter = *it;
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
 * koto_group_store_add_new_group:
 * @store: A #KotoGroupStore.
 * @iter: An optional #GtkTreeIter, which will be set to point at the new group.
 * @name: The name of the group to create.
 *
 * Create a new category  called @name and set @iter to point at it.  If
 * the group already exists, @iter will point at the existing group.
 */
void
koto_group_store_add_new_category (KotoGroupStore *store, GtkTreeIter *iter, const char *name)
{
  /* TODO: merge this with add_group below */
  KotoGroupStorePrivate *priv;
  GtkTreeIter *current_it, new_it;
  KotoGroup *group;

  g_return_if_fail (KOTO_IS_GROUP_STORE (store));
  g_return_if_fail (name);
  
  priv = GET_PRIVATE (store);
  
  current_it = g_hash_table_lookup (priv->iter_hash, name);
  if (current_it) {
    if (iter) *iter = *current_it;
  } else {
    group = koto_category_group_new (name);

    gtk_list_store_insert_with_values (GTK_LIST_STORE (store), &new_it, 0,
                                       COL_GROUP, group, -1);
    g_hash_table_insert (priv->iter_hash, g_strdup (name), gtk_tree_iter_copy (&new_it));

    g_object_unref (group);
    
    if (iter) *iter = new_it;
  }
}

/*
 * Add an arbitrary group to the store.
 */
void
koto_group_store_add_group (KotoGroupStore *store, KotoGroup *group)
{
  KotoGroupStorePrivate *priv;
  GtkTreeIter iter;

  g_return_if_fail (KOTO_IS_GROUP_STORE (store));
  g_return_if_fail (KOTO_IS_GROUP (group));
  
  priv = GET_PRIVATE (store);

  gtk_list_store_insert_with_values (GTK_LIST_STORE (store), &iter, 0, COL_GROUP, group, -1);
  //TODO? g_hash_table_insert (priv->iter_hash, g_strdup (name), gtk_tree_iter_copy (&new_it));
}

/* Helper struct for koto_group_store_match_group */
typedef struct {
  const char *guess;
  char *found;
} MatchData;

/* Helper function for koto_group_store_match_group */
static gboolean
find_group (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
  MatchData *data = user_data;
  KotoGroup *group;
  gboolean ret = FALSE;

  gtk_tree_model_get (model, iter, COL_GROUP, &group, -1);

  if (g_type_is_a (G_OBJECT_TYPE (group), KOTO_TYPE_CATEGORY_GROUP)) {
    if (e_util_utf8_strstrcasedecomp (koto_group_get_name (group), data->guess)) {
      data->found = g_strdup (koto_group_get_name (group));
      ret = TRUE;
    }
  }
  
  g_object_unref (group);
  return ret;
}

/**
 * koto_group_store_match_group:
 * @store: A #KotoGroupStore.
 * @guess: The group name to search for.
 *
 * Seach the groups for a group named roughly @guess.  If one is found, return
 * it as a newly allocated string, otherwise return NULL.
 *
 * Currently "named roughly" means case-insensitive stripped decomposed
 * substrings.
 */
char *
koto_group_store_match_group (KotoGroupStore *store, const char *guess)
{
  MatchData data;

  g_return_val_if_fail (KOTO_IS_GROUP_STORE (store), NULL);
  g_return_val_if_fail (guess, NULL);

  data.guess = guess;
  data.found = NULL;

  gtk_tree_model_foreach (GTK_TREE_MODEL (store), find_group, &data);

  return data.found;
}



#if WITH_TESTS

#include <string.h>

int main (int argc, char **argv)
{
  KotoGroupStore *store;
  GtkTreeIter iter;

  g_type_init ();

  store = (KotoGroupStore*)koto_group_store_new
    (KOTO_GROUP_STORE_TYPE_FILTER, NULL);
  g_assert (store);

  koto_group_store_add_new_group (store, NULL, "Foo");
  koto_group_store_add_new_group (store, NULL, "Bar");

  g_assert (koto_group_store_get_iter_for_group (store, "Foo", &iter));
  g_assert (koto_group_store_get_iter_for_group (store, "Bar", &iter));
  g_assert (koto_group_store_get_iter_for_group (store, "Flob", &iter) == FALSE);
  
  g_assert (koto_group_store_match_group (store, "Flob") == NULL);
  g_assert (strcmp (koto_group_store_match_group (store, "Foo"), "Foo") == 0);
  g_assert (strcmp (koto_group_store_match_group (store, "foo"), "Foo") == 0);
  g_assert (strcmp (koto_group_store_match_group (store, "baR"), "Bar") == 0);
  
  return 0;
}
#endif
