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

#ifndef _KOTO_GROUP_STORE
#define _KOTO_GROUP_STORE

#include <libecal/e-cal-view.h>
#include <gtk/gtkliststore.h>
#include "koto-group.h"

G_BEGIN_DECLS

#define KOTO_TYPE_GROUP_STORE koto_group_store_get_type()

#define KOTO_GROUP_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  KOTO_TYPE_GROUP_STORE, KotoGroupStore))

#define KOTO_GROUP_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  KOTO_TYPE_GROUP_STORE, KotoGroupStoreClass))

#define KOTO_IS_GROUP_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  KOTO_TYPE_GROUP_STORE))

#define KOTO_IS_GROUP_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  KOTO_TYPE_GROUP_STORE))

#define KOTO_GROUP_STORE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  KOTO_TYPE_GROUP_STORE, KotoGroupStoreClass))

enum {
  COL_GROUP, /* KotoGroup */
};

typedef struct {
  GtkListStore parent;
} KotoGroupStore;

typedef struct {
  GtkListStoreClass parent_class;
} KotoGroupStoreClass;

GType koto_group_store_get_type (void);

GtkTreeModel* koto_group_store_new (ECalView *view);

void koto_group_store_set_view (KotoGroupStore *store, ECalView *view);

void koto_group_store_add_new_category (KotoGroupStore *store, GtkTreeIter *iter, const char *name);

void koto_group_store_add_group (KotoGroupStore *store, KotoGroup *group);

gboolean koto_group_store_get_iter_for_group (KotoGroupStore *store, const char *group, GtkTreeIter *iter);

char* koto_group_store_match_group (KotoGroupStore *store, const char *guess);

G_END_DECLS

#endif /* _KOTO_GROUP_STORE */
