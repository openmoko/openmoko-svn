/**
 *  @file tool-box.c
 *  @brief The tool box in the main window
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
#include <string.h>

#include "tool-box.h"
#include "navigation-area.h"

/**
 * @brief The callback function of the button "upgrade"
 */
void 
on_upgrade_clicked (GtkButton *bupgrade, gpointer data)
{
  g_debug ("Clicked the button upgrade");
}

/**
 * @brief The callback function of the button "Apply"
 */
void 
on_apply_clicked (GtkButton *bapply, gpointer data)
{
  g_debug ("Clicked the button apply");
}

/**
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
      //FIXME Add code later
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

/**
 * @brief Create a new tool box for the application manager data
 * @param appdata The application manager data
 * @return The toplevel widget of the tool box
 */
MokoToolBox *
tool_box_new (ApplicationManagerData *appdata)
{
  MokoToolBox *toolbox;
  MokoPixmapButton *bapply;
  MokoPixmapButton *bupgrade;
  GtkEntry    *searchentry;

  toolbox = MOKO_TOOL_BOX (moko_tool_box_new_with_search ());

  bupgrade = moko_tool_box_add_action_button (toolbox);
  gtk_button_set_label (GTK_BUTTON (bupgrade), "Upgrade");
  g_signal_connect ((gpointer)bupgrade, "clicked",
                    G_CALLBACK (on_upgrade_clicked), 
                    appdata);

  bapply = moko_tool_box_add_action_button (toolbox);
  gtk_button_set_label (GTK_BUTTON (bapply), "Apply");
  g_signal_connect ((gpointer)bapply, "clicked",
                    G_CALLBACK (on_apply_clicked), 
                    appdata);

  searchentry = moko_tool_box_get_entry (toolbox);
  application_manager_data_set_search_entry (appdata, searchentry);
  gtk_entry_set_max_length (searchentry, MAX_SEARCH_ENTRY_TEXT_LENGTH);
  g_signal_connect ((gpointer) searchentry, "changed",
                    G_CALLBACK (on_search_entry_changed),
                    appdata);

  return toolbox;
}
