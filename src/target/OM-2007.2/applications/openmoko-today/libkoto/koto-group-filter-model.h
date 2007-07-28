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

#ifndef _KOTO_GROUP_MODEL_FILTER
#define _KOTO_GROUP_MODEL_FILTER

#include <gtk/gtktreemodelfilter.h>
#include "koto-group.h"
#include "koto-task-store.h"

G_BEGIN_DECLS

#define KOTO_TYPE_GROUP_MODEL_FILTER koto_group_model_filter_get_type()

#define KOTO_GROUP_MODEL_FILTER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  KOTO_TYPE_GROUP_MODEL_FILTER, KotoGroupFilterModel))

#define KOTO_GROUP_MODEL_FILTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  KOTO_TYPE_GROUP_MODEL_FILTER, KotoGroupFilterModelClass))

#define KOTO_IS_GROUP_MODEL_FILTER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  KOTO_TYPE_GROUP_MODEL_FILTER))

#define KOTO_IS_GROUP_MODEL_FILTER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  KOTO_TYPE_GROUP_MODEL_FILTER))

#define KOTO_GROUP_MODEL_FILTER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  KOTO_TYPE_GROUP_MODEL_FILTER, KotoGroupFilterModelClass))

typedef struct {
  GtkTreeModelFilter parent;
} KotoGroupFilterModel;

typedef struct {
  GtkTreeModelFilterClass parent_class;
} KotoGroupFilterModelClass;

GType koto_group_model_filter_get_type (void);

GtkTreeModel* koto_group_model_filter_new (KotoTaskStore *model);

void koto_group_model_filter_set_group (KotoGroupFilterModel *filter, KotoGroup *group);

G_END_DECLS

#endif /* _KOTO_GROUP_MODEL_FILTER */
