/**
 *  @file package-list.h
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
#ifndef _FIC_PACKAGE_LIST_H
#define _FIC_PACKAGE_LIST_H

#include <gtk/gtk.h>

#define PACKAGE_LIST_NO_SECTION_STRING "no section"

gint init_package_list (ApplicationManagerData *appdata);

gint package_list_build_index (ApplicationManagerData *appdata);

void package_list_add_section_to_filter_menu (ApplicationManagerData *appdata);

void translate_package_list_to_store (ApplicationManagerData *appdata, 
                                      GtkListStore *store, 
                                      gpointer pkglist);

gpointer package_list_get_with_name (ApplicationManagerData *appdata,
                                     const gchar *name);

gint package_list_get_package_status (gpointer data);

#endif
