/**
 *  @file filter-menu.c
 *  @brief The filter menu item
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 *  @author Chaowei Song (songcw@fic-sh.com.cn)
 */

#include "filter-menu.h"
#include "appmanager-window.h"

/**
 * @brief The callback function of the search result menuitem.
 */
void 
on_search_result_activate (GtkMenuItem *menuitem, gpointer userdata)
{
  g_debug ("Clicked the search result menuitem");
}

/**
 * @brief The callback function of the installed menuitem.
 */
void 
on_installed_activate (GtkMenuItem *menuitem, gpointer userdata)
{
  g_debug ("Clicked the installed menuitem");
}

/**
 * @brief The callback function of the upgradeable menuitem.
 */
void 
on_upgradeable_activate (GtkMenuItem *menuitem, gpointer userdata)
{
  g_debug ("Clicked the upgradeable menuitem");
}

/**
 * @brief The callback function of the selected menuitem.
 */
void 
on_selected_activate (GtkMenuItem *menuitem, gpointer userdata)
{
  g_debug ("Click the selected menuitem");
}

/**
 * @brief The callback function of the dynamic menuitem.
 */
void 
on_dynamic_menu_item_activate (GtkMenuItem *menuitem, gpointer userdata)
{
  g_debug ("Click the dynamic menuitem");
}

/**
 * @brief Create a new filter menu for the application manager
 * 
 * At this function, it only can create the static menu items.
 * It must add the dynamic menu items late.
 * @param appdata The application manager data
 * @return The filter menu.
 */
GtkMenu *
filter_menu_new (ApplicationManagerData *appdata)
{
  GtkMenu   *filtermenu;
  GtkWidget *searchresult;
  GtkWidget *installed;
  GtkWidget *upgradeable;
  GtkWidget *selected;

  g_debug ("Init the filter filtermenu");
  g_return_val_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (appdata), NULL);

  filtermenu = GTK_MENU (gtk_menu_new ());

  searchresult = gtk_menu_item_new_with_label (_("Search Results"));
  gtk_widget_show (searchresult);
  gtk_container_add (GTK_CONTAINER (filtermenu), searchresult);
  g_signal_connect ((gpointer) searchresult, "activate",
                    G_CALLBACK (on_search_result_activate), appdata);

  installed = gtk_menu_item_new_with_label (_("Installed"));
  gtk_widget_show (installed);
  gtk_container_add (GTK_CONTAINER (filtermenu), installed);
  g_signal_connect ((gpointer) installed, "activate",
                    G_CALLBACK (on_installed_activate), appdata);

  upgradeable = gtk_menu_item_new_with_label (_("Upgradeable"));
  gtk_widget_show (upgradeable);
  gtk_container_add (GTK_CONTAINER (filtermenu), upgradeable);
  g_signal_connect ((gpointer) upgradeable, "activate",
                    G_CALLBACK (on_upgradeable_activate), appdata);

  selected = gtk_menu_item_new_with_label (_("Selected"));
  gtk_widget_show (selected);
  gtk_container_add (GTK_CONTAINER (filtermenu), selected);
  g_signal_connect ((gpointer) selected, "activate",
                    G_CALLBACK (on_selected_activate), appdata);

  return filtermenu;
}

/**
 * @brief Add a menu item to the filter menu
 * 
 * @param filtermenu The filter menu
 * @param name The label name of the menu item
 */
void 
filter_menu_add_item (GtkMenu *filtermenu, const gchar *name, 
                      ApplicationManagerData *appdata)
{
  GtkWidget *dymenuitem;

  g_return_if_fail (GTK_IS_MENU (filtermenu));

  dymenuitem = gtk_menu_item_new_with_label (name);
  gtk_widget_show (dymenuitem);
  gtk_container_add (GTK_CONTAINER (filtermenu), dymenuitem);
  g_signal_connect ((gpointer) dymenuitem, "activate",
                    G_CALLBACK (on_dynamic_menu_item_activate), appdata);

}
