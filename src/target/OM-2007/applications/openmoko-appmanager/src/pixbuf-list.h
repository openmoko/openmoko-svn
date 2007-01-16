/**
 *  @file pixbuf-list.h
 *  @brief The package list that get from the lib ipkg
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
#ifndef _FIC_PIXBUF_LIST_H
#define _FIC_PIXBUF_LIST_H

#include <gtk/gtk.h>

/**
 * @brief The package status id
 *
 * The id is used to find the related pixbuf from pixbuf list.
 */
typedef enum _pkgstatusid {
  PKG_STATUS_AVAILABLE = 0,                /* Package is available and not installed. */
  PKG_STATUS_INSTALLED,                    /* Package is installed and can not be upgrade. */
  PKG_STATUS_UPGRADEABLE,                  /* Package is installed and can be upgrade. */
  PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL,   /* Available package is marked for install. */
  PKG_STATUS_INSTALLED_MARK_FOR_REMOVE,    /* Installed package is marked for remove. */
  PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE, /* Upgradeable package is marked for upgrade. */
  PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE,  /* Upgradeable package is mark for remove. */
  N_COUNT_PKG_STATUS                       /* The number of valid status. */
} PkgStatusId;

GdkPixbuf *create_pixbuf (const gchar *filename);


#endif

