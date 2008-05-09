/*
 *  @file package-store.c
 *  @brief A GTK+ list store that contains the list of available packages
 *
 *  Copyright (C) 2007 OpenMoko Inc.
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
 *    OpenedHand Ltd. <info@openedhand.com>
 */

#include "package-store.h"
#include <libopkg/opkg.h>
#include <glib.h>

void
opkg_package_callback (opkg_t *opkg, opkg_package_t *pkg, void *user_data)
{
  GtkListStore *store = GTK_LIST_STORE (user_data);

  gtk_list_store_insert_with_values (store, NULL, -1,
                                     COL_STATUS, pkg->installed,
                                     COL_NAME, pkg->name,
                                     COL_POINTER, pkg,
                                     -1);
}

GtkTreeModel *
package_store_new (opkg_t *opkg)
{
  GtkListStore *store;
  
  /* status, name, size, pkg */
  store = gtk_list_store_new (NUM_COL, G_TYPE_INT,
                              G_TYPE_STRING, G_TYPE_POINTER);

  opkg_list_packages (opkg, opkg_package_callback, store);

  return GTK_TREE_MODEL (store);
}
