/*
 * OpenMoko Appearance - Change various appearance related settings
 *
 * Copyright (C) 2007 by OpenMoko, Inc.
 * Written by OpenedHand Ltd <info@openedhand.com>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include "appearance.h"

struct color
{
  gchar *name;
  gchar *value;
} color_list[] =
{
  /* apart from the OpenMoko colour, these are from the Tango project */
  {"OpenMoko", "#ff7d00"},
  {"Chameleon", "#8ae234"},
  {"Scarlet", "#ef2929"},
  {"Plumb", "#ad7fa8"},
  {"Sky", "#729fcf"},
  {"Aluminium", "#888a85"}
};

/* function declerations */
static void colors_combo_changed_cb (GtkComboBox *widget, AppearanceData *data);

/* "public" functions */
GtkWidget *
colors_page_new (AppearanceData *data)
{
  GtkWidget *table, *w;
  int i;

  table = gtk_table_new (2, 2, FALSE);
  /* setting these padding values here is not ideal but unfortunatly it is not
   * possible to do this in the theme */
  gtk_container_set_border_width (GTK_CONTAINER (table), 12);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);

  w = gtk_label_new ("Highlight:");
  gtk_table_attach (GTK_TABLE (table), w, 0, 1, 0, 1, 0, 0, 0, 0);

  data->colors_combo = gtk_combo_box_new_text ();
  /* add the items for the combo box */
  for (i = 0; i < G_N_ELEMENTS (color_list); i++)
  {
    gtk_combo_box_append_text (GTK_COMBO_BOX (data->colors_combo), color_list[i].name);
  }
  /* connect to the "changed" event of the combo box */
  g_signal_connect (data->colors_combo, "changed", G_CALLBACK (colors_combo_changed_cb), data);
  gtk_table_attach (GTK_TABLE (table), data->colors_combo, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND, 0, 0, 0);

  return table;
}

/* private functions */
static void
colors_combo_changed_cb (GtkComboBox *widget, AppearanceData *data)
{
  GConfClient *client;
  gchar *value;
  gint index;

  index = gtk_combo_box_get_active (widget);
  value = g_strconcat ("selected_bg_color:", color_list[index].value, NULL);

  client = gconf_client_get_default ();
  gconf_client_set_string (client, "/desktop/poky/interface/gtk_color_scheme", value, NULL);

  g_free (value);
  g_object_unref (client);
}

