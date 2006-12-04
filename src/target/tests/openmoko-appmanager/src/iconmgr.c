/**
 * @file iconmgr.c 
 * @brief Manage all icons that will be used in the 
 * application manager.
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
 * @date 2006-10-10
 */

#include "support.h"
#include "iconmgr.h"
#include "widgets.h"

static GdkPixbuf *pkgstatus[N_COUNT_PKG_STATUS];

/**
 * @brief Update the application icon with the specified file.
 *
 * @param filename The name of picture file. The file must locate
 * in the /usr/local/share/appmgr/pixmaps or /usr/share/pixmaps.
 */
void
update_application_icon_from_file (const char *filename)
{
  GtkWidget *image;
  GdkPixbuf *pixbuf;

  image = get_widget_pointer (FIC_WIDGET_IMAGE_IMAGE1);
  g_return_if_fail (image != NULL);

  pixbuf = create_pixbuf (filename);

  gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);

}

/**
 * @brief Load the icons of package status into memory.
 */
void
init_package_status_icon (void)
{
  pkgstatus[PKG_STATUS_AVAILABLE] = create_pixbuf ("package-available.png");
  if (pkgstatus[PKG_STATUS_AVAILABLE] == NULL)
    {
      g_warning ("Can't init the picture of package status:available");
    }

  pkgstatus[PKG_STATUS_INSTALLED] = create_pixbuf ("package-installed-updated.png");
  if (pkgstatus[PKG_STATUS_INSTALLED] == NULL)
    {
      g_warning ("Can't init the picture of package status:installed");
    }

  pkgstatus[PKG_STATUS_UPGRADEABLE] = create_pixbuf ("package-installed-outdated.png");
  if (pkgstatus[PKG_STATUS_UPGRADEABLE] == NULL)
    {
      g_warning ("Can't init the picture of package status:upgradeable");
    }

  pkgstatus[PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL] = create_pixbuf ("package-new.png");
  if (pkgstatus[PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL] == NULL)
    {
      g_warning ("Can't init the picture of package status:mark for install");
    }

  pkgstatus[PKG_STATUS_INSTALLED_MARK_FOR_REMOVE] = create_pixbuf ("package-remove.png");
  if (pkgstatus[PKG_STATUS_INSTALLED_MARK_FOR_REMOVE] == NULL)
    {
      g_warning ("Can't init the picture of package status:mark for remove");
    }

  pkgstatus[PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE] = create_pixbuf ("package-reinstall.png");
  if (pkgstatus[PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE] == NULL)
    {
      g_warning ("Can't init the picture of package status:mark for upgrade");
    }

  pkgstatus[PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE] = pkgstatus[PKG_STATUS_INSTALLED_MARK_FOR_REMOVE];
}

/**
 * @brief Get the icon that spacified.
 */
GdkPixbuf *
get_package_status_icon (PACKAGE_STATUS_ID id)
{
  g_return_val_if_fail (id < N_COUNT_PKG_STATUS, NULL);

  return pkgstatus[id];
}
