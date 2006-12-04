/**
 *  @file appmanager-data.c
 *  @brief The all data that the application manager will used
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

#include "appmanager-data.h"

/**
 * @brief Create a new ApplicationManagerData
 * @return The ApplicationManagerData. If fail to create, it will return NULL.
 */
ApplicationManagerData *
application_manager_data_new (void)
{
  ApplicationManagerData *data;
  int i;

  data = g_malloc (sizeof (ApplicationManagerData));
  if (data == NULL)
    {
      g_debug ("Can not malloc memory for the init data struct, process will be abort");
      return NULL;
    }

  data->mwindow = NULL;
  data->tvpkglist = NULL;
  data->tvdetail = NULL;
  data->pkglist = NULL;
  data->sectionlist = NULL;
  data->installedlist = NULL;
  data->upgradelist = NULL;
  data->selectedlist = NULL;

  for (i = 0; i < N_COUNT_PKG_STATUS; i++)
    {
      data->statuspix[i] = NULL;
    }

  return data;
}

/**
 * @brief Set the moko panad window to the application manager data
 * @param appdata The application manager data struct
 * @param window The main window
 */
void 
application_manager_data_set_main_window (ApplicationManagerData *appdata, 
                                          MokoPanedWindow *window)
{
  g_return_if_fail (appdata != NULL);

  appdata->mwindow = window;
}

/**
 * @brief Set the treeview widget of package list to the 
 * application manager data
 * @param appdata The application manager data struct
 * @param tvpkglist The treeview widget of the package list
 */
void 
application_manager_data_set_tvpkglist (ApplicationManagerData *appdata,
                                       GtkWidget *tvpkglist)
{
  g_return_if_fail (appdata != NULL);

  appdata->tvpkglist = tvpkglist;
}

/**
 * @brief Set the textview widget of detail to the application manager data
 * @param appdata The application manager data struct
 * @param tvdetail The textview widget of detail
 */
void 
application_manager_data_set_tvdetail (ApplicationManagerData *appdata,
                                       GtkWidget *tvdetail)
{
  g_return_if_fail (appdata != NULL);

  appdata->tvdetail = tvdetail;
}

/**
 * @brief Set the header of package list to the application manager data
 * @param appdata The application manager data struct
 * @param pkglist The header of the package list
 */
void 
application_manager_data_set_pkglist (ApplicationManagerData *appdata,
                                      gpointer pkglist)
{
  g_return_if_fail (appdata != NULL);

  appdata->pkglist = pkglist;
}

/**
 * @brief Set the header of section list to the application manager data
 * @param appdata The application manager data struct
 * @param sectionlist The header of the section list
 */
void 
application_manager_data_set_section_list (ApplicationManagerData *appdata,
                                           gpointer sectionlist)
{
  g_return_if_fail (appdata != NULL);

  appdata->sectionlist = sectionlist;
}

/**
 * @brief Set the header of the installed list to the application manager data
 * @param appdata The application manager data struct
 * @param installedlist The header of the installed list
 */
void 
application_manager_data_set_installed_list (ApplicationManagerData *appdata,
                                             gpointer installedlist)
{
  g_return_if_fail (appdata != NULL);

  appdata->installedlist = installedlist;
}

/**
 * @brief Set the header of the upgrade list to the application manager data
 * @param appdata The application manager data struct
 * @param upgradelist The header of the upgrade list
 */
void 
application_manager_data_set_upgrade_list (ApplicationManagerData *appdata,
                                           gpointer upgradelist)
{
  g_return_if_fail (appdata != NULL);

  appdata->upgradelist = upgradelist;
}

/**
 * @brief Set the header of the selected list to the application manager data
 * @param appdata The application manager data struct
 * @param selectedlist The header of the selected list
 */
void 
application_manager_data_set_selected_list (ApplicationManagerData *appdata,
                                            gpointer selectedlist)
{
  g_return_if_fail (appdata != NULL);

  appdata->selectedlist = selectedlist;
}

/**
 * @brief Set the pixbuf to the pixbuf list in the application manager data
 * @param appdata The application manager data struct
 * @param pixbuf A GdkPixbuf
 * @param id The package status id
 */
void 
application_manager_data_set_status_pixbuf (ApplicationManagerData *appdata,
                                            GdkPixbuf *pixbuf,
                                            guint id)
{
  g_return_if_fail (appdata != NULL);
  g_return_if_fail (id < N_COUNT_PKG_STATUS);

  appdata->statuspix[id] = pixbuf;
}

/**
 * @brief Init the pixbuf list.
 *
 * Load all pixbuf from the appointed file, and set the pixbuf to the
 * application manager data struct.
 * @param appdata The application manager data struct
 */
