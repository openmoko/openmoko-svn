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

#ifndef _KOTO_TASK
#define _KOTO_TASK

#include <glib-object.h>
#include <libical/icalcomponent.h>

G_BEGIN_DECLS

typedef struct {
  /* Reference count */
  volatile gint ref_count;
  /* The real data */
  icalcomponent *comp;
  /* The following are read-only caches */
  char **categories;
} KotoTask;

#define KOTO_TYPE_TASK koto_task_get_type ()

GType koto_task_get_type (void);

KotoTask *koto_task_new (icalcomponent *comp);

KotoTask *koto_task_ref (KotoTask *task);

void koto_task_unref (KotoTask *task);

G_END_DECLS

#endif /* _KOTO_TASK */
