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
 * @brief Create a new filter menu for a window
 * 
 * At this function, it only can 
 * @param window The main window that the filter menu will add to.
 * @return The filter menu.
 */
GtkMenu *
filter_menu_new_for_window (MokoPanedWindow *window)
{
  GtkMenu   *menu;
  GtkWidget *menuitem1;
  GtkWidget *menuitem2;
  GtkWidget *menuitem3;
  GtkWidget *menuitem4;

  g_debug ("Init the filter menu");

  menu = GTK_MENU (gtk_menu_new ());

  menuitem1 = gtk_menu_item_new_with_label (_("Search Results"));
  gtk_widget_show (menuitem1);
  gtk_container_add (GTK_CONTAINER (menu), menuitem1);
  g_signal_connect ((gpointer) menuitem1, "activate",
                    G_CALLBACK (on_search_result_activate), window);

  menuitem2 = gtk_menu_item_new_with_label (_("Installed"));
  gtk_widget_show (menuitem2);
  gtk_container_add (GTK_CONTAINER (menu), menuitem2);
  g_signal_connect ((gpointer) menuitem2, "activate",
                    G_CALLBACK (on_installed_activate), window);

  menuitem3 = gtk_menu_item_new_with_label (_("Upgradeable"));
  gtk_widget_show (menuitem3);
  gtk_container_add (GTK_CONTAINER (menu), menuitem3);
  g_signal_connect ((gpointer) menuitem3, "activate",
                    G_CALLBACK (on_upgradeable_activate), window);

  menuitem4 = gtk_menu_item_new_with_label (_("Selected"));
  gtk_widget_show (menuitem4);
  gtk_container_add (GTK_CONTAINER (menu), menuitem4);
  g_signal_connect ((gpointer) menuitem4, "activate",
                    G_CALLBACK (on_selected_activate), window);

  return menu;
}

