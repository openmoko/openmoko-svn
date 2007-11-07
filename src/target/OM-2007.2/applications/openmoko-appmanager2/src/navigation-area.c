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
#include "package-list.h"
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

static GtkListStore *
create_package_list_store (void)
{
  GtkListStore *store;

  store = gtk_list_store_new (NUM_COL, GDK_TYPE_PIXBUF,
                              G_TYPE_STRING, G_TYPE_STRING,
                              G_TYPE_POINTER);

  return store;
}


/*
 * model_filter_func
 */

gboolean
model_filter_func (GtkTreeModel *model, GtkTreeIter *iter, ApplicationManagerData *appdata)
{
  gboolean result = TRUE;
  
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
  return result;
}

/*
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

  GtkTreeModel       *model, *filter;
  GtkTreeViewColumn  *col;
  GtkCellRenderer    *renderer;

  treeview = gtk_tree_view_new ();
  gtk_widget_show (treeview);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview), FALSE);

  /* Add the status as the first column. */
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("S"));

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "pixbuf", COL_STATUS,
                                       NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col);

  /* Add the name as the second column. */
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("Name"));

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_attributes (col, renderer, "text", COL_NAME, NULL);
  g_object_set (G_OBJECT (renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), col);

  model = GTK_TREE_MODEL (create_package_list_store ());
  
  filter = gtk_tree_model_filter_new (model, NULL);
  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (filter),
                                          (GtkTreeModelFilterVisibleFunc) model_filter_func, appdata, NULL);
  g_object_unref (model);
  
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

/*
 * @brief Refresh the navigation area with the package list
 * @param appdata The application manager data
 * @param pkglist The pachage list
 */
void 
navigation_area_refresh_with_package_list (ApplicationManagerData *appdata, 
                                           gpointer pkglist)
{
  GtkWidget     *treeview;
  GtkTreeModel  *model, *filter;
  GtkListStore  *store;
  GtkTreeIter    iter;

  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (appdata));
  g_return_if_fail (pkglist != NULL);

  treeview = application_manager_get_tvpkglist (appdata);
  g_return_if_fail (GTK_IS_TREE_VIEW (treeview));

  filter = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
  model = gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER (filter));
  
  g_return_if_fail (GTK_IS_TREE_MODEL (model));
  store = GTK_LIST_STORE (model);

  gtk_list_store_clear (store);

  translate_package_list_to_store (appdata, store, pkglist);
  /* Save current list to the application manager data */
  application_manager_data_set_current_list (appdata, pkglist);
  
  /* ensure one item is selected */
  if (gtk_tree_model_get_iter_first (filter, &iter))
    gtk_tree_selection_select_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
                                    &iter);
}

/*
 * @brief Rebuild the navigation list from the latest package list
 * @param appdata The application manager data
 */
void 
navigation_area_rebuild_from_latest (ApplicationManagerData *appdata)
{
  gpointer       pkglist;

  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (appdata));

  pkglist = application_manager_data_get_currentlist (appdata);
  g_return_if_fail (pkglist != NULL);

  navigation_area_refresh_with_package_list (appdata, pkglist);
}

/*
 * @brief Rebuild the navigagion list from the search result of the latest
 * package list
 * @param appdata The application manager data
 * @param str The search string
 */
void 
navigation_area_rebuild_search_result (ApplicationManagerData *appdata,
                                       const gchar *str)
{
  GtkWidget     *treeview;
  GtkTreeModel  *model;
  GtkListStore  *store;

  gpointer       pkglist;

  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (appdata));

  pkglist = application_manager_data_get_currentlist (appdata);

  treeview = application_manager_get_tvpkglist (appdata);
  g_return_if_fail (GTK_IS_TREE_VIEW (treeview));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
  g_return_if_fail (GTK_IS_TREE_MODEL (model));
  store = GTK_LIST_STORE (model);

  g_object_ref (model);
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), NULL);
  gtk_list_store_clear (store);

  /* FIXME Add search and build the store */
  search_and_translate_package_list_to_store (appdata, store, pkglist, str);

  gtk_tree_view_set_model (GTK_TREE_VIEW(treeview), model);
  g_object_unref (model);
}

/*
 * @brief Increase search for the package list
 * @param appdata The application manager data
 * @param str The search string
 */
void 
navigation_area_increase_search (ApplicationManagerData *appdata,
                                 const gchar *str)
{
  GtkWidget     *treeview;
  GtkTreeModel  *model;
  GtkTreeIter   iter, next;
  gint          res;
  gchar         *pkgname = NULL;

  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (appdata));

  treeview = application_manager_get_tvpkglist (appdata);
  g_return_if_fail (GTK_IS_TREE_VIEW (treeview));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
  g_return_if_fail (GTK_IS_TREE_MODEL (model));

  g_object_ref (model);
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), NULL);

  res = gtk_tree_model_get_iter_first (model, &next);
  while (res)
    {
      iter = next;
      res = gtk_tree_model_iter_next (model, &next);

      gtk_tree_model_get (model, &iter, COL_NAME, &pkgname, -1);
      if (pkgname == NULL)
        {
          g_debug ("Can not the package name in a row");
          continue;
        }
      
      if (strstr ((char *)pkgname, (char *)str) != NULL)
        {
          g_free (pkgname);
          pkgname = NULL;
          continue;
        }

      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
      g_free (pkgname);
      pkgname = NULL;
    }

  gtk_tree_view_set_model (GTK_TREE_VIEW(treeview), model);
  g_object_unref (model);
}

