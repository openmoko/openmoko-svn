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

#ifndef _KOTO_TASK_STORE
#define _KOTO_TASK_STORE

#include <gtk/gtkliststore.h>
#include <libecal/e-cal-view.h>
#include "koto-task.h"

G_BEGIN_DECLS

#define KOTO_TYPE_TASK_STORE koto_task_store_get_type()

#define KOTO_TASK_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  KOTO_TYPE_TASK_STORE, KotoTaskStore))

#define KOTO_TASK_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  KOTO_TYPE_TASK_STORE, KotoTaskStoreClass))

#define KOTO_IS_TASK_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  KOTO_TYPE_TASK_STORE))

#define KOTO_IS_TASK_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  KOTO_TYPE_TASK_STORE))

#define KOTO_TASK_STORE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  KOTO_TYPE_TASK_STORE, KotoTaskStoreClass))

enum {
  COLUMN_ICAL, /* an icalcomponent. There is no boxing so free it when removing */
  COLUMN_DONE, /* gboolean */
  COLUMN_WEIGHT, /* int, effective weight */
  COLUMN_PRIORITY, /* int */
  COLUMN_DUE, /* GDate */
  COLUMN_SUMMARY, /* string */
  COLUMN_URL, /* string */
};

typedef struct {
  GtkListStore parent;
} KotoTaskStore;

typedef struct {
  GtkListStoreClass parent_class;
} KotoTaskStoreClass;

GType koto_task_store_get_type (void);

GtkTreeModel* koto_task_store_new (ECalView *view);

void koto_task_store_set_view (KotoTaskStore *store, ECalView *view);

void koto_task_store_set_done (KotoTaskStore *store, GtkTreeIter *iter, gboolean done);

gboolean koto_task_store_get_iter_for_uid (KotoTaskStore *store, const char *uid, GtkTreeIter *iter);

G_END_DECLS

#endif /* _KOTO_TASK_STORE */
