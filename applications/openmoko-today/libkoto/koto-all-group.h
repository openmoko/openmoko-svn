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

#ifndef _KOTO_ALL_GROUP
#define _KOTO_ALL_GROUP

#include "koto-group.h"

G_BEGIN_DECLS

#define KOTO_TYPE_ALL_GROUP koto_all_group_get_type()

#define KOTO_ALL_GROUP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  KOTO_TYPE_ALL_GROUP, KotoAllGroup))

#define KOTO_ALL_GROUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  KOTO_TYPE_ALL_GROUP, KotoAllGroupClass))

#define KOTO_IS_ALL_GROUP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  KOTO_TYPE_ALL_GROUP))

#define KOTO_IS_ALL_GROUP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  KOTO_TYPE_ALL_GROUP))

#define KOTO_ALL_GROUP_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  KOTO_TYPE_ALL_GROUP, KotoAllGroupClass))

typedef struct {
  KotoGroup parent;
} KotoAllGroup;

typedef struct {
  KotoGroupClass parent_class;
} KotoAllGroupClass;

GType koto_all_group_get_type (void);

KotoGroup* koto_all_group_new (void);

G_END_DECLS

#endif /* _KOTO_ALL_GROUP */
