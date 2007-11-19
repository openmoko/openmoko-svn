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
#include "appmanager-window.h"
#include "ipkg-utils.h"

/*
 * @brief The callback function of the button "upgrade"
 */
void 
on_upgrade_clicked (GtkButton *bupgrade, gpointer data)
{
  GList *list;
  int upgrades;
  
  update_package_list (data);
  
  list = get_upgrade_list ();
  
  if ((upgrades = g_list_length (list)) > 0)
  {
    GtkWidget *dlg;
    
    dlg = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO,
                            "There are %d updates available. Would you like to install them now?",
                            upgrades);
    if (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_YES)
    {
      gtk_widget_destroy (dlg);
      upgrade_packages ();
    }
    else
    {
      gtk_widget_destroy (dlg);
    }
  }
  
  g_list_free (list);
  
  /*
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
  */
}


void
on_install_clicked (GtkWidget *button, ApplicationManagerData *data)
{
  GtkTreeSelection *sel;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *name;
  
  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (data->tvpkglist));
  
  if (!gtk_tree_selection_get_selected (sel, &model, &iter))
    return;
  
  gtk_tree_model_get (model, &iter, COL_NAME, &name, -1);
  
  install_package (data, name);
}

void
on_remove_clicked (GtkWidget *button, ApplicationManagerData *data)
{
  GtkTreeSelection *sel;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *name;
  
  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (data->tvpkglist));
  
  if (!gtk_tree_selection_get_selected (sel, &model, &iter))
    return;
  
  gtk_tree_model_get (model, &iter, COL_NAME, &name, -1);
  
  remove_package (data, name);
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
  GtkToolItem *tool_button;
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

  /* install package */
  tool_button = gtk_tool_button_new_from_stock (GTK_STOCK_ADD);
  g_signal_connect (tool_button, "clicked", G_CALLBACK (on_install_clicked), appdata);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbox), tool_button, -1);
  gtk_tool_item_set_expand (tool_button, TRUE);
  appdata->install_btn = tool_button;
  
  /* remove package */
  tool_button = gtk_tool_button_new_from_stock (GTK_STOCK_DELETE);
  g_signal_connect (tool_button, "clicked", G_CALLBACK (on_remove_clicked), appdata);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbox), tool_button, -1);
  gtk_tool_item_set_expand (tool_button, TRUE);
  appdata->remove_btn = tool_button;

  return toolbox;
}

