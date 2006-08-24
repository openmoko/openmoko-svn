/**
 * @file detareamgr.c - Manager the details area view
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
 * @date 2006-8-16
 **/

#include <gtk/gtk.h>
#include <string.h>

#include "errorcode.h"
#include "detareamgr.h"
#include "winresize.h"
#include "pkgmgr.h"
#include "ipkgapi.h"
#include "support.h"

/**
 * @brief For dependencies list
 */
enum {
  DEP_COL_PACKAGE = 0,
  DEP_COL_NUMS
};

/**
 * @brief For details list
 */
enum {
  DET_COL_ITEM = 0,
  DET_COL_NOTES,
  DET_COL_NUMS
};


/**
 * @brief Init the dependency view in details area, add a empty store to it
 *
 * This function should be called once at the initial process.
 *
 * @return Error code
 */
/*
static gint
init_depend_info_view (GtkWidget *window)
{
  GtkWidget   *depview;

  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeStore        *treestore;

  depview = lookup_widget (window, STRING_TEXT_VIEW_DEPEND);
  if (depview == NULL)
    return OP_WINDOW_SIZE_WIDGET_NOT_FIND;

  col = gtk_tree_view_column_new ();

  gtk_tree_view_column_set_title (col, _("Package"));
  gtk_tree_view_append_column (GTK_TREE_VIEW (depview), col);
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", DEP_COL_PACKAGE);

  treestore = gtk_tree_store_new (DEP_COL_NUMS,
                                  G_TYPE_STRING);

  gtk_tree_view_set_model (GTK_TREE_VIEW(depview), GTK_TREE_MODEL (treestore));
  g_object_unref (GTK_TREE_MODEL (treestore));

  return OP_SUCCESS;

}
*/

static gint
init_detail_info_view (GtkWidget *window)
{
  GtkWidget   *detview;

  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkTreeStore        *treestore;

  detview = lookup_widget (window, STRING_TREE_VIEW_DETAILS);
  if (detview == NULL)
    return OP_WINDOW_SIZE_WIDGET_NOT_FIND;

  col = gtk_tree_view_column_new ();

  gtk_tree_view_column_set_title (col, _("Item"));
  gtk_tree_view_append_column (GTK_TREE_VIEW (detview), col);
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", DET_COL_ITEM);

  col = gtk_tree_view_column_new ();

  gtk_tree_view_column_set_title (col, _("Notes"));
  gtk_tree_view_append_column (GTK_TREE_VIEW (detview), col);
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", DET_COL_NOTES);

  treestore = gtk_tree_store_new (DET_COL_NUMS,
                                  G_TYPE_STRING,
                                  G_TYPE_STRING);

  gtk_tree_view_set_model (GTK_TREE_VIEW(detview), GTK_TREE_MODEL (treestore));
  g_object_unref (GTK_TREE_MODEL (treestore));

  return OP_SUCCESS;
}

/**
 * @brief Init all widget in details area.
 *
 * This function should be call once at init.
 *
 * @param window Any widget in the main window
 * @return Error code
 * @retval OP_SUCCESS Operation success
 */
gint
init_details_area (GtkWidget *window)
{
  //init_depend_info_view (window);
  init_detail_info_view (window);

  update_details_area (window);

  return OP_SUCCESS;
}

static void
update_detail_info_view (GtkWidget *window, PACKAGE_DETAIL_INFO *detail)
{
  GtkWidget           *detview;

  GtkTreeModel        *model;
  GtkTreeStore        *store;
  GtkTreeIter         iter;

  detview = lookup_widget (window, STRING_TREE_VIEW_DETAILS);

  if (detview == NULL)
    return;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (detview));

  if (model == NULL)
    return;

  g_object_ref (model);

  gtk_tree_view_set_model (GTK_TREE_VIEW (detview), NULL);

  store = GTK_TREE_STORE (model);
  gtk_tree_store_clear (store);

  /** Insert headers */
  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      DET_COL_ITEM, _("Item"),
                      DET_COL_NOTES, _("Notes"),
                      -1);
 
  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      DET_COL_ITEM, _("File Name"),
                      DET_COL_NOTES, (detail->filename == NULL) ? "" : detail->filename,
                      -1);

  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      DET_COL_ITEM, _("Version"),
                      DET_COL_NOTES, (detail->version == NULL) ? "" : detail->version,
                      -1);

  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      DET_COL_ITEM, _("Package Size"),
                      DET_COL_NOTES, (detail->size == NULL) ? "" : detail->size,
                      -1);

  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      DET_COL_ITEM, _("Installed Size"),
                      DET_COL_NOTES, (detail->installed_size == NULL) ? "" : detail->installed_size,
                      -1);

  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      DET_COL_ITEM, _("More Infomation"),
                      DET_COL_NOTES, (detail->maintainer == NULL) ? "" : detail->maintainer,
                      -1);

  gtk_tree_view_set_model (GTK_TREE_VIEW (detview), model);
  g_object_unref (model);

}

static void
update_depend_info_view (GtkWidget *window, PACKAGE_DETAIL_INFO *detail)
{
  GtkWidget           *depview;
  GtkTextBuffer *buffer;

  depview = lookup_widget (window, STRING_TEXT_VIEW_DEPEND);

  if (depview != NULL)
    {
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (depview));
      gtk_text_buffer_set_text (buffer, detail->depends, -1);
    }

}

static void
update_summary_info_view (GtkWidget *window, PACKAGE_DETAIL_INFO *detail)
{
  GtkWidget  *tvsummary;
  GtkTextBuffer *buffer;

  tvsummary = lookup_widget (window, STRING_TEXT_VIEW_SUMMARY);

  if (tvsummary != NULL)
    {
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tvsummary));
      gtk_text_buffer_set_text (buffer, detail->description, -1);
    }

}

/**
 * @brief Update the details area view
 *
 * @param window A widget pointer used to lookup the other widget.
 * @return Error code
 * @retval OP_SUCCESS Operation success
 */
gint
update_details_area (GtkWidget *window)
{
  gchar               *pkgname;
  PACKAGE_DETAIL_INFO *detail;

  pkgname = get_first_selected_package_name (window);

  if (pkgname != NULL)
    {
       detail = ipkg_get_pkg_detail_info (pkgname, PKG_AVAILABLE);
       if (detail != NULL)
         {
           update_summary_info_view (window, detail);
           update_detail_info_view (window, detail);
           update_depend_info_view (window, detail);

           free_pkg_detail_info (detail);
           detail = NULL;
         }

       g_free(pkgname);
       pkgname = NULL;
    }

  return OP_SUCCESS;
}
