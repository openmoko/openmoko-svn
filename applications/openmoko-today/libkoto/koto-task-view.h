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

#ifndef _KOTO_TASK_VIEW
#define _KOTO_TASK_VIEW

#include <gtk/gtktreeview.h>
#include "koto-group-filter-model.h"
#include "koto-task-store.h"
#include "koto-task.h"

G_BEGIN_DECLS

#define KOTO_TYPE_TASK_VIEW koto_task_view_get_type()

#define KOTO_TASK_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  KOTO_TYPE_TASK_VIEW, KotoTaskView))

#define KOTO_TASK_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  KOTO_TYPE_TASK_VIEW, KotoTaskViewClass))

#define KOTO_IS_TASK_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  KOTO_TYPE_TASK_VIEW))

#define KOTO_IS_TASK_VIEW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  KOTO_TYPE_TASK_VIEW))

#define KOTO_TASK_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  KOTO_TYPE_TASK_VIEW, KotoTaskViewClass))

typedef struct {
  GtkTreeView parent;
} KotoTaskView;

typedef struct {
  GtkTreeViewClass parent_class;
} KotoTaskViewClass;

GType koto_task_view_get_type (void);

GtkWidget * koto_task_view_new (KotoTaskStore *store, KotoGroupFilterModel *filter);

KotoTask * koto_task_view_get_selected_task (KotoTaskView *view);

gboolean koto_task_view_get_selected_iter (KotoTaskView *view, GtkTreeIter *iter);

G_END_DECLS

#endif /* _KOTO_TASK_VIEW */
