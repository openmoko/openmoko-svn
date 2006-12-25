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

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>
#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-pixmap-button.h>

#include "mainmenu.h"
#include "menu-list.h"
#include "close-page.h"
#include "callbacks.h"
#include "mokoiconview.h" 

gboolean test = FALSE;
 
void
moko_wheel_bottom_press_cb (GtkWidget *self, MokoMainmenuApp *mma)
{
  if (test) {
    gtk_widget_hide (mma->close);
    gtk_widget_show (mma->mm);
  }
  else {
    gtk_widget_hide (mma->mm);
    gtk_widget_show (mma->close);
  }
  
test = !test;

}

void
moko_wheel_left_up_press_cb (GtkWidget *self, MokoMainmenuApp *mma)
{
  gtk_widget_grab_focus (mma->mm->icon_view);
  g_signal_emit_by_name (G_OBJECT(mma->mm->icon_view), "move-cursor", GTK_MOVEMENT_DISPLAY_LINES, -1);

}

void
moko_wheel_right_down_press_cb (GtkWidget *self, MokoMainmenuApp *mma)
{
  gtk_widget_grab_focus (mma->mm->icon_view);
  g_signal_emit_by_name (G_OBJECT(mma->mm->icon_view), "move-cursor", GTK_MOVEMENT_DISPLAY_LINES, 1);
}

void
moko_close_page_close_btn_released_cb (GtkButton *button, MokoMainmenuApp *mma)
{
  if (mma->mm)
    {
  	moko_main_menu_clear (mma->mm);
       gtk_widget_destroy (mma->mm);
    }
  if (mma->close)
    gtk_widget_destroy (mma->close);
  if (mma->wheel)
    gtk_widget_destroy (mma->wheel);
  if (mma->toolbox)
    gtk_widget_destroy (mma->toolbox);
  if (mma->window)
    moko_window_clear (mma->window);

  g_free (mma);

  gtk_main_quit();
}

void 
moko_up_btn_cb (GtkButton *button, MokoMainMenu *mm)
{
  gtk_widget_grab_focus (mm->icon_view);
  g_signal_emit_by_name (G_OBJECT(mm->icon_view), "move-cursor", GTK_MOVEMENT_DISPLAY_LINES, -1);
}

void 
moko_down_btn_cb (GtkButton *button, MokoMainMenu *mm)
{
  gtk_widget_grab_focus (mm->icon_view);
  g_signal_emit_by_name (G_OBJECT(mm->icon_view), "move-cursor", GTK_MOVEMENT_DISPLAY_LINES, 1);
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
			GtkMovementStep arg1, gint arg2, MokoMainmenuApp *mma) {
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
moko_icon_view_selection_changed_cb(GtkIconView *iconview, MokoMainmenuApp *mma) {
  g_debug ("call moko_selection_changed_cb");
  gint total = 0, cursor = 0;
  char item_total[6];

  total = moko_icon_view_get_total_items (mma->mm->icon_view);
  cursor = moko_icon_view_get_cursor_positon (mma->mm->icon_view);

  snprintf (item_total, 6, "%d/%d", cursor, total);
  moko_set_label_content (mma->mm->item_total, item_total);

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
