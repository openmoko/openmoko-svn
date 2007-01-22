/**
 *  @file navigation-area.c
 *  @brief The navigation area in the main window
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 *  @author Chaowei Song (songcw@fic-sh.com.cn)
 */
#include <libmokoui/moko-tree-view.h>
#include <string.h>

#include "appmanager-window.h"
#include "navigation-area.h"
#include "errorcode.h"
#include "detail-area.h"
#include "package-list.h"
#include "select-menu.h"

/**
 * @brief The callback function of the signal "cursor-changed"
 */
void 
on_treeview_cursor_changed (GtkTreeView *treeview, 
                            gpointer     user_data)
{
  GtkTreeModel     *model;
  GtkTreeIter      iter;
  GtkTreeSelection *selection;
  gpointer         pkg;

  g_debug ("Call the on_treeview_cursor_changed");
  selection = gtk_tree_view_get_selection (treeview);

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, COL_POINTER, &pkg, -1);
      detail_area_update_info ((ApplicationManagerData *) user_data, pkg);
    }
}

/**
 * @brief The callback function of the signal "unselect-all"
 */
gboolean 
on_treeview_unselect_all (GtkTreeView *treeview,
                          gpointer     user_data)
{
  g_debug ("Call the on_treeview_unselect_all");

  return FALSE;
}

/**
 * @brief The callback function of the signal "button_press_event"
 */
gboolean 
on_treeview_button_press_event (GtkWidget *treeview, 
                                GdkEventButton *event,
                                gpointer data)
{
  g_return_val_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (data), FALSE);

  if (event->type == GDK_BUTTON_PRESS)
    {
      GtkTreeSelection *selection;
      GtkTreeIter iter;
      GtkTreePath *path;
      GtkTreeModel  *model;
      GtkTreeViewColumn *column;
      GtkTreeViewColumn *firstcol;
      gpointer  pkg;

      GtkMenu   *selectmenu;

      if (!(event->window == gtk_tree_view_get_bin_window (GTK_TREE_VIEW (treeview))))
        {
          g_debug ("Not a package list view event");
          return FALSE;
        }

      if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (treeview),
                                         (int)event->x, (int)event->y,
                                         &path, &column, NULL, NULL))
        {
          selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
          firstcol = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), 0);
          gtk_tree_selection_unselect_all (selection);
          gtk_tree_view_set_cursor (GTK_TREE_VIEW (treeview), path, NULL, FALSE);

          if (!((event->button == 1) && (strcmp(firstcol->title, column->title) == 0)))
            {
              return TRUE;
            }

          if (gtk_tree_selection_get_selected (selection, &model, &iter))
            {
              gtk_tree_model_get (model, &iter, COL_POINTER, &pkg, -1);
              selectmenu = application_manager_get_select_menu (
                              MOKO_APPLICATION_MANAGER_DATA (data));
              g_return_val_if_fail (MOKO_IS_SELECT_MENU (selectmenu), TRUE);
              g_debug ("popup menu");
              moko_select_menu_popup (MOKO_SELECT_MENU (selectmenu), 
                                      event, 
                                      MOKO_APPLICATION_MANAGER_DATA (data),
                                      pkg);
            }
          return TRUE;
        }

    }

  return FALSE;
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

  /* Add the status as the first column. */
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("S"));
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "pixbuf", COL_STATUS,
                                       NULL);

  moko_tree_view_append_column (MOKO_TREE_VIEW (treeview), col);

  /* For some reason, there must set the column length to fixed */
  gtk_tree_view_column_set_resizable (col, FALSE);
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (col, 20);

  /* Add the name as the second column. */
  col = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (col, _("Name"));
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_set_attributes (col, renderer,
                                       "text", COL_NAME,
                                       NULL);

  moko_tree_view_append_column (MOKO_TREE_VIEW (treeview), col);

  /* For some reason, there must set the column length to fixed */
  gtk_tree_view_column_set_resizable (col, FALSE);
  gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_fixed_width (col, 240);

  /* Add the size as the third column. */
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
  /* FIXME Set the treeview as the single selection mode now.
     Maybe it uses the multi selection mode in the feature. */
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
                               GTK_SELECTION_SINGLE);

  scrollwindow = GTK_WIDGET (moko_tree_view_put_into_scrolled_window (MOKO_TREE_VIEW (treeview)));
  application_manager_data_set_tvpkglist (appdata, treeview);

  /* Connect signal to the treeview */
  g_signal_connect ((gpointer) treeview, "cursor_changed",
                    G_CALLBACK (on_treeview_cursor_changed),
                    appdata);

  g_signal_connect ((gpointer) treeview, "unselect_all",
                    G_CALLBACK (on_treeview_cursor_changed),
                    appdata);

  g_signal_connect ((gpointer) treeview, "button_press_event",
                    G_CALLBACK (on_treeview_button_press_event),
                    appdata);

  return scrollwindow;
}

