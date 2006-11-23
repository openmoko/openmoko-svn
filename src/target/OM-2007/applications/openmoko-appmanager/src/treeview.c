/**
 * @file treeview.c 
 * @brief Init the column and store in the treeview of package list.
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * @author Chaowei Song(songcw@fic-sh.com.cn)
 * @date 2006-10-12
 */

#include <gtk/gtk.h>

#include "support.h"
#include "treeview.h"
#include "widgets.h"
#include "pkglist.h"
#include "errorcode.h"

static void init_pkglist_head (void);

/**
 * @brief Init the list store of package list.
 * 
 * @return The list store.
 */
static GtkListStore *
init_package_list (void)
{
  GtkListStore *store;

  store = gtk_list_store_new (NUM_COL, GDK_TYPE_PIXBUF,
                              G_TYPE_STRING, G_TYPE_STRING,
                              G_TYPE_POINTER);

  return store;
}


/**
 * @brief Init the tree view of package list.
 */
void
init_package_view (void)
{
  GtkWidget *treeview;
  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;

  treeview = get_widget_pointer (FIC_WIDGET_TREE_VIEW_PKG_LIST);
  if (treeview == NULL)
    {
      g_error ("Init tree view fail. The system not init the tree view of package list correctly");
    }

  //! Add the status as the first column.
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("S"));
  gtk_tree_view_column_set_resizable (col, FALSE);
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (col, 20);

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "pixbuf", COL_STATUS,
                                       NULL);
  gtk_tree_view_column_set_resizable (col, FALSE);

  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col);

  //! Add the name as the second column.
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("Name"));
  gtk_tree_view_column_set_resizable (col, FALSE);
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (col, 240);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "text", COL_NAME,
                                       NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col);

  //! Add the size as the third column.
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("Size"));

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "text", COL_SIZE,
                                       NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col);

  model = GTK_TREE_MODEL (init_package_list ());

  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), model);

  g_object_unref (model);

  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
                               GTK_SELECTION_SINGLE);

  init_pkglist_head ();
}

/**
 * @brief Init the navigation area with the theme
 */
static void 
init_pkglist_head (void)
{
  GtkWidget  *pkgfix;
  GtkWidget  *headfix;
  GtkWidget  *label1;
  GtkWidget  *label2;
  GtkWidget  *sep1;

  pkgfix = get_widget_pointer (FIC_WIDGET_FIX_PKG_LIST);
  if (pkgfix == NULL)
    {
      ERROR ("Can not find the fix widget in the navigation area.");
      return;
    }

  headfix = lookup_widget (pkgfix, "fixedpkghead");
  if (headfix == NULL)
    {
      ERROR ("Can not find the fix widget in the navigation head area.");
      return;
    }
  gtk_fixed_set_has_window (GTK_FIXED (headfix), TRUE);
  gtk_widget_set_name (headfix, "bg_navigation_list_head");
  //gtk_widget_show (headfix);
  //gtk_fixed_put (GTK_FIXED (pkgfix), headfix, 17, 14);
  //gtk_widget_set_size_request (headfix, 403, 33);

  label1 = gtk_label_new (_("Name"));
  gtk_widget_set_name (label1, "label_navigation_list_head");
  gtk_widget_show (label1);
  gtk_fixed_put (GTK_FIXED (headfix), label1, 33, 0);
  gtk_widget_set_size_request (label1, 64, 33);

  label2 = gtk_label_new (_("Size"));
  gtk_widget_set_name (label2, "label_navigation_list_head");
  gtk_widget_show (label2);
  gtk_fixed_put (GTK_FIXED (headfix), label2, 254, 0);
  gtk_widget_set_size_request (label2, 96, 33);

  sep1 = gtk_vseparator_new ();
  gtk_widget_set_name (sep1, "navigation_list_head_separate");
  gtk_widget_show (sep1);
  gtk_fixed_put (GTK_FIXED (headfix), sep1, 220, 0);
  gtk_widget_set_size_request (sep1, 10, 33);
}
