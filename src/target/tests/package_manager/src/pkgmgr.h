/**
 * @file packmgr.h - Manager the package
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
 * @date 2006-8-11
 **/

#ifndef _FIC_PACK_VIEW_H
#define _FIC_PACK_VIEW_H

enum {
  PKG_STATUS_INSTALLED = 1,
  PKG_STATUS_AVAILABLE,
  PKG_STATUS_UPDATES,
  PKG_STATUS_SELECTED,
  PKG_STATUS_NUMS
};

enum {
  PKG_VIEW_DISPLAY_ALL = 1,
  PKG_VIEW_DISPLAY_SEARCH
};

#define MAX_SEARCH_ENTRY_LENGTH    30

gint init_package_view (GtkWidget *window);

gchar * get_first_selected_package_name (GtkWidget *window);

const gchar * get_user_input_string (GtkWidget *window);

gint display_search_package_list (GtkWidget *window, char *name);

gint get_first_selected_package_status (GtkWidget *window);

void update_button_action_status (GtkWidget *window, gint status);

gint get_button_action_status (void);

void update_button_package_status (GtkWidget *window, gint status);

void update_button_all_status (GtkWidget *window, gint status);

gint change_package_list (GtkWidget *window, gint status);

gint get_package_view_status (void);

gint roll_package_view_status (GtkWidget *window);

void clear_search_string (void);

void search_package_from_package_list (GtkWidget *window, gchar *name);

void add_remove_package (GtkWidget *window);

#endif
