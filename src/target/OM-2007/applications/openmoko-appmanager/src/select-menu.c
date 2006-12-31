/**
 *  @file select-menu.c
 *  @brief The select menu that popuo in the treeview
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

#include "select-menu.h"
#include "appmanager-window.h"
#include "pixbuf-list.h"
#include "package-list.h"
#include "navigation-area.h"

static void moko_select_menu_class_init (MokoSelectMenuClass *klass);
static void moko_select_menu_init (MokoSelectMenu *data);

G_DEFINE_TYPE (MokoSelectMenu, moko_select_menu, GTK_TYPE_MENU)

#define MOKO_SELECT_MENU_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOKO_TYPE_SELECT_MENU, MokoSelectMenuPriv))

typedef struct _MokoSelectMenuPriv {
  GtkWidget   *unmark;
  GtkWidget   *markinstall;
  GtkWidget   *markupgrade;
  GtkWidget   *markremove;
} MokoSelectMenuPriv;

/**
 * @brief The callback function of the ummark menuitem
 */
void 
on_unmark_activate (GtkMenuItem *unmark, gpointer data)
{
  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter      iter;
  gpointer         pkg = NULL;
  GtkWidget        *treeview;
  GdkPixbuf        *pkgpix;

  ApplicationManagerData  *appdata;
  PkgStatusId      pkgid;

  g_debug ("The unmark menuitem activated");

  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (data));
  appdata = MOKO_APPLICATION_MANAGER_DATA (data);

  treeview = application_manager_get_tvpkglist (appdata);
  g_return_if_fail (MOKO_IS_TREE_VIEW (treeview));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      return;
    }

  gtk_tree_model_get (model, &iter, COL_POINTER, (gpointer *)&pkg, -1);
  g_return_if_fail (pkg != NULL);

  pkgid = package_list_get_package_status (pkg);
  switch (pkgid)
    {
      case PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL:
        pkgid = PKG_STATUS_AVAILABLE;
        break;

      case PKG_STATUS_INSTALLED_MARK_FOR_REMOVE:
        pkgid = PKG_STATUS_INSTALLED;
        break;

      case PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE:
      case PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE:
        pkgid = PKG_STATUS_UPGRADEABLE;
        break;

      default:
        pkgid = PKG_STATUS_AVAILABLE;
    }

  package_list_set_package_status (pkg, pkgid);
  pkgpix = application_manager_data_get_status_pixbuf (appdata, pkgid);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COL_STATUS, pkgpix,
                      -1);

  package_list_remove_package_from_selected_list (appdata, pkg);
}

/**
 * @brief The callback function of the mark install menuitem
 */
void 
on_mark_install_activate (GtkMenuItem *markinstall, gpointer data)
{
  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter      iter;
  gpointer         pkg = NULL;
  GtkWidget        *treeview;
  GdkPixbuf        *pkgpix;

  ApplicationManagerData  *appdata;
  PkgStatusId      pkgid;

  g_debug ("The mark install menuitem activated");

  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (data));
  appdata = MOKO_APPLICATION_MANAGER_DATA (data);

  treeview = application_manager_get_tvpkglist (appdata);
  g_return_if_fail (MOKO_IS_TREE_VIEW (treeview));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      return;
    }

  gtk_tree_model_get (model, &iter, COL_POINTER, (gpointer *)&pkg, -1);
  g_return_if_fail (pkg != NULL);

  pkgid = package_list_get_package_status (pkg);
  switch (pkgid)
    {
      case PKG_STATUS_AVAILABLE:
        pkgid = PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL;
        // FIXME Add the package to the mark list
        package_list_add_node_to_selected_list (appdata, pkg);
        break;

      default:
        pkgid = PKG_STATUS_AVAILABLE;
    }

  package_list_set_package_status (pkg, pkgid);
  pkgpix = application_manager_data_get_status_pixbuf (appdata, pkgid);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COL_STATUS, pkgpix,
                      -1);
}