/**
 * @brief Insert test data
 */
gint 
navigation_area_insert_test_data (ApplicationManagerData *appdata)
{
  GtkWidget  *treeview;

  GtkTreeModel  *model;
  GtkListStore  *store;
  GtkTreeIter   iter;
  GdkPixbuf     *pix = NULL;

  treeview = application_manager_get_tvpkglist (appdata);
  if (treeview == NULL)
    {
      g_debug ("Treeview not init correctly");
      return OP_ERROR;
    }
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
  store = GTK_LIST_STORE (model);
  if (store == NULL)
    {
      g_debug ("The store of package list not init correctly");
      return OP_ERROR;
    }

  g_object_ref (model);
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), NULL);
  gtk_list_store_clear (store);

  pix = application_manager_data_get_status_pixbuf (appdata, PKG_STATUS_AVAILABLE);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COL_STATUS, pix,
                      COL_NAME, "test-core",
                      COL_SIZE, "11k",
                      COL_POINTER, NULL,
                      -1);

  pix = application_manager_data_get_status_pixbuf (appdata, PKG_STATUS_INSTALLED);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COL_STATUS, pix,
                      COL_NAME, "test-vim",
                      COL_SIZE, "21k",
                      COL_POINTER, NULL,
                      -1);

  pix = application_manager_data_get_status_pixbuf (appdata, PKG_STATUS_UPGRADEABLE);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COL_STATUS, pix,
                      COL_NAME, "test-game",
                      COL_SIZE, "188k",
                      COL_POINTER, NULL,
                      -1);

  pix = application_manager_data_get_status_pixbuf (appdata, PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COL_STATUS, pix,
                      COL_NAME, "test-dialer",
                      COL_SIZE, "211k",
                      COL_POINTER, NULL,
                      -1);

  gtk_tree_view_set_model (GTK_TREE_VIEW(treeview), model);
  g_object_unref (model);

  return OP_SUCCESS;
}

/**
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

/**
 * @brief Refresh the navigation area with the package list
 * @param appdata The application manager data
 * @param pkglist The pachage list
 */
void 
navigation_area_refresh_with_package_list (ApplicationManagerData *appdata, 
                                           gpointer pkglist)
{
  GtkWidget     *treeview;
  GtkTreeModel  *model;
  GtkListStore  *store;

  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (appdata));
  g_return_if_fail (pkglist != NULL);

  treeview = application_manager_get_tvpkglist (appdata);
  g_return_if_fail (GTK_IS_TREE_VIEW (treeview));

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
  g_return_if_fail (GTK_IS_TREE_MODEL (model));
  store = GTK_LIST_STORE (model);

  g_object_ref (model);
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), NULL);
  gtk_list_store_clear (store);

  translate_package_list_to_store (appdata, store, pkglist);
  /* Save current list to the application manager data */
  application_manager_data_set_current_list (appdata, pkglist);

  gtk_tree_view_set_model (GTK_TREE_VIEW(treeview), model);
  g_object_unref (model);
}

/**
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

/**
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

/**
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

