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

 #include "callbacks.h"

gboolean test = FALSE;
void
moko_wheel_bottom_press_cb (GtkWidget *self, MokoMainmenuApp *mma)
{
  if (test) {
    //moko_finger_window_set_contents( mma->window, GTK_WIDGET(mma->mm));
    gtk_widget_hide (mma->close);
    gtk_widget_show (mma->mm);
  }
  else {
    //moko_finger_window_set_contents( mma->window, GTK_WIDGET(mma->close));
    gtk_widget_hide (mma->mm);
    gtk_widget_show (mma->close);
  }
  
  g_debug ("test for wheel bottom pressed _________________________");
test = !test;

}

void
moko_wheel_left_up_press_cb (GtkWidget *self, MokoMainmenuApp *mma)
{
  g_debug ("test for wheel left_up pressed _________________________");
}

void
moko_wheel_right_down_press_cb (GtkWidget *self, MokoMainmenuApp *mma)
{
  g_debug ("test for wheel rifht_down pressed _________________________");

}

/*test*/ 
void
moko_item_select_cb(GtkIconView *icon_view, GtkTreePath *path, gpointer data) {
    g_debug ("call moko_item_select_cb");
    }

gboolean 
moko_activate_cursor_item_cb(GtkIconView *iconview, gpointer user_data) {
    g_debug ("call moko_active_cursor_item_cb");
    }

void
moko_item_acitvated_cb(GtkIconView *iconview, GtkTreePath *arg1, gpointer user_data) {
    g_debug ("call moko_item_acitvated_cb");
    }
                                            
//"move-cursor"
gboolean
moko_move_cursor_cb(GtkIconView *iconview, 
			GtkMovementStep arg1, gint arg2, gpointer user_data) {
    g_debug ("call moko_move_cursor_cb");
    }
//"select-all"
void
moko_select_all_cb(GtkIconView *iconview, gpointer user_data) {
    g_debug ("call moko_select_all_cb");
    }
//"select-cursor-item"
void
moko_select_cursor_item_cb(GtkIconView *iconview, gpointer user_data) {
    g_debug ("call moko_select_cursor_item_cb");
    }
//"selection-changed"
void
moko_selection_changed_cb(GtkIconView *iconview, gpointer user_data) {
    g_debug ("call moko_selection_changed_cb");
    }

//"set-scroll-adjustments"
void
moko_set_scroll_adjustments_cb(GtkIconView *iconview, 
					GtkAdjustment *arg1, GtkAdjustment *arg2, gpointer user_data) {
    g_debug ("call moko_set_scroll_adjustments_cb");
    }

//"toggle-cursor-item"
void
moko_toggle_cursor_item_cb(GtkIconView *iconview, gpointer user_data) {
    g_debug ("call moko_toggle_cursor_cb");
    }
//"unselect-all"
void
moko_unselect_all_cb(GtkIconView *iconview, gpointer user_data) {
    g_debug ("moko_unselect_all_cb");
    }
