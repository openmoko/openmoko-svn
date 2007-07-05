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

#ifndef _KOTO_GROUP
#define _KOTO_GROUP

#include "koto-task.h"

G_BEGIN_DECLS

#define KOTO_TYPE_GROUP koto_group_get_type()

#define KOTO_GROUP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  KOTO_TYPE_GROUP, KotoGroup))

#define KOTO_GROUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  KOTO_TYPE_GROUP, KotoGroupClass))

#define KOTO_IS_GROUP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  KOTO_TYPE_GROUP))

#define KOTO_IS_GROUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  KOTO_TYPE_GROUP))

#define KOTO_GROUP_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  KOTO_TYPE_GROUP, KotoGroupClass))

typedef struct {
  GObject parent;
} KotoGroup;

typedef struct {
  GObjectClass parent_class;
  const char * (* get_name) (KotoGroup *group);
  int (* get_weight) (KotoGroup *group);
  gboolean (* includes_task) (KotoGroup *group, KotoTask *task);
} KotoGroupClass;

GType koto_group_get_type (void);

const char *koto_group_get_name (KotoGroup *group);

int koto_group_get_weight (KotoGroup *group);

gboolean koto_group_includes_task (KotoGroup *group, KotoTask *task);

G_END_DECLS

#endif /* _KOTO_GROUP */
