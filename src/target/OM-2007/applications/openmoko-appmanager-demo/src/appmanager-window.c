/**
 *  @file appmanager-window.c
 *  @brief The application manager in the Openmoko
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

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-paned-window.h>
#include <libmokoui/moko-tool-box.h>

#include <gtk/gtk.h>

#include "appmanager-window.h"
#include "application-menu.h"
#include "filter-menu.h"
#include "navigation-area.h"
#include "tool-box.h"
#include "detail-area.h"
#include "appmanager-data.h"

/**
 * @brief The start function.
 */
int 
main (int argc, char* argv[])
{
  ApplicationManagerData *appdata;
  MokoApplication *app;
  MokoPanedWindow *window;
  GtkMenu         *appmenu;
  GtkMenu         *filtermenu;
  GtkWidget       *navigation;
  MokoToolBox     *toolbox;
  GtkWidget       *detail;

  g_debug ("appplication manager start up");

  gtk_init (&argc, &argv);

  if (argc != 1)
    {
      //Add init code.
    }

  appdata = application_manager_data_new ();
  if (appdata == NULL)
    {
      g_debug ("Create main data struct error. Abort.");
      return -1;
    }

  app = MOKO_APPLICATION (moko_application_get_instance ());
  g_set_application_name (_("Application manager"));

  window = MOKO_PANED_WINDOW (moko_paned_window_new ());
  g_signal_connect (G_OBJECT (window), "delete_event", 
                    G_CALLBACK (gtk_main_quit), NULL);
  application_manager_data_set_main_window (appdata, window);

  appmenu = application_menu_new (appdata);
  moko_paned_window_set_application_menu (window, appmenu);

  filtermenu = filter_menu_new (appdata);
  moko_paned_window_set_filter_menu (window, filtermenu);

  navigation = navigation_area_new (appdata);
  moko_paned_window_set_upper_pane (window, navigation);

  toolbox = tool_box_new (appdata);
  moko_paned_window_add_toolbox (window, toolbox);

  detail = detail_area_new (appdata);
  moko_paned_window_set_lower_pane (window, detail);

  gtk_widget_show_all (GTK_WIDGET (window));
  g_debug ("application manager enter main loop");
  gtk_main ();

  return 0;
}
