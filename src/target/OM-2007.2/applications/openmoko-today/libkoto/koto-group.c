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
#include "koto-group.h"

G_DEFINE_ABSTRACT_TYPE (KotoGroup, koto_group, G_TYPE_OBJECT);

static void
koto_group_class_init (KotoGroupClass *klass)
{}

static void
koto_group_init (KotoGroup *self)
{}

const char *
koto_group_get_name (KotoGroup *group)
{
  if (KOTO_GROUP_GET_CLASS (group)->get_name)
    return KOTO_GROUP_GET_CLASS (group)->get_name (group);

  g_warning ("KotoGroup %s doesn't implement get_name", G_OBJECT_TYPE_NAME (group));
  return NULL;
}

int
koto_group_get_weight (KotoGroup *group)
{
  if (KOTO_GROUP_GET_CLASS (group)->get_weight)
    return KOTO_GROUP_GET_CLASS (group)->get_weight (group);

  g_warning ("%s doesn't implement get_weight", G_OBJECT_TYPE_NAME (group));
  return 0;
}

gboolean
koto_group_includes_task (KotoGroup *group, KotoTask *task)
{
  if (KOTO_GROUP_GET_CLASS (group)->includes_task)
    return KOTO_GROUP_GET_CLASS (group)->includes_task (group, task);

  g_warning ("%s doesn't implement includes_task", G_OBJECT_TYPE_NAME (group));
  return FALSE;
}