void 
init_pixbuf_list (ApplicationManagerData *appdata)
{
  GdkPixbuf  *pixbuf;

  g_return_if_fail (appdata != NULL);

  pixbuf = create_pixbuf ("package-available.png");
  if (pixbuf != NULL)
    {
      application_manager_data_set_status_pixbuf (appdata, pixbuf,
                                                  PKG_STATUS_AVAILABLE);
    }

  pixbuf = create_pixbuf ("package-installed.png");
  if (pixbuf != NULL)
    {
      application_manager_data_set_status_pixbuf (appdata, pixbuf,
                                                  PKG_STATUS_INSTALLED);
    }

  pixbuf = create_pixbuf ("package-installed-outdated.png");
  if (pixbuf != NULL)
    {
      application_manager_data_set_status_pixbuf (appdata, pixbuf,
                                                  PKG_STATUS_UPGRADEABLE);
    }

  pixbuf = create_pixbuf ("package-mark-install.png");
  if (pixbuf != NULL)
    {
      application_manager_data_set_status_pixbuf (appdata, pixbuf,
                                                  PKG_STATUS_AVAILABLE_MARK_FOR_INSTALL);
    }

  pixbuf = create_pixbuf ("package-remove.png");
  if (pixbuf != NULL)
    {
      application_manager_data_set_status_pixbuf (appdata, pixbuf,
                                                  PKG_STATUS_INSTALLED_MARK_FOR_REMOVE);
      application_manager_data_set_status_pixbuf (appdata, pixbuf,
                                                  PKG_STATUS_UPGRADEABLE_MARK_FOR_REMOVE);
    }

  pixbuf = create_pixbuf ("package-upgrade.png");
  if (pixbuf != NULL)
    {
      application_manager_data_set_status_pixbuf (appdata, pixbuf,
                                                  PKG_STATUS_UPGRADEABLE_MARK_FOR_UPGRADE);
    }

}

/**
 * @brief Get the main window from the application manager data
 * @param appdata The application manager data
 * @return The main window
 */
MokoPanedWindow *
application_manager_get_main_window (ApplicationManagerData *appdata)
{
  g_return_val_if_fail (appdata != NULL, NULL);

  return appdata->mwindow;
}

/**
 * @brief Get the treeview widget of the package list from 
 * the application manager data
 *
 * @param appdata The application manager data
 * @return The treeview widget
 */
GtkWidget *
application_manager_get_tvpkglist (ApplicationManagerData *appdata)
{
  g_return_val_if_fail (appdata != NULL, NULL);

  return appdata->tvpkglist;
}

/**
 * @brief Get the textview widget of the details info from 
 * the application manager data
 * 
 * @param appdata The application manager data
 * @return The text widget
 */
GtkWidget *
application_manager_get_tvdetail (ApplicationManagerData *appdata)
{
  g_return_val_if_fail (appdata != NULL, NULL);

  return appdata->tvdetail;
}

/**
 * @brief Get the package list that get from libipkg from the 
 * application manager data
 *
 * @param appdata The application manager data
 * @return The package list that get from libipkg
 */
gpointer 
application_manager_data_get_pkglist (ApplicationManagerData *appdata)
{
  g_return_val_if_fail (appdata != NULL, NULL);

  return appdata->pkglist;
}

/**
 * @brief Get the section list from the application manager data
 * @param appdata The application manager data
 * @return The section list
 */
gpointer 
application_manager_data_get_sectionlist (ApplicationManagerData *appdata)
{
  g_return_val_if_fail (appdata != NULL, NULL);

  return appdata->sectionlist;
}

/**
 * @brief Get the installed list from the application manager data
 * @param appdata The application manager data
 * @return The installed list
 */
gpointer 
application_manager_data_get_installedlist (ApplicationManagerData *appdata)
{
  g_return_val_if_fail (appdata != NULL, NULL);

  return appdata->installedlist;
}

/**
 * @brief Get the upgrade list from the application manager data
 * @param appdata The application manager data
 * @return The upgrade list
 */
gpointer 
application_manager_data_get_upgradelist (ApplicationManagerData *appdata)
{
  g_return_val_if_fail (appdata != NULL, NULL);

  return appdata->upgradelist;
}

/**
 * @brief Get the selected list from the application manager data
 * @param appdata The application manager data
 * @return The selected list
 */
gpointer 
application_manager_data_get_selectedlist (ApplicationManagerData *appdata)
{
  g_return_val_if_fail (appdata != NULL, NULL);

  return appdata->selectedlist;
}

/**
 * @brief Get the appointed pixbuf from the application manager data
 * @param appdata The application manager data
 * @param id The status id
 * @return The pixbuf. If the id is an error id, it returns NULL
 */
GdkPixbuf *
application_manager_data_get_status_pixbuf (ApplicationManagerData *appdata, 
                                            guint id)
{
  g_return_val_if_fail (appdata != NULL, NULL);
  g_return_val_if_fail (id < N_COUNT_PKG_STATUS, NULL);

  return appdata->statuspix[id];
}
