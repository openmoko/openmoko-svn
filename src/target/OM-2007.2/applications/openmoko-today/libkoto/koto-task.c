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
#include "koto-task.h"
#include "ical-util.h"

GType
koto_task_get_type (void)
{
  static GType type = 0;
  if (G_UNLIKELY (type == 0)) {
    type = g_boxed_type_register_static ("KotoTask",
                                         (GBoxedCopyFunc)koto_task_ref,
                                         (GBoxedFreeFunc)koto_task_unref);
  }
  return type;
}

KotoTask *
koto_task_new (icalcomponent *comp)
{
  KotoTask *task;
  const char *s;

  g_return_val_if_fail (comp, NULL);
  
  task = g_slice_new0 (KotoTask);
  task->ref_count = 1;
  task->comp = comp;
  
  /* Set the caches */
  s = ical_util_get_categories (comp);
  if (s)
    task->categories = g_strsplit (s, ",", 0);
  
  return task;
}

KotoTask *
koto_task_ref (KotoTask *task)
{
  g_return_val_if_fail (task, NULL);
  g_return_val_if_fail (task->ref_count > 0, NULL);

  g_atomic_int_inc (&task->ref_count);

  return task;
}

void
koto_task_unref (KotoTask *task)
{
  g_return_if_fail (task);
  g_return_if_fail (task->ref_count > 0);

  if (g_atomic_int_dec_and_test (&task->ref_count)) {
    icalcomponent_free (task->comp);
    g_strfreev (task->categories);
    g_slice_free (KotoTask, task);
  }
}
