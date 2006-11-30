/**
 *  @file application-menu.c
 *  @brief The application menu item
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

#include "application-menu.h"
#include "appmanager-window.h"

/**
 * @brief The Callback function of the show status menu
 */
void 
on_showstatus_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  g_debug ("Call on_showstatus_activate");
}

/**
 * @brief The Callback function of the show source menu
 */
void 
on_showsource_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  g_debug ("Call on_showsource_activate");
}

/**
 * @brief The Callback function of the install single application menu
 */
void 
on_install_single_application_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  g_debug ("Call on_install_single_application_activate");
}

/**
 * @brief The Callback function of the show help menu
 */
void 
on_showhelp_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  g_debug ("Call on_showhelp_activate");
}

/**
 * @brief The Callback function of the quit menu
 */
void 
on_quit_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  g_debug ("Call on_quit_activate");
  gtk_main_quit ();
}

/**
 * @brief Create a new application menu for the application manager
 * @param appdata The application manager data
 * @return The GtkMenu widget. If there is error, 
 * it will return NULL.
 */
GtkMenu *
application_menu_new (ApplicationManagerData *appdata)
{
  GtkMenu *menu;
  GtkWidget   *menuitem1;
  GtkWidget   *menuitem2;
  GtkWidget   *menuitem3;
  GtkWidget   *menuitem4;
  GtkWidget   *menuitem5;

  g_debug ("Init the application menu");
  menu = GTK_MENU (gtk_menu_new ());

  menuitem1 = gtk_menu_item_new_with_mnemonic (_("Shows tatus"));
  gtk_widget_show (menuitem1);
  gtk_container_add (GTK_CONTAINER (menu), menuitem1);
  g_signal_connect ((gpointer) menuitem1, "activate",
                    G_CALLBACK (on_showstatus_activate), appdata);

  menuitem2 = gtk_menu_item_new_with_mnemonic (_("Show source"));
  gtk_widget_show (menuitem2);
  gtk_container_add (GTK_CONTAINER (menu), menuitem2);
  g_signal_connect ((gpointer) menuitem2, "activate",
                    G_CALLBACK (on_showsource_activate), appdata);

  menuitem3 = gtk_menu_item_new_with_mnemonic (_("Install single application"));
  gtk_widget_show (menuitem3);
  gtk_container_add (GTK_CONTAINER (menu), menuitem3);
  g_signal_connect ((gpointer) menuitem3, "activate",
                    G_CALLBACK (on_install_single_application_activate), appdata);

  menuitem4 = gtk_menu_item_new_with_mnemonic (_("Show help"));
  gtk_widget_show (menuitem4);
  gtk_container_add (GTK_CONTAINER (menu), menuitem4);
  g_signal_connect ((gpointer) menuitem4, "activate",
                    G_CALLBACK (on_showhelp_activate), appdata);

  menuitem5 = gtk_menu_item_new_with_mnemonic (_("Quit"));
  gtk_widget_show (menuitem5);
  gtk_container_add (GTK_CONTAINER (menu), menuitem5);
  g_signal_connect ((gpointer) menuitem5, "activate",
                    G_CALLBACK (on_quit_activate), appdata);

  return menu;
}
