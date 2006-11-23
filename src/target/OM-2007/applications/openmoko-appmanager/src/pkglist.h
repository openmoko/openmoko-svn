/**
 * @file pkglist.h 
 * @brief Manage the package list and related function
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
 * @date 2006-10-09
 */

#ifndef _FIC_PKGLIST_H
#define _FIC_PKGLIST_H

#include <gtk/gtk.h>

/**
 * Package list column id
 */
enum {
  COL_STATUS = 0,
  COL_NAME,
  COL_SIZE,
  COL_POINTER,
  NUM_COL
}; 

/**
 * @brief The max length of the search string
 */
#define MAX_LENGTH_OF_SEARCH_STRING   30

void init_package_view (void);

gint init_package_list_data (void);

void init_filter_menu (void);

gint get_select_status (gpointer data);

void package_unmark_event (void);

void package_mark_install_event (void);

void package_mark_upgrade_event (void);

void package_mark_remove_event (void);

void update_detail_info (GtkWidget *widget);

gint change_package_list_status (gint id);

gchar *get_section_name (gint id);

gboolean check_marked_list_empty (void);

void fill_store_with_marked_list (GtkTreeStore *store, gint column);

void dispose_marked_package (void);

gboolean get_sensitive_of_item (gint id);

gboolean check_section_id (gint id);

gboolean check_search_string_empty (void);

void search_user_input (void);

void free_all_dynamic_memory (void);

void uninit_libipkg (void);

gint reinit_package_list_data (void);

void install_from_ipk_file (gchar *filename);

void mark_all_upgrade (void);

#endif //_FIC_PKGLIST_H
