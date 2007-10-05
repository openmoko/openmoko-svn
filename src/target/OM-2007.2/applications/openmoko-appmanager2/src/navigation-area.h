/*
 *  @file navigation-area.h
 *  @brief The navigation area in the main window
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
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
#ifndef _FIC_NAVIGATION_AREA_H
#define _FIC_NAVIGATION_AREA_H

#include <gtk/gtk.h>

#include "appmanager-data.h"

enum {
  COL_STATUS = 0,
  COL_NAME,
  COL_SIZE,
  COL_POINTER,
  NUM_COL
};

GtkWidget *navigation_area_new (ApplicationManagerData *appdata);

gint navigation_area_insert_test_data (ApplicationManagerData *appdata);

gchar *treeview_get_selected_name (GtkWidget *treeview);

void navigation_area_refresh_with_package_list (ApplicationManagerData *appdata, 
                                                gpointer pkglist);

void navigation_area_rebuild_from_latest (ApplicationManagerData *appdata);

void navigation_area_rebuild_search_result (ApplicationManagerData *appdata,
                                            const gchar *str);

void 
navigation_area_increase_search (ApplicationManagerData *appdata,
                                 const gchar *str);

#endif

