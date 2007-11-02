/*
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Author: OpenedHand Ltd <info@openedhand.com>
 */

#include <string.h>

#include "search-bar.h"

#include "appmanager-data.h"
#include "navigation-area.h"
#include "package-list.h"



static gboolean
combo_seperator_func (GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
  gchar *s;
  gtk_tree_model_get (model, iter, 0, &s, -1);
  return !s;
}

static void
text_changed_cb (MokoSearchBar *searchbar, GtkEditable *editable, ApplicationManagerData *data)
{
}

static void
combo_changed_cb (MokoSearchBar *searchbar, GtkComboBox *combo, ApplicationManagerData *data)
{
  int ret;
  gchar *secname;
  GtkTreeIter iter;
  gpointer pkglist;
  
  if ((ret = gtk_combo_box_get_active (combo)) < 5)
  {
    switch (ret)
    {
      case 0:
        /* installed */
        g_debug ("Clicked the installed menuitem");
        pkglist = application_manager_data_get_installedlist (data);
        navigation_area_refresh_with_package_list (data, pkglist);
        return;
      case 1:  
        /* upgradeable */
        g_debug ("Clicked the upgradeable menuitem");
        pkglist = application_manager_data_get_upgradelist (data);
        navigation_area_refresh_with_package_list (data, pkglist);
        return;
      case 2:
        /* selected */
        g_debug ("Click the selected menuitem");
        pkglist = application_manager_data_get_selectedlist (data);
        navigation_area_refresh_with_package_list (data, pkglist);
        return;
    }
  }

  gtk_combo_box_get_active_iter (combo, &iter);
  gtk_tree_model_get (data->filter_store, &iter, 0, &secname, -1);
  
  ret = strcmp (secname, PACKAGE_LIST_NO_SECTION_STRING);
  if (ret == 0)
  {
    pkglist = application_manager_data_get_nosecpkglist (
                    MOKO_APPLICATION_MANAGER_DATA (data));
    navigation_area_refresh_with_package_list (
                    MOKO_APPLICATION_MANAGER_DATA (data), 
                    pkglist);
    return;
  }

  pkglist = package_list_get_with_name (
          MOKO_APPLICATION_MANAGER_DATA (data),
          secname);
  if (pkglist == NULL)
  {
    g_debug ("Can not find the section that named:%s", secname);
    return;
  }
  navigation_area_refresh_with_package_list (
          MOKO_APPLICATION_MANAGER_DATA (data),
          pkglist);
}

static void
searchbar_toggled_cb (MokoSearchBar *searchbar, gboolean search, ApplicationManagerData *data)
{
}

GtkWidget *
search_bar_new (ApplicationManagerData *appdata)
{
  GtkWidget *combo;
  GtkWidget *searchbar;
  GtkListStore *filter;
  GtkCellRenderer *renderer;

  filter = gtk_list_store_new (1, G_TYPE_STRING);
  appdata->filter_store = GTK_TREE_MODEL (filter);

  gtk_list_store_insert_with_values (filter, NULL, FILTER_INSTALLED, 0, "Installed", -1);
  gtk_list_store_insert_with_values (filter, NULL, FILTER_UPGRADEABLE, 0, "Upgradeable", -1);
  gtk_list_store_insert_with_values (filter, NULL, FILTER_SELECTED, 0, "Selected", -1);
  gtk_list_store_insert_with_values (filter, NULL, 3, 0, NULL, -1);
  
  renderer = gtk_cell_renderer_text_new ();
  
  combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (filter));
  gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), combo_seperator_func, NULL, NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), renderer, "text", 0);
  
  searchbar = moko_search_bar_new_with_combo (GTK_COMBO_BOX (combo));
  g_signal_connect (searchbar, "combo-changed", G_CALLBACK (combo_changed_cb), appdata);
  
  return searchbar;
}


void
search_bar_add_filter_item (ApplicationManagerData *appdata, gchar *item)
{
  gtk_list_store_insert_with_values (GTK_LIST_STORE (appdata->filter_store), NULL, -1, 0, item, -1);
}

void
search_bar_set_active_filter (MokoSearchBar *bar, SearchBarFilter filter)
{
  GtkComboBox *combo;
  
  combo = moko_search_bar_get_combo_box (bar);
  gtk_combo_box_set_active (combo, filter);
}