/**
 * @brief The callback function of the mark upgrade menuitem
 */
void 
on_mark_upgrade_activate (GtkMenuItem *markupgrade, gpointer data)
{
  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter      iter;
  gpointer         pkg = NULL;
  GtkWidget        *treeview;
  GdkPixbuf        *pkgpix;

  ApplicationManagerData  *appdata;
  PkgStatusId      pkgid;

  g_debug ("The mark upgrade menuitem activated");

  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (data));
  appdata = MOKO_APPLICATION_MANAGER_DATA (data);

  treeview = application_manager_get_tvpkglist (appdata);
  g_return_if_fail (MOKO_IS_TREE_VIEW (treeview));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      return;
    }

  gtk_tree_model_get (model, &iter, COL_POINTER, (gpointer *)&pkg, -1);
  g_return_if_fail (pkg != NULL);

  pkgid = package_list_get_package_status (pkg);
  switch (pkgid)
    {
      case PKG_STATUS_UPGRADEABLE:
        //FIXME Add insert pkg to the select list
        package_list_add_node_to_selected_list (appdata, pkg);

      case PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE:
        pkgid = PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE;
        break;

      default:
        pkgid = PKG_STATUS_AVAILABLE;
    }

  package_list_set_package_status (pkg, pkgid);
  pkgpix = application_manager_data_get_status_pixbuf (appdata, pkgid);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COL_STATUS, pkgpix,
                      -1);
}

/**
 * @brief The callback function of the mark remove menuitem
 */
void 
on_mark_remove_activate (GtkMenuItem *markremove, gpointer data)
{
  GtkTreeModel     *model;
  GtkTreeSelection *selection;
  GtkTreeIter      iter;
  gpointer         pkg = NULL;
  GtkWidget        *treeview;
  GdkPixbuf        *pkgpix;

  ApplicationManagerData  *appdata;
  PkgStatusId      pkgid;

  g_debug ("The mark remove menuitem activated");

  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (data));
  appdata = MOKO_APPLICATION_MANAGER_DATA (data);

  treeview = application_manager_get_tvpkglist (appdata);
  g_return_if_fail (MOKO_IS_TREE_VIEW (treeview));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      return;
    }

  gtk_tree_model_get (model, &iter, COL_POINTER, (gpointer *)&pkg, -1);
  g_return_if_fail (pkg != NULL);

  pkgid = package_list_get_package_status (pkg);
  switch (pkgid)
    {
      case PKG_STATUS_INSTALLED:
        package_list_add_node_to_selected_list (appdata, pkg);
        pkgid = PKG_STATUS_INSTALLED_MARK_FOR_REMOVE;
        break;

      case PKG_STATUS_UPGRADEABLE:
        package_list_add_node_to_selected_list (appdata, pkg);
      case PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE:
        pkgid = PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE;
        break;

      default:
        pkgid = PKG_STATUS_AVAILABLE;
    }

  package_list_set_package_status (pkg, pkgid);
  pkgpix = application_manager_data_get_status_pixbuf (appdata, pkgid);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COL_STATUS, pkgpix,
                      -1);
}

static void 
moko_select_menu_class_init (MokoSelectMenuClass *klass)
{
  g_type_class_add_private (klass, sizeof (MokoSelectMenuPriv));
}

static void 
moko_select_menu_init (MokoSelectMenu *data)
{
  g_debug ("New a moko select menu");
}

/**
 * @brief Create a new select menu
 * @param appdata The application manager data
 * @return The MokoSelectMenu
 */
