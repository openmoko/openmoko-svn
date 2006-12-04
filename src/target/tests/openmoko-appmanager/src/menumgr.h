/**
 * @file menumgr.h 
 * @brief Manage the application menu and the filter menu.
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
 * @date 2006-10-12
 */

#ifndef _FIC_MENUMGR_H
#define _FIC_MENUMGR_H

#include <gtk/gtk.h>

/**
 * The static element of filter menu.
 */
typedef enum filter_menu_element {
  FILTER_MENU_SEARCH_RESULT,    ///<The search result element
  FILTER_MENU_INSTALLED,        ///<The installed element
  FILTER_MENU_UPGRADEABLE,      ///<The upgradeable element
  FILTER_MENU_SELECTED,         ///<The selected element
  FILTER_MENU_NUM               ///<The number of static element of filter menu
} FILTER_MENU_ELEMENT;

/**
 * The element of select menu.
 */
typedef enum select_menu_ELEMENT {
  SELECT_MENU_UNMARK = 0,       ///<The unmark element
  SELECT_MENU_MARK_INSTALL,     ///<The install element
  SELECT_MENU_MARK_UPGRADE,     ///<The upgrade element
  SELECT_MENU_MARK_REMOVE,      ///<The remove element
  NU_SELECT_MENU                ///<The number of select menu
} SELECT_MENU_ELEMENT;

GtkWidget* create_filter_menu (void);

void on_search_result_activate                 (GtkMenuItem     *menuitem,
                                                gpointer         user_data);

void on_installed_activate                 (GtkMenuItem     *menuitem,
                                            gpointer         user_data);

void on_upgradeable_activate                 (GtkMenuItem     *menuitem,
                                              gpointer         user_data);

void on_selected_activate                 (GtkMenuItem     *menuitem,
                                           gpointer         user_data);

void on_category_activate                 (GtkMenuItem     *menuitem,
                                           gpointer         user_data);

void destroy_filter_menu (void);

void popup_select_menu (GtkWidget *treeview, 
                        GdkEventButton *event,
                        gpointer pkg);

void change_filter_menu_id (gint id);

void get_filter_menu_id (gint *cid, gint *lid);

void set_sensitive_filter_menu (GtkWidget *menu);

void reinit_id_of_filter_menu (void);

#endif  //_FIC_MENUMGR_H
