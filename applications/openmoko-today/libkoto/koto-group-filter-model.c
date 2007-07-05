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
#include "koto-task-store.h"
#include "koto-group-filter-model.h"

G_DEFINE_TYPE (KotoGroupFilterModel,
               koto_group_model_filter,
               GTK_TYPE_TREE_MODEL_FILTER);

#define GET_PRIVATE(o)                                                  \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), KOTO_TYPE_GROUP_MODEL_FILTER, KotoGroupFilterModelPrivate))

typedef struct _KotoGroupFilterModelPrivate KotoGroupFilterModelPrivate;

struct _KotoGroupFilterModelPrivate
{
  KotoGroup *group;
};

static gboolean
visible_func (GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
  KotoGroupFilterModelPrivate *priv = GET_PRIVATE (data);
  KotoTask *task = NULL;
  gboolean visible = FALSE;
  
  gtk_tree_model_get (model, iter, COLUMN_ICAL, &task, -1);
  
  if (task == NULL)
    return FALSE;
  
  if (priv->group) {
    visible = koto_group_includes_task (priv->group, task);
    if (!visible)
      goto done;
  }
 done:
  koto_task_unref (task);
  
  return visible;
}

static void
koto_group_model_filter_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_group_model_filter_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
koto_group_model_filter_dispose (GObject *object)
{
  KotoGroupFilterModelPrivate *priv = GET_PRIVATE (object);

  if (priv->group) {
    g_object_unref (priv->group);
    priv->group = NULL;
  }

  if (G_OBJECT_CLASS (koto_group_model_filter_parent_class)->dispose)
    G_OBJECT_CLASS (koto_group_model_filter_parent_class)->dispose (object);
}

static void
koto_group_model_filter_class_init (KotoGroupFilterModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (KotoGroupFilterModelPrivate));

  object_class->get_property = koto_group_model_filter_get_property;
  object_class->set_property = koto_group_model_filter_set_property;
  object_class->dispose = koto_group_model_filter_dispose;
}

static void
koto_group_model_filter_init (KotoGroupFilterModel *self)
{
  gtk_tree_model_filter_set_visible_func
    (GTK_TREE_MODEL_FILTER (self), visible_func, self, NULL);
}

GtkTreeModel *
koto_group_model_filter_new (KotoTaskStore *model)
{
  return g_object_new (KOTO_TYPE_GROUP_MODEL_FILTER,
                       "child-model", model,
                       NULL);
}

void
koto_group_model_filter_set_group (KotoGroupFilterModel *filter, KotoGroup *group)
{
  KotoGroupFilterModelPrivate *priv = GET_PRIVATE (filter);

  g_return_if_fail (KOTO_IS_GROUP_MODEL_FILTER (filter));

  if (priv->group) {
    g_object_unref (priv->group);
    priv->group = NULL;
  }

  if (group) {
    priv->group = g_object_ref (group);
  }
  
  gtk_tree_model_filter_refilter  (GTK_TREE_MODEL_FILTER (filter));
}
