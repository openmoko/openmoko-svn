/*
 *  @file apply-dialog.c
 *  @brief It is an infomation dialog that will display the all package 
 *  that will be changed.
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
#include "apply-dialog.h"
#include "appmanager-window.h"
#include "package-list.h"

/*
 * @brief The id of package list that display in apply dialog.
 */
enum {
  MARK_COL_NAME = 0,      /* Package name */
  MARK_NUM_COL            /* Column number */
};

/*
 * @brief Create a new apply dialog
 * @param The application manager data
 * @return The apply dialog
 */
GtkWidget *
apply_dialog_new (ApplicationManagerData *appdata)
{
  GtkWidget *applydialog;
  GtkWidget *dialogvbox;
  GtkWidget *applymsg;
  GtkWidget *listwindow;
  GtkWidget *applylist;
  GtkWidget *actionarea;
  GtkWidget *calcelbutton;
  GtkWidget *okbutton;

  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeStore    *store;

  /* Create a new dialog and add a GtkTreeview and two button to the dialog */
  applydialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (applydialog), _("summary"));
  gtk_window_set_position (GTK_WINDOW (applydialog), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_window_set_default_size (GTK_WINDOW (applydialog), 200, 300);
  gtk_window_set_type_hint (GTK_WINDOW (applydialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialogvbox = GTK_DIALOG (applydialog)->vbox;
  gtk_widget_show (dialogvbox);

  applymsg = gtk_label_new (_("Apply the following changes"));
  gtk_widget_show (applymsg);
  gtk_box_pack_start (GTK_BOX (dialogvbox), applymsg, FALSE, FALSE, 0);

  applylist = gtk_tree_view_new ();
  gtk_widget_show (applylist);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (applylist), FALSE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (applylist), FALSE);

  listwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (listwindow),applylist);
  gtk_widget_show (listwindow);
  gtk_box_pack_start (GTK_BOX (dialogvbox), listwindow, TRUE, TRUE, 0);

  actionarea = GTK_DIALOG (applydialog)->action_area;
  gtk_widget_show (actionarea);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (actionarea), GTK_BUTTONBOX_END);

  calcelbutton = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (calcelbutton);
  gtk_dialog_add_action_widget (GTK_DIALOG (applydialog), calcelbutton, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (calcelbutton, GTK_CAN_DEFAULT);

  okbutton = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton);
  gtk_dialog_add_action_widget (GTK_DIALOG (applydialog), okbutton, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton, GTK_CAN_DEFAULT);

  /* Add column to the treeview */
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("Package Name"));

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "text", MARK_COL_NAME,
                                       NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (applylist), col);

  store = gtk_tree_store_new (MARK_NUM_COL, G_TYPE_STRING);

  package_list_fill_store_with_selected_list (store, 
          application_manager_data_get_selectedlist (appdata),
          MARK_COL_NAME);

  gtk_tree_view_set_model (GTK_TREE_VIEW (applylist), GTK_TREE_MODEL (store));
  g_object_unref (GTK_TREE_MODEL (store));

  return applydialog;
}

