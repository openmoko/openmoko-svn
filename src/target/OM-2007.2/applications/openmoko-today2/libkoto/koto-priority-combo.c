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

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "koto-priority-combo.h"
#include "ical-util.h"

G_DEFINE_TYPE (KotoPriorityCombo, koto_priority_combo, GTK_TYPE_COMBO_BOX);

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), KOTO_TYPE_PRIORITY_COMBO, KotoPriorityComboPrivate))

typedef struct {
  guint changed_id;
  int priority;
} KotoPriorityComboPrivate;

static void
combo_changed (GtkComboBox *combo)
{
  KotoPriorityComboPrivate *priv = GET_PRIVATE (combo);
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  model = gtk_combo_box_get_model (combo);
  gtk_combo_box_get_active_iter (combo, &iter);

  gtk_tree_model_get (model, &iter, 1, &priv->priority, -1);
}

static void
koto_priority_combo_class_init (KotoPriorityComboClass *klass)
{
  g_type_class_add_private (klass, sizeof (KotoPriorityComboPrivate));
}

static void
koto_priority_combo_init (KotoPriorityCombo *self)
{
  KotoPriorityComboPrivate *priv;
  GtkListStore *store;
  GtkCellRenderer *cell;
  /* GTK+ 2.8 requires an iterator */
  GtkTreeIter iter;

  priv = GET_PRIVATE (self);

  priv->changed_id = g_signal_connect (self, "changed", G_CALLBACK (combo_changed), NULL);

  store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  g_object_set (self, "model", store, NULL);
  g_object_unref (store);

  gtk_list_store_insert_with_values (store, &iter, 0,
                                     0, _("Low"), 1, PRIORITY_LOW, -1);
  gtk_list_store_insert_with_values (store, &iter, 0,
                                     0, _("Normal"), 1, PRIORITY_MEDIUM, -1);
  gtk_list_store_insert_with_values (store, &iter, 0,
                                     0, _("High"), 1, PRIORITY_HIGH, -1);

  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self), cell, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (self), cell,
                                  "text", 0,
                                  NULL);
}

GtkWidget *
koto_priority_combo_new (void)
{
  return g_object_new (KOTO_TYPE_PRIORITY_COMBO, NULL);
}

void
koto_priority_combo_set_priority (KotoPriorityCombo *combo, int priority)
{
  KotoPriorityComboPrivate *priv;
  int index = 0;

  g_return_if_fail (KOTO_IS_PRIORITY_COMBO (combo));

  priv = GET_PRIVATE (combo);

  priv->priority = priority;

  if (priority == PRIORITY_NONE) {
    index = 1;
  } else if (priority < PRIORITY_MEDIUM) {
    index = 0;
  } else if (priority == PRIORITY_MEDIUM) {
    index = 1;
  } else if (priority > PRIORITY_MEDIUM) {
    index = 2;
  }

  g_signal_handler_block (combo, priv->changed_id);
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), index);
  g_signal_handler_unblock (combo, priv->changed_id);
}

int
koto_priority_combo_get_priority (KotoPriorityCombo *combo)
{
  KotoPriorityComboPrivate *priv;
  
  g_return_val_if_fail (KOTO_IS_PRIORITY_COMBO (combo), PRIORITY_NONE);
  
  priv = GET_PRIVATE (combo);

  return priv->priority;
}
