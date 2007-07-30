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
#include <glib/gi18n.h>
#include "koto-meta-group.h"

G_DEFINE_TYPE (KotoMetaGroup, koto_meta_group, KOTO_TYPE_GROUP);

#define GET_PRIVATE(o)                                                  \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), KOTO_TYPE_META_GROUP, KotoMetaGroupPrivate))

typedef struct {
  KotoMetaGroupKind kind;
  int weight;
} KotoMetaGroupPrivate;

static const char *
get_name (KotoGroup *group)
{
  KotoMetaGroupPrivate *priv;

  g_return_val_if_fail (KOTO_IS_META_GROUP (group), NULL);
  
  priv = GET_PRIVATE (group);

  switch (priv->kind) {
  case KOTO_META_GROUP_SEPERATOR:
    return "";
  case KOTO_META_GROUP_NONE:
    return _("None");
  case KOTO_META_GROUP_NEW:
    return _("New Group...");
  }

  return NULL;
}

static int
get_weight (KotoGroup *group)
{
  g_return_val_if_fail (KOTO_IS_META_GROUP (group), 0);

  return GET_PRIVATE (group)->weight;
}

static void
koto_meta_group_class_init (KotoMetaGroupClass *klass)
{
  KotoGroupClass *group_class = KOTO_GROUP_CLASS (klass);

  g_type_class_add_private (klass, sizeof (KotoMetaGroupPrivate));
  
  group_class->get_name = get_name;
  group_class->get_weight = get_weight;
}

static void
koto_meta_group_init (KotoMetaGroup *self)
{
}

KotoGroup *
koto_meta_group_new (KotoMetaGroupKind kind, int weight)
{
  KotoGroup *group;

  group = g_object_new (KOTO_TYPE_META_GROUP, NULL);
  /* TODO: set a property */
  GET_PRIVATE (group)->kind = kind;
  GET_PRIVATE (group)->weight = weight;

  return group;
}

KotoMetaGroupKind
koto_meta_group_get_kind (KotoMetaGroup *group)
{
  g_return_val_if_fail (KOTO_IS_META_GROUP (group), 0);

  return GET_PRIVATE (group)->kind;
}
