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
#include <glib/gi18n.h>
#include "ical-util.h"
#include "koto-no-category-group.h"

G_DEFINE_TYPE (KotoNoCategoryGroup, koto_no_category_group, KOTO_TYPE_GROUP);

static const char *
get_name (KotoGroup *group)
{
  g_return_val_if_fail (KOTO_IS_NO_CATEGORY_GROUP (group), NULL);
  
  return _("No Category");
}

static int
get_weight (KotoGroup *group)
{
  return 100;
}

static gboolean
includes_task (KotoGroup *group, KotoTask *task)
{
  g_return_val_if_fail (KOTO_IS_NO_CATEGORY_GROUP (group), FALSE);
  g_return_val_if_fail (task, FALSE);

  return ical_util_get_categories (task->comp) == NULL ? TRUE : FALSE;
}

static void
koto_no_category_group_class_init (KotoNoCategoryGroupClass *klass)
{
  KotoGroupClass *group_class = KOTO_GROUP_CLASS (klass);

  group_class->get_name = get_name;
  group_class->get_weight = get_weight;
  group_class->includes_task = includes_task;
}

static void
koto_no_category_group_init (KotoNoCategoryGroup *self)
{
}

KotoGroup *
koto_no_category_group_new (void)
{
  return g_object_new (KOTO_TYPE_NO_CATEGORY_GROUP, NULL);
}
