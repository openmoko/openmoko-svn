/*
 *  @file filter-menu.h
 *  @brief The filter menu item
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
#ifndef _FIC_FILTER_MENU_H
#define _FIC_FILTER_MENU_H

#include <gtk/gtk.h>

#include "appmanager-data.h"

GtkWidget *filter_menu_new (ApplicationManagerData *appdata);

void filter_menu_add_item (GtkMenu *filtermenu, const gchar *name,
                           ApplicationManagerData *appdata);

void
filter_menu_show_install_list (ApplicationManagerData *appdata);

#endif

