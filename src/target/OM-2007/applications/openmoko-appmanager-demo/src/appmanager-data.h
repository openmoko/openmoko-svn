/**
 *  @file appmanager-data.h
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
#ifndef _FIC_APPMANAGER_DATA_H
#define _FIC_APPMANAGER_DATA_H

#include <gtk/gtk.h>

#include <libmokoui/moko-paned-window.h>

/**
 * @brief The all data that will be used in the application manager
 *
 * 
 */
typedef struct _ApplicationManagerData {
  MokoPanedWindow  *mwindow;           ///<! The main window
  GtkWidget        *tvpkglist;         ///<! The treeview of the package list
  GtkWidget        *tvdetail;          ///<! The textview of the details info
  gpointer         pkglist;            ///<! The package list get from lib ipkg
  gpointer         sectionlist;        ///<! The section list parse from the package list
  gpointer         installedlist;      ///<! The list of all installed packages
  gpointer         upgradelist;        ///<! The list of all upgradeable packages
  gpointer         selectedlist;       ///<! The list of packages that user selected
  GdkPixbuf        *statusPix[PkgStatusId];    ///<! The all pixbufs that need by the package list store
} ApplicationManagerData;

ApplicationManagerData *application_manager_data_new (void);

void application_manager_data_set_main_window (ApplicationManagerData *appdata, 
                                               MokoPanedWindow *window);

void application_manager_data_set_tvpkglist (ApplicationManagerData *appdata,
                                             GtkWidget *tvpkglist);

void application_manager_data_set_tvdetail (ApplicationManagerData *appdata,
                                            GtkWidget *tvdetail);

void application_manager_data_set_pkglist (ApplicationManagerData *appdata,
                                           gpointer pkglist);

void application_manager_data_set_section_list (ApplicationManagerData *appdata,
                                                gpointer sectionlist);

void application_manager_data_set_installed_list (ApplicationManagerData *appdata,
                                                  gpointer installedlist);

void application_manager_data_set_upgrade_list (ApplicationManagerData *appdata,
                                                gpointer upgradelist);

void application_manager_data_set_selected_list (ApplicationManagerData *appdata,
                                                 gpointer selectedlist);

#endif
