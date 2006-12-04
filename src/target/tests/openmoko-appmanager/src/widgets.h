/**
 * @file widgets.h 
 * @brief Save some widgets pointer.
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

#ifndef _FIC_WIDGETS_H
#define _FIC_WIDGETS_H

#include <gtk/gtk.h>


/**
 * The id of main widgets in application manager.
 */
typedef enum fic_widget_id{
  FIC_WIDGET_WINDOW_MAIN_WINDOW = 0,   ///<Main window

  FIC_WIDGET_DIALOG_APPLYING_DIALOG,   ///<The applying window dialog

  FIC_WIDGET_DIALOG_APPLYING_TEXT,   ///<The applying text dialog

  FIC_WIDGET_FIX_PKG_LIST,   ///<The fix widget of package list

  FIC_WIDGET_SCROLLED_WINDOW_PKG_LIST,   ///<The scrolled window widget of package list

  FIC_WIDGET_SCROLLED_WINDOW_DETAILS,   ///<The scrolled window widget of details

  FIC_WIDGET_TREE_VIEW_PKG_LIST,      ///<The tree view widget of package list

  FIC_WIDGET_IMAGE_IMAGE1,    ///<The image widget in detail area

  FIC_WIDGET_TEXT_VIEW_TEXTAPPNAME,         ///<The textview widget in detail area to display application name

  FIC_WIDGET_TEXT_VIEW_DETAIL,         ///<The textview widget in detail area to display detail info

  FIC_WIDGET_BUTTON_FILTER,         ///<The button widget of filter

  FIC_WIDGET_BUTTON_SEARCH,         ///<The button widget of search

  FIC_WIDGET_BUTTON_SEARCHON,         ///<The button widget of searchon

  FIC_WIDGET_BUTTON_APPLY,         ///<The button widget of apply

  FIC_WIDGET_BUTTON_MODE,         ///<The button widget of mode

  FIC_WIDGET_BUTTON_UPGRADE,         ///<The button widget of upgrade

  FIC_WIDGET_FIXED_TOOL_BAR,         ///<The fixed widget of toolbar

  FIC_WIDGET_ENTRY_SEARCH,         ///<The entry widget of search

  FIC_WIDGET_MENU_APPLICATION_MENU,     ///<The application menu widget

  FIC_WIDGET_MENU_FILTER_MENU,      ///<The filter menu widget

  FIC_WIDGET_MENU_SELECT_MENU        ///<The select menu widget
} FIC_WIDEGT_ID;

void save_main_window (GtkWidget *window);

void save_application_menu (GtkWidget *menu);

void save_filter_menu (GtkWidget *menu);

void save_applying_dialog (GtkWidget *dialog);

GtkWidget *get_widget_pointer (FIC_WIDEGT_ID id);

void save_mark_menu (GtkWidget *menu);

#endif // end _FIC_WIDGETS_H
