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
      data->statusPix[i] = NULL;
    }

  return data;
}

void 
application_manager_data_set_main_window (ApplicationManagerData *appdata, 
                                          MokoPanedWindow *window)
{
  appdata->mwindow = window;
}

void 
application_manager_data_set_tvpkglist (ApplicationManagerData *appdata,
                                       GtkWidget *tvpkglist)
{
  appdata->tvpkglist = tvpkglist;
}

void 
application_manager_data_set_tvdetail (ApplicationManagerData *appdata,
                                       GtkWidget *tvdetail)
{
  appdata->tvdetail = tvdetail;
}

void 
application_manager_data_set_pkglist (ApplicationManagerData *appdata,
                                      gpointer pkglist)
{
  appdata->pkglist = pkglist;
}

void 
application_manager_data_set_section_list (ApplicationManagerData *appdata,
                                           gpointer sectionlist)
{
  appdata->sectionlist = sectionlist;
}

void 
application_manager_data_set_installed_list (ApplicationManagerData *appdata,
                                             gpointer installedlist)
{
  appdata->installedlist = installedlist;
}

void 
application_manager_data_set_upgrade_list (ApplicationManagerData *appdata,
                                           gpointer upgradelist)
{
  appdata->upgradelist = upgradelist;
}

void 
application_manager_data_set_selected_list (ApplicationManagerData *appdata,
                                            gpointer selectedlist)
{
  appdata->selectedlist = selectedlist;
}
