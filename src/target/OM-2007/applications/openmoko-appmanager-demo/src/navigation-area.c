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

#include "navigation-area.h"

/**
 * @brief Create all widgets in the navigation area for the main window.
 *
 * @param window The main window
 * @return The toplevel widget in the navigation area
 */
GtkWidget *
navigation_area_new_for_window (MokoPanedWindow *window)
{
  GtkWidget *scrollwindow;
  GtkWidget *treeview;

  treeview = moko_tree_view_new ();
  gtk_widget_show (treeview);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), FALSE);

  scrollwindow = GTK_WIDGET (moko_tree_view_put_into_scrolled_window (MOKO_TREE_VIEW (treeview)));

  return scrollwindow;
}
