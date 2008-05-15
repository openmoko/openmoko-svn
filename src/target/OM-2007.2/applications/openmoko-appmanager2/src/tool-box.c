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
#include "am-progress-dialog.h"

/*
 * @brief The callback function of the button "upgrade"
 */
void 
on_upgrade_clicked (GtkButton *bupgrade, gpointer data)
{
 /* 
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
  
  */
}

void
progress_update_cb (opkg_t *opkg, const opkg_progress_data_t *pdata, void *am_progress_dialog)
{
  AmProgressDialog *dlg = AM_PROGRESS_DIALOG (am_progress_dialog);
  gchar *text, *name;

  if (!dlg)
    return;

  if (pdata->package)
     name = pdata->package->name;
  else
    name = "";

  gdk_threads_enter();
  am_progress_dialog_set_progress (dlg, pdata->percentage / 100.0);

  switch (pdata->action)
  {
    case OPKG_INSTALL:
      text = g_strdup_printf ("Installing %s...", name);
      break;
    case OPKG_REMOVE:
      text = g_strdup_printf ("Removing %s...", name);
      break;
    case OPKG_DOWNLOAD:
      text = g_strdup_printf ("Downloading %s...", name);
      break;
    default:
      text = g_strdup ("Please wait...");
  }

  am_progress_dialog_set_label_text (dlg, text);
  g_free (text);
  gdk_threads_leave();

}
void start_install (ApplicationManagerData *data)
{
  GtkTreeSelection *sel;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *name;
  GtkWidget *dialog;
  gint ret;
  
  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (data->tvpkglist));
  
  if (!gtk_tree_selection_get_selected (sel, &model, &iter))
    return;
  
  gtk_tree_model_get (model, &iter, COL_NAME, &name, -1);

  
  ret = opkg_install_package (data->opkg, name, progress_update_cb, data->installdialog);

  gdk_threads_enter();
  if (ret == 0)
    dialog = gtk_message_dialog_new (NULL,0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s was installed", name);
  else
    dialog = gtk_message_dialog_new (NULL,0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s could not be installed", name);

  gtk_widget_destroy (data->installdialog);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
  gdk_threads_leave();
}

void
on_install_clicked (GtkWidget *button, ApplicationManagerData *data)
{
  data->installdialog = am_progress_dialog_new ();
  gtk_widget_show_all (data->installdialog);

  g_thread_create ((GThreadFunc) start_install, data, FALSE, NULL);
}

void
on_remove_clicked (GtkWidget *button, ApplicationManagerData *data)
{
  GtkTreeSelection *sel;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *name;
  GtkWidget *dialog;
  
  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (data->tvpkglist));
  
  if (!gtk_tree_selection_get_selected (sel, &model, &iter))
    return;
  
  gtk_tree_model_get (model, &iter, COL_NAME, &name, -1);
  
  if (opkg_remove_package (data->opkg, name, NULL, NULL) == 0)
    dialog = gtk_message_dialog_new (NULL,0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s was removed", name);
  else
    dialog = gtk_message_dialog_new (NULL,0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s could not be removed", name);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

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

