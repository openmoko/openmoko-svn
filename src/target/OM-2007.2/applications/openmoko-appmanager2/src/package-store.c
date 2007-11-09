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
#include "ipkgapi.h"
#include <glib.h>

GtkTreeModel *
package_store_new ()
{
  GtkListStore *store;
  IPK_PACKAGE *pkg;
  PKG_LIST_HEAD list;
  int ret;
  GRegex *regex;
  
  ipkg_initialize (0);
  
  /* status, name, size, pkg */
  store = gtk_list_store_new (NUM_COL, G_TYPE_INT,
                              G_TYPE_STRING, G_TYPE_POINTER);

  ret = ipkg_list_available_cmd (&list);
  g_return_val_if_fail (ret >= 0, NULL);
  
  pkg = list.pkg_list;
  
  regex = g_regex_new ("(-doc$|-dev$|-dbg$|-locale)", G_REGEX_OPTIMIZE, 0, NULL);
  
  while (pkg)
  {
    if (!g_regex_match (regex, pkg->name, 0, NULL))
      gtk_list_store_insert_with_values (store, NULL, -1,
                                       COL_STATUS, pkg->state_status,
                                       COL_NAME, pkg->name,
                                       COL_POINTER, pkg,
                                       -1);
    pkg = pkg->next;
  }
  
  g_regex_unref (regex);

  return GTK_TREE_MODEL (store);
}
