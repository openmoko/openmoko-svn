/**
 * @file iconmgr.h 
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

#ifndef _FIC_ICONMGR_H
#define _FIC_ICONMGR_H

#include <gtk/gtk.h>

/**
 * @brief The id of package status.
 */
typedef enum package_status_id {
  PKG_STATUS_AVAILABLE = 0,        ///<Package is available and not installed.

  PKG_STATUS_INSTALLED,           ///<Package is installed and can not be upgrade.

  PKG_STATUS_UPGRADEABLE,    ///<Package is installed and can be upgrade.

  PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL,   ///<Available package is marked for install.

  PKG_STATUS_INSTALLED_MARK_FOR_REMOVE,    ///<Installed package is marked for remove.

  PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE,  ///<Upgradeable package is marked for upgrade.

  PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE,   ///<Upgradeable package is mark for remove.

  N_COUNT_PKG_STATUS     ///<The number of valid status.
} PACKAGE_STATUS_ID;


void update_application_icon_from_file (const char *filename);

void init_package_status_icon (void);

GdkPixbuf * get_package_status_icon (PACKAGE_STATUS_ID id);

#endif //_FIC_ICONMGR_H
