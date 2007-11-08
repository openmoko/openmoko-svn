/*
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
 *  Author: OpenedHand Ltd. <info@openedhand.com>
 */

#ifndef SEARCH_BAR_H
#define SEARCH_BAR_H

#include <gtk/gtk.h>
#include <moko-search-bar.h>

#include "appmanager-data.h"


typedef enum
{
  FILTER_INSTALLED,
  FILTER_UPGRADEABLE,
  FILTER_SELECTED
} SearchBarFilter;

GtkWidget* search_bar_new (ApplicationManagerData *appdatam, GtkTreeModel *pkg_list);
void search_bar_add_filter_item (ApplicationManagerData *appdata, gchar *item);
void search_bar_set_active_filter (MokoSearchBar *bar, SearchBarFilter filter);


#endif /* SEARCH_BAR_H */