MokoSelectMenu *
moko_select_menu_new (ApplicationManagerData *appdata)
{
  MokoSelectMenu *self = MOKO_SELECT_MENU (g_object_new (MOKO_TYPE_SELECT_MENU, NULL));
  MokoSelectMenuPriv *priv = MOKO_SELECT_MENU_GET_PRIVATE (self);

  priv->unmark = gtk_menu_item_new_with_mnemonic (_("unmark"));
  gtk_widget_show (priv->unmark);
  gtk_container_add (GTK_CONTAINER (self), priv->unmark);
  g_signal_connect ((gpointer) priv->unmark, "activate",
                    G_CALLBACK (on_unmark_activate), appdata);

  priv->markinstall = gtk_menu_item_new_with_mnemonic (_("mark install"));
  gtk_widget_show (priv->markinstall);
  gtk_container_add (GTK_CONTAINER (self), priv->markinstall);
  g_signal_connect ((gpointer) priv->markinstall, "activate",
                    G_CALLBACK (on_mark_install_activate), appdata);

  priv->markupgrade = gtk_menu_item_new_with_mnemonic (_("mark upgrade"));
  gtk_widget_show (priv->markupgrade);
  gtk_container_add (GTK_CONTAINER (self), priv->markupgrade);
  g_signal_connect ((gpointer) priv->markupgrade, "activate",
                    G_CALLBACK (on_mark_upgrade_activate), appdata);

  priv->markremove = gtk_menu_item_new_with_mnemonic (_("mark remove"));
  gtk_widget_show (priv->markremove);
  gtk_container_add (GTK_CONTAINER (self), priv->markremove);
  g_signal_connect ((gpointer) priv->markremove, "activate",
                    G_CALLBACK (on_mark_remove_activate), appdata);

  return self;
}

/**
 * @brief Popup the select menu
 * @param menu The select menu
 * @param appdata The application manager data
 * @param pkg The package that been selected
 */
void 
moko_select_menu_popup (MokoSelectMenu *menu, 
                        GdkEventButton *event,
                        ApplicationManagerData *appdata, 
                        gpointer pkg)
{
  MokoSelectMenuPriv *priv;
  gint               pkgid;

  g_return_if_fail (MOKO_IS_APPLICATION_MANAGER_DATA (appdata));
  g_return_if_fail (MOKO_IS_SELECT_MENU (menu));

  priv = MOKO_SELECT_MENU_GET_PRIVATE (menu);

  pkgid = package_list_get_package_status (pkg);
  g_debug ("aaaa");

  switch (pkgid)
    {
      case PKG_STATUS_AVAILABLE:
        gtk_widget_set_sensitive (GTK_WIDGET (priv->unmark), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markinstall), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markupgrade), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markremove), FALSE);
        break;

      case PKG_STATUS_INSTALLED:
        gtk_widget_set_sensitive (GTK_WIDGET (priv->unmark), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markinstall), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markupgrade), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markremove), TRUE);
        break;

      case PKG_STATUS_UPGRADEABLE:
        gtk_widget_set_sensitive (GTK_WIDGET (priv->unmark), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markinstall), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markupgrade), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markremove), TRUE);
        break;

      case PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL:
        gtk_widget_set_sensitive (GTK_WIDGET (priv->unmark), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markinstall), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markupgrade), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markremove), FALSE);
        break;

      case PKG_STATUS_INSTALLED_MARK_FOR_REMOVE:
        gtk_widget_set_sensitive (GTK_WIDGET (priv->unmark), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markinstall), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markupgrade), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markremove), FALSE);
        break;

      case PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE:
        gtk_widget_set_sensitive (GTK_WIDGET (priv->unmark), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markinstall), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markupgrade), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markremove), TRUE);
        break;

      case PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE:
        gtk_widget_set_sensitive (GTK_WIDGET (priv->unmark), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markinstall), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markupgrade), TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markremove), FALSE);
        break;

      default:
        g_debug ("Error package status");
        gtk_widget_set_sensitive (GTK_WIDGET (priv->unmark), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markinstall), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markupgrade), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (priv->markremove), FALSE);
    }

  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, 0,
                  (event != NULL) ? event->button : 0,
                  gdk_event_get_time((GdkEvent *) event));
}

