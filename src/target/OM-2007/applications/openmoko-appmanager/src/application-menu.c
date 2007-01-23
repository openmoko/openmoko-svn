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
#include "package-list.h"
#include "ipkgapi.h"
#include "errorcode.h"

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
  ApplicationManagerData  *appdata;
  GtkWidget  *confirmdialog;
  gint       ret;
  GtkWidget      *filechooser;
  gint           res;
  GtkFileFilter  *filter;
  gchar          *filename;
  char           *newname = NULL;

  g_debug ("Call on_install_single_application_activate");
  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (user_data));
  appdata = MOKO_APPLICATION_MANAGER_DATA (user_data);

  if (!package_list_check_marked_list_empty (appdata))
    {
      confirmdialog = gtk_message_dialog_new (NULL, 
                                              GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_MESSAGE_QUESTION,
                                              GTK_BUTTONS_YES_NO,
                                              _("It will unselect all you selected packages, continue?"));
      ret = gtk_dialog_run (GTK_DIALOG (confirmdialog));
      gtk_widget_destroy (confirmdialog);
      if (ret != GTK_RESPONSE_YES)
        {
          g_debug ("User cancel");
          return;
        }
    }

  filechooser = gtk_file_chooser_dialog_new(_("Add application"),
                                            NULL,
                                            GTK_FILE_CHOOSER_ACTION_OPEN,
                                            GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                            NULL);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(filechooser), FALSE);
  gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(filechooser), FALSE);
  gtk_window_set_default_size (GTK_WINDOW (filechooser), 400, 300);

  filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, _("Ipk files(*.ipk)"));
  gtk_file_filter_add_pattern(filter, "*.ipk");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(filechooser), filter);

  filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, _("All files(*.*)"));
  gtk_file_filter_add_pattern(filter, "*");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(filechooser), filter);

  res = gtk_dialog_run(GTK_DIALOG(filechooser));

  if( (res == GTK_RESPONSE_ACCEPT) || (res == GTK_RESPONSE_OK) ) {
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooser));
    g_debug ("file name of user selected is :%s\n", filename);

    res = ipkg_install_cmd (filename, "root", &newname);
    if (res == 0)
      {
        package_list_free_all_dynamic (appdata);
        ret = reinit_package_list (appdata);
        if (ret != OP_SUCCESS)
          {
            g_debug ("Can not initial the libipkg, the result is%d", ret);
            g_free (filename);
            gtk_widget_destroy(filechooser);
            return;
          }
        ret = package_list_build_index (appdata);
        if (ret != OP_SUCCESS)
          {
            g_debug ("Can not build index for packages");
            g_free (filename);
            gtk_widget_destroy(filechooser);
            return;
          }
        if (newname != NULL)
          {
            free (newname);
            newname = NULL;
          }
      }
    else
      {
        g_debug ("Install error, the error message is:%s", get_error_msg());
        if (newname != NULL)
          {
            free (newname);
            newname = NULL;
          }
      }

    g_free (filename);
  }

  gtk_widget_destroy(filechooser);
  return;
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
  GtkMenu     *appmenu;
  GtkWidget   *showstatus;
  GtkWidget   *showsource;
  GtkWidget   *installsingleapp;
  GtkWidget   *showhelp;
  GtkWidget   *quit;

  g_debug ("Init the application appmenu");
  g_return_val_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (appdata), NULL);

  appmenu = GTK_MENU (gtk_menu_new ());

  showstatus = gtk_menu_item_new_with_mnemonic (_("Shows tatus"));
  gtk_widget_show (showstatus);
  gtk_container_add (GTK_CONTAINER (appmenu), showstatus);
  g_signal_connect ((gpointer) showstatus, "activate",
                    G_CALLBACK (on_showstatus_activate), appdata);

  showsource = gtk_menu_item_new_with_mnemonic (_("Show source"));
  gtk_widget_show (showsource);
  gtk_container_add (GTK_CONTAINER (appmenu), showsource);
  g_signal_connect ((gpointer) showsource, "activate",
                    G_CALLBACK (on_showsource_activate), appdata);

  installsingleapp = gtk_menu_item_new_with_mnemonic (_("Install single application"));
  gtk_widget_show (installsingleapp);
  gtk_container_add (GTK_CONTAINER (appmenu), installsingleapp);
  g_signal_connect ((gpointer) installsingleapp, "activate",
                    G_CALLBACK (on_install_single_application_activate), appdata);

  showhelp = gtk_menu_item_new_with_mnemonic (_("Show help"));
  gtk_widget_show (showhelp);
  gtk_container_add (GTK_CONTAINER (appmenu), showhelp);
  g_signal_connect ((gpointer) showhelp, "activate",
                    G_CALLBACK (on_showhelp_activate), appdata);

  quit = gtk_menu_item_new_with_mnemonic (_("Quit"));
  gtk_widget_show (quit);
  gtk_container_add (GTK_CONTAINER (appmenu), quit);
  g_signal_connect ((gpointer) quit, "activate",
                    G_CALLBACK (on_quit_activate), appdata);

  return appmenu;
}

