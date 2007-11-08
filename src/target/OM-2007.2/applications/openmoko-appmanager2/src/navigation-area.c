/*
 *  @file navigation-area.c
 *  @brief The navigation area in the main window
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
 *  Authors:
 *    Chaowei Song <songcw@fic-sh.com.cn>
 *    OpenedHand Ltd. <info@openedhand.com>
 */
#include <string.h>
#include <moko-finger-scroll.h>
#include <moko-search-bar.h>

#include "appmanager-window.h"
#include "navigation-area.h"
#include "errorcode.h"
#include "detail-area.h"
#include "package-store.h"
#include "select-menu.h"

#include "ipkgapi.h"

/*
 * @brief The callback function of the signal "cursor-changed"
 */
void 
on_selection_changed (GtkTreeSelection *selection, 
                            ApplicationManagerData *data)
{
  GtkTreeModel     *model;
  GtkTreeIter      iter;
  IPK_PACKAGE      *pkg;

  g_debug ("Call the on_treeview_cursor_changed");

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, COL_POINTER, &pkg, -1);
    detail_area_update_info (data, pkg);
    
    if (pkg->state_status == SS_INSTALLED)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (data->install_btn), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (data->remove_btn), TRUE);
    }
    else
    {
      gtk_widget_set_sensitive (GTK_WIDGET (data->install_btn), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (data->remove_btn), FALSE);
    }
  }
  else
  {
    gtk_widget_set_sensitive (GTK_WIDGET (data->install_btn), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (data->remove_btn), FALSE);
  }
}

/*
 * model_filter_func
 */

gboolean
model_filter_func (GtkTreeModel *model, GtkTreeIter *iter, ApplicationManagerData *appdata)
{
  gboolean result = TRUE;
  
  if (!appdata->searchbar)
    return FALSE;
    
  
  if (moko_search_bar_search_visible (MOKO_SEARCH_BAR (appdata->searchbar)))
  {
    gchar *haystack;
    const gchar *needle;
    GtkEntry *entry;
    
    gtk_tree_model_get (model, iter, COL_NAME, &haystack, -1);
    
    entry = moko_search_bar_get_entry (MOKO_SEARCH_BAR (appdata->searchbar));
    needle = gtk_entry_get_text (entry);
    
    result = (strstr (haystack, needle) != NULL);
    
    g_free (haystack);
    return result;
  }
  else
  {
    GtkComboBox *combo;
    GtkTreeIter cb_iter;
    GtkTreeModel *cb_model;
    gchar *needle;
    IPK_PACKAGE *pkg;
    
    combo = moko_search_bar_get_combo_box (MOKO_SEARCH_BAR (appdata->searchbar));
    if (!gtk_combo_box_get_active_iter (combo, &cb_iter))
      return FALSE;
    cb_model = gtk_combo_box_get_model (combo);
    if (!cb_model) return FALSE;
    gtk_tree_model_get (cb_model, &cb_iter, 0, &needle, -1);

    gtk_tree_model_get (model, iter, COL_POINTER, &pkg, -1);

    if (pkg && pkg->section && needle && !strcmp (pkg->section, needle))
      result = TRUE;
    else
      result = FALSE;

    g_free (needle);
    return result;
  }
}

/*
 * @brief Create all widgets in the navigation area for the application manager data.
 *
 * @param appdata The application manager data
 * @return The toplevel widget in the navigation area
 */
GtkWidget *
navigation_area_new (ApplicationManagerData *appdata, GtkTreeModel *pkg_list)
{
  GtkWidget          *scrollwindow;
  GtkWidget          *treeview;

  GtkTreeModel       *filter;
  GtkTreeViewColumn  *col;
  GtkCellRenderer    *renderer;

  treeview = gtk_tree_view_new ();
  gtk_widget_show (treeview);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), FALSE);

  /* Add the status as the first column. */
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("S"));

  /*
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "pixbuf", COL_STATUS,
                                       NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col);
  */

  /* Add the name as the second column. */
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("Name"));

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_attributes (col, renderer, "text", COL_NAME, NULL);
  g_object_set (G_OBJECT (renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col);

  filter = gtk_tree_model_filter_new (pkg_list, NULL);
  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (filter),
                                          (GtkTreeModelFilterVisibleFunc) model_filter_func, appdata, NULL);

  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), filter);
  g_object_unref (filter);

  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
                               GTK_SELECTION_BROWSE);

  scrollwindow = moko_finger_scroll_new ();
  gtk_container_add (GTK_CONTAINER (scrollwindow), treeview);
  application_manager_data_set_tvpkglist (appdata, treeview);

  /* Connect signal to the treeview */
  g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
                    "changed", G_CALLBACK (on_selection_changed), appdata);

  return scrollwindow;
}

/*
 * @brief Get the name that the roll was selected from the treeview
 * @param treeview The treeview
 * @return The package name. 
 *   -If there is not any row selected, it will return NULL.
 *   -If the return is not NULL, it must be freed by g_free.
 */
gchar *
treeview_get_selected_name (GtkWidget *treeview)
{
  GtkTreeModel     *model;
  GtkTreeIter      iter;
  GtkTreeSelection *selection;
  gchar            *name = NULL;

  g_return_val_if_fail (GTK_IS_TREE_VIEW (treeview), NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, COL_NAME, &name, -1);
      return name;
    }

  return NULL;
}
