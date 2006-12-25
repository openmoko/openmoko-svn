/*
 *  openmoko-mainmenu
 *
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
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
 */

 #ifndef _MAIN_MENU_CALLBACKS_H
 #define _MAIN_MENU_CALLBACKS_H
 #include <gtk/gtk.h>
 #include "main.h"

void
moko_wheel_bottom_press_cb (GtkWidget *self, MokoMainmenuApp *mma);

void
moko_wheel_left_up_press_cb (GtkWidget *self, MokoMainmenuApp *mma);

void
moko_wheel_right_down_press_cb (GtkWidget *self, MokoMainmenuApp *mma);

void
moko_close_page_close_btn_released_cb (GtkButton *button, MokoMainmenuApp *mma);

void 
moko_up_btn_cb (GtkButton *button, MokoMainMenu *mm);

void 
moko_down_btn_cb (GtkButton *button, MokoMainMenu *mm);


void
moko_item_select_cb(GtkIconView *icon_view, GtkTreePath *path, gpointer data);

gboolean /*will be call when Enter key pressed*/
moko_activate_cursor_item_cb(GtkIconView *iconview, gpointer user_data);

void /*double click callback*/
moko_item_acitvated_cb(GtkIconView *iconview, GtkTreePath *arg1, gpointer user_data);
                                            
//"move-cursor"
gboolean
moko_move_cursor_cb(GtkIconView *iconview, GtkMovementStep arg1, gint arg2, MokoMainmenuApp *mma);

//"select-all"
void
moko_select_all_cb(GtkIconView *iconview, gpointer user_data);

//"select-cursor-item"
void
moko_select_cursor_item_cb(GtkIconView *iconview, gpointer user_data);

//"selection-changed"
void
moko_icon_view_selection_changed_cb(GtkIconView *iconview, MokoMainmenuApp *mma);

//"set-scroll-adjustments"
void
moko_set_scroll_adjustments_cb(GtkIconView *iconview, GtkAdjustment *arg1, GtkAdjustment *arg2, gpointer user_data);

//"toggle-cursor-item"
void
moko_toggle_cursor_item_cb(GtkIconView *iconview, gpointer user_data);

//"unselect-all"
void
moko_unselect_all_cb(GtkIconView *iconview, gpointer user_data);

 
 #endif /*callbacks.h*/


