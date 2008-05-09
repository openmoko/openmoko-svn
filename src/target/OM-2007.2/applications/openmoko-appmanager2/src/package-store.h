/*
 *  @file package-store.h
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

#ifndef PACKAGE_STORE_H
#define PACKAGE_STORE_H

#include <gtk/gtk.h>
#include <libopkg/opkg.h>

enum {
  COL_STATUS = 0,
  COL_NAME,
  COL_POINTER,
  NUM_COL
};


GtkTreeModel * package_store_new (opkg_t *opkg);

#endif /* PACKAGE_STORE_H */
