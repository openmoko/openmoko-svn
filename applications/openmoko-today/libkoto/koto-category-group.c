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
#include "ical-util.h"
#include "koto-category-group.h"

G_DEFINE_TYPE (KotoCategoryGroup, koto_category_group, KOTO_TYPE_GROUP);

#define GET_PRIVATE(o)                                                  \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), KOTO_TYPE_CATEGORY_GROUP, KotoCategoryGroupPrivate))

typedef struct {
  char *name;
} KotoCategoryGroupPrivate;

static const char *
get_name (KotoGroup *group)
{
  g_return_val_if_fail (KOTO_IS_CATEGORY_GROUP (group), NULL);
  
  return GET_PRIVATE (group)->name;
}

static int
get_weight (KotoGroup *group)
{
  return 0;
}

static gboolean
includes_task (KotoGroup *group, KotoTask *task)
{
  KotoCategoryGroupPrivate *priv;
  char **l;
  gboolean visible = FALSE;

  g_return_val_if_fail (KOTO_IS_CATEGORY_GROUP (group), FALSE);
  g_return_val_if_fail (task, FALSE);

  priv = GET_PRIVATE (group);
  
  /* Handle no categories */
  if (task->categories == NULL)
    return FALSE;
  
  for (l = task->categories; *l ; l++) {
    /* TODO: strmp? or funky decomposition compare? */
    if (strcmp (priv->name, *l) == 0) {
      visible = TRUE;
      break;
    }
  }
  
  return visible;
}

static void
koto_category_group_class_init (KotoCategoryGroupClass *klass)
{
  KotoGroupClass *group_class = KOTO_GROUP_CLASS (klass);

  g_type_class_add_private (klass, sizeof (KotoCategoryGroupPrivate));
  
  group_class->get_name = get_name;
  group_class->get_weight = get_weight;
  group_class->includes_task = includes_task;
}

static void
koto_category_group_init (KotoCategoryGroup *self)
{
}

KotoGroup *
koto_category_group_new (const char *name)
{
  KotoGroup *group;

  g_return_val_if_fail (name, NULL);

  group = g_object_new (KOTO_TYPE_CATEGORY_GROUP, NULL);
  /* TODO: set a property */
  GET_PRIVATE (group)->name = g_strdup (name);

  return group;
}
