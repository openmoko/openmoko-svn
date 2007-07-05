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

#ifndef _KOTO_GROUP_COMBO
#define _KOTO_GROUP_COMBO

#include <gtk/gtkcombobox.h>
#include "koto-group-filter-model.h"
#include "koto-group-store.h"

G_BEGIN_DECLS

#define KOTO_TYPE_GROUP_COMBO koto_group_combo_get_type()

#define KOTO_GROUP_COMBO(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  KOTO_TYPE_GROUP_COMBO, KotoGroupCombo))

#define KOTO_GROUP_COMBO_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  KOTO_TYPE_GROUP_COMBO, KotoGroupComboClass))

#define KOTO_IS_GROUP_COMBO(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  KOTO_TYPE_GROUP_COMBO))

#define KOTO_IS_GROUP_COMBO_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  KOTO_TYPE_GROUP_COMBO))

#define KOTO_GROUP_COMBO_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  KOTO_TYPE_GROUP_COMBO, KotoGroupComboClass))

typedef struct {
  GtkComboBox parent;
} KotoGroupCombo;

typedef struct {
  GtkComboBoxClass parent_class;
} KotoGroupComboClass;

GType koto_group_combo_get_type (void);

GtkWidget* koto_group_combo_new (KotoGroupStore *store);

KotoGroup* koto_group_combo_get_active_group (KotoGroupCombo *combo);

void koto_group_combo_connect_filter (KotoGroupCombo *combo, KotoGroupFilterModel *filter);

G_END_DECLS

#endif /* _KOTO_GROUP_COMBO */
