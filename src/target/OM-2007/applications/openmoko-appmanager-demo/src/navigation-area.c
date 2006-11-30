/**
 *  @file navigation-area.c
 *  @brief The navigation area in the main window
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
#include <libmokoui/moko-tree-view.h>

#include "appmanager-window.h"
#include "navigation-area.h"

static GtkListStore *
create_package_list_store (void)
{
  GtkListStore *store;

  store = gtk_list_store_new (NUM_COL, GDK_TYPE_PIXBUF,
                              G_TYPE_STRING, G_TYPE_STRING,
                              G_TYPE_POINTER);

  return store;
}


/**
 * @brief Create all widgets in the navigation area for the application manager data.
 *
 * @param appdata The application manager data
 * @return The toplevel widget in the navigation area
 */
GtkWidget *
navigation_area_new (ApplicationManagerData *appdata)
{
  GtkWidget          *scrollwindow;
  GtkWidget          *treeview;

  GtkTreeModel       *model;
  GtkTreeViewColumn  *col;
  GtkCellRenderer    *renderer;

  treeview = moko_tree_view_new ();
  gtk_widget_show (treeview);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), FALSE);

  ///<! Add the status as the first column.
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("S"));
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  //gtk_tree_view_column_set_fixed_width (col, 20);

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "pixbuf", COL_STATUS,
                                       NULL);

  moko_tree_view_append_column (MOKO_TREE_VIEW (treeview), col);

  ///<! Add the name as the second column.
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("Name"));
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  //gtk_tree_view_column_set_fixed_width (col, 240);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "text", COL_NAME,
                                       NULL);

  moko_tree_view_append_column (MOKO_TREE_VIEW (treeview), col);

  ///<! Add the size as the third column.
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("Size"));

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "text", COL_SIZE,
                                       NULL);

  moko_tree_view_append_column (MOKO_TREE_VIEW (treeview), col);

  model = GTK_TREE_MODEL (create_package_list_store ());
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), model);
  g_object_unref (model);

  scrollwindow = GTK_WIDGET (moko_tree_view_put_into_scrolled_window (MOKO_TREE_VIEW (treeview)));
  application_manager_data_set_tvpkglist (appdata, treeview);

  return scrollwindow;
}
