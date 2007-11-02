/*
 *  @file tool-box.c
 *  @brief The tool box in the main window
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
#include <string.h>

#include "config.h"

#include "tool-box.h"
#include "navigation-area.h"
#include "package-list.h"
#include "appmanager-window.h"
#include "apply-dialog.h"
#include "install-dialog.h"

/*
 * @brief The callback function of the button "upgrade"
 */
void 
on_upgrade_clicked (GtkButton *bupgrade, gpointer data)
{
  GtkWidget *dialog;

  g_debug ("Clicked the button upgrade");
  package_list_mark_all_upgradeable (MOKO_APPLICATION_MANAGER_DATA (data));
  navigation_area_rebuild_from_latest (MOKO_APPLICATION_MANAGER_DATA (data));

  g_debug ("Create a dialog");
  dialog = gtk_message_dialog_new (NULL,
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_INFO,
                                   GTK_BUTTONS_OK,
                                   _("Marked all upgradeable packages"));
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
  g_debug ("destroy a dialog");
}

/*
 * @brief The callback function of the button "Apply"
 */
void 
on_apply_clicked (GtkButton *bapply, gpointer data)
{
  GtkWidget *dialog;
  InstallDialog *installdialog;
  gint      res;
  gint      number;

  g_debug ("Clicked the button apply");

  if (package_list_check_marked_list_empty (
         MOKO_APPLICATION_MANAGER_DATA (data)))
    {
      dialog = gtk_message_dialog_new (NULL,
                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_INFO,
                                       GTK_BUTTONS_OK,
                                       _("No package that has been selected"));
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      return;
    }

  dialog = apply_dialog_new (MOKO_APPLICATION_MANAGER_DATA (data));
  res = gtk_dialog_run (GTK_DIALOG (dialog));

  /* FIXME Add code to install/remove/upgrade package */
  if (res == GTK_RESPONSE_OK)
    {
      g_debug ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      g_debug ("Check the number of the selected list");
      number = package_list_get_number_of_selected (MOKO_APPLICATION_MANAGER_DATA (data));
      installdialog = install_dialog_new (MOKO_APPLICATION_MANAGER_DATA (data), number);
      application_manager_data_set_install_dialog (MOKO_APPLICATION_MANAGER_DATA (data),
                                                   GTK_WIDGET (installdialog));
      g_debug ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      g_debug ("Begin to install/upgrade/remove packages");
      if (!g_thread_supported ())
        {
          g_thread_init (NULL);
        }

      g_thread_create (package_list_execute_change, data, TRUE, NULL);
      gtk_dialog_run (GTK_DIALOG (installdialog));
      gtk_widget_destroy (GTK_WIDGET (installdialog));
    }

  gtk_widget_destroy (dialog);
}

/*
 * @brief The callback function of the search entry
 */
void 
on_search_entry_changed (GtkEditable *entryedit, gpointer data)
{
  ApplicationManagerData *appdata;
  GtkEntry               *entry;
  const gchar            *searchstr;
  gchar                  *searchhistory;

  g_debug ("Change the contant of entry");
  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (data));

  appdata = data;
  entry = application_manager_get_search_entry (appdata);
  g_return_if_fail (GTK_IS_ENTRY (entry));
  searchstr = gtk_entry_get_text (entry);

  g_return_if_fail (searchstr != NULL);

  if (searchstr[0] == '\0')
    {
      /* FIXME Add code later */
      g_debug ("The length of search string is 0");
      navigation_area_rebuild_from_latest (appdata);
      return;
    }

  searchhistory = application_manager_data_get_search_history (appdata);
  if (searchhistory == NULL)
    {
      searchhistory = g_malloc (MAX_SEARCH_ENTRY_TEXT_LENGTH + 1);
      g_return_if_fail (searchhistory != NULL);

      searchhistory[0] = '\0';
      application_manager_data_set_search_history (appdata, searchhistory);
    }

  if (strstr ((char *)searchstr, (char *)searchhistory) != NULL)
    {
      g_debug ("Increase search");
      navigation_area_increase_search (appdata, searchstr);
    }
  else 
    {
      g_debug ("Search from the beginning");
      navigation_area_rebuild_search_result (appdata, searchstr);
    }

  strncpy (searchhistory, searchstr, MAX_SEARCH_ENTRY_TEXT_LENGTH);
}

/*
 * @brief Create a new tool box for the application manager data
 * @param appdata The application manager data
 * @return The toplevel widget of the tool box
 */
GtkWidget *
tool_box_new (ApplicationManagerData *appdata)
{
  GtkWidget   *toolbox;
  GtkToolItem *bapply;
  GtkToolItem *bupgrade;
  GtkWidget   *anImage;

  toolbox = gtk_toolbar_new ();

  anImage = gtk_image_new_from_file (PKGDATADIR "/Upgrades.png");
  bupgrade = gtk_tool_button_new (anImage, "Upgrades");
  g_signal_connect ((gpointer)bupgrade, "clicked",
                    G_CALLBACK (on_upgrade_clicked), 
                    appdata);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbox), bupgrade, -1);
  gtk_container_child_set (GTK_CONTAINER (toolbox), GTK_WIDGET (bupgrade), "expand", TRUE, NULL);

  anImage = gtk_image_new_from_file (PKGDATADIR "/Apply.png");
  bapply = gtk_tool_button_new (anImage, "Apply");
  g_signal_connect ((gpointer)bapply, "clicked",
                    G_CALLBACK (on_apply_clicked), 
                    appdata);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbox), bapply, -1);
  gtk_container_child_set (GTK_CONTAINER (toolbox), GTK_WIDGET (bapply), "expand", TRUE, NULL);
#if 0
  searchentry = moko_tool_box_get_entry (toolbox);
  application_manager_data_set_search_entry (appdata, GTK_ENTRY (searchentry));
  gtk_entry_set_max_length (GTK_ENTRY (searchentry), MAX_SEARCH_ENTRY_TEXT_LENGTH);
  g_signal_connect ((gpointer) searchentry, "changed",
                    G_CALLBACK (on_search_entry_changed),
                    appdata);
#endif
  return toolbox;
}

