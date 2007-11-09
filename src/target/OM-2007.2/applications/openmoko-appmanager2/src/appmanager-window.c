/*
 *  @file appmanager-window.c
 *  @brief The application manager in the Openmoko
 *
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 *  @author Chaowei Song (songcw@fic-sh.com.cn)
 */

#include <gtk/gtk.h>

#include "appmanager-window.h"
#include "application-menu.h"
#include "navigation-area.h"
#include "tool-box.h"
#include "detail-area.h"
#include "appmanager-data.h"
#include "errorcode.h"
#include "search-bar.h"

#include "ipkg-utils.h"

/*
 * @brief The start function.
 */
int
main (int argc, char* argv[])
{
  ApplicationManagerData *appdata;
  GtkWidget       *window;
  GtkWidget       *navigation;
  GtkWidget       *toolbox;
  GtkWidget       *detail;
  GtkWidget       *notebook;
  GtkWidget *searchbar;
  GtkWidget *vbox, *nav_vbox;
  
  GtkTreeModel *pkg_list;

  g_debug ("application manager start up");

  g_thread_init (NULL);
  gdk_threads_init ();
  gtk_init (&argc, &argv);
  g_set_application_name( _("Application Manager") );

  if (argc != 1)
    {
      /* Add init code. */
    }

  appdata = application_manager_data_new ();
  if (appdata == NULL)
    {
      g_debug ("Create main data struct error. Abort.");
      return -1;
    }

  init_pixbuf_list (appdata);

  /* create the package list store */
  pkg_list = package_store_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (window), "delete_event",
                    G_CALLBACK (gtk_main_quit), NULL);
  application_manager_data_set_main_window (appdata, GTK_WINDOW (window));

  /* main vbox */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (appdata->mwindow), vbox);

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_BOTTOM);
  gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);

  /* navigation page */
  nav_vbox = gtk_vbox_new (FALSE, 0);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), nav_vbox,
     gtk_image_new_from_stock (GTK_STOCK_INDEX, GTK_ICON_SIZE_LARGE_TOOLBAR));
  gtk_container_child_set (GTK_CONTAINER (notebook), nav_vbox, "tab-expand", TRUE, NULL);

  toolbox = tool_box_new (appdata);
  gtk_box_pack_start (GTK_BOX (nav_vbox), toolbox, FALSE, FALSE, 0);

  searchbar = search_bar_new (appdata, pkg_list);
  gtk_box_pack_start (GTK_BOX (nav_vbox), searchbar, FALSE, FALSE, 0);
  
  navigation = navigation_area_new (appdata, pkg_list);
  gtk_box_pack_start (GTK_BOX (nav_vbox), navigation, TRUE, TRUE, 0);
  

  detail = detail_area_new (appdata);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), detail,
     gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_LARGE_TOOLBAR));
  gtk_container_child_set (GTK_CONTAINER (notebook), detail, "tab-expand", TRUE, NULL);

#if 0
  /* Load the list of all package in the memory */
  ret = init_package_list (appdata);
  if (ret != OP_SUCCESS)
  {
    g_debug ("Can not initialize libipkg, result was %d, aborting.", ret);
    return -1;
  }

  ret = package_list_build_index (appdata);
  if (ret != OP_SUCCESS)
  {
    GtkWidget *dlg;
      
    dlg = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_QUESTION,
                                  GTK_BUTTONS_YES_NO,
                                  "Package list not available. "
                                  "Would you like to update it now?");
    if (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_YES)
    {
      /* update the package list */
      update_package_list (appdata);

      /* try to reload the package list */
      ret = reinit_package_list (appdata);
      ret = package_list_build_index (appdata);

      gtk_widget_destroy (dlg);

      if (ret != OP_SUCCESS)
      {
        dlg = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                "Could not update the package list");
        gtk_dialog_run (GTK_DIALOG (dlg));
        gtk_widget_destroy (dlg);
        return -1;
      }
    }
    else      
    {
      g_debug ("Can not build index for packages, aborting.");
      return -1;
    }
  }
  /* Add section list to the filter menu */
  package_list_add_section_to_filter_menu (appdata);
#endif

  search_bar_set_active_filter (MOKO_SEARCH_BAR (searchbar), FILTER_INSTALLED);

  gtk_widget_show_all (GTK_WIDGET (window));
  g_debug ("application manager enter main loop");
  gtk_main ();
  g_debug ("application finished");

  return 0;
}

