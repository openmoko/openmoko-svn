/**
 *  @file fingermenu.c
 *  @brief The Main Menu in the Openmoko
 *
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 */
#include <gtk/gtk.h>
#include "fingermenu.h"
#include "callbacks.h"

static void moko_finger_menu_init(MokoFingerMenu *self);

static void moko_finger_menu_class_init(MokoFingerMenuClass *self);

GType
moko_finger_menu_get_type (void) /* Typechecking */
{
    static GType menu_type = 0;

    if (!menu_type)
    {
        static const GTypeInfo menu_info =
        {
            sizeof (MokoFingerMenuClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moko_finger_menu_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoFingerMenu),
            0,
            (GInstanceInitFunc) moko_finger_menu_init,
            NULL
        };
        menu_type = g_type_register_static (MOKO_TYPE_APPLICATION, "MokoFingerMenu", &menu_info, 0);
    }

    return menu_type;
}

void
moko_finger_menu_class_init(MokoFingerMenuClass *self)
{

}

void
moko_finger_menu_init(MokoFingerMenu *self)
{
  self->app = MOKO_APPLICATION(moko_application_get_instance());
  g_set_application_name( "OpenMoko Main Menu" );

  self->window = MOKO_FINGER_WINDOW(moko_finger_window_new());
  gtk_widget_show (GTK_WIDGET (self->window));

  self->wheel = MOKO_FINGER_WHEEL(moko_finger_window_get_wheel (self->window));

  self->toolbox = MOKO_FINGER_TOOL_BOX(moko_finger_window_get_toolbox (self->window));
  self->history = moko_app_history_init (self->toolbox);
  self->mm = MAINMENU (moko_main_menu_new());
  gtk_widget_show (GTK_WIDGET(self->mm));

  g_signal_connect (self->wheel, "press_bottom", G_CALLBACK ( moko_wheel_bottom_press_cb), self);
  g_signal_connect (self->wheel, "press_left_up", G_CALLBACK ( moko_wheel_left_up_press_cb), self);
  g_signal_connect (self->wheel, "press_right_down", G_CALLBACK ( moko_wheel_right_down_press_cb), self);
  g_signal_connect (self->mm->icon_view, "selection-changed", G_CALLBACK (moko_icon_view_selection_changed_cb), self);
  g_signal_connect (self->mm->icon_view, "item_activated", G_CALLBACK (moko_icon_view_item_acitvated_cb), self);

  moko_finger_window_set_contents (self->window, GTK_WIDGET(self->mm));
}

MokoFingerMenu*
moko_finger_menu_new ()
{
  return FINGERMENU(g_object_new(moko_finger_menu_get_type(), NULL));
}

void
moko_finger_menu_show (MokoFingerMenu *self)
{
  if (!self)
	return;

  gtk_widget_show_all (GTK_WIDGET(self->window));
  gtk_widget_show_all (GTK_WIDGET(self->mm));

    //gtk_widget_show (GTK_WIDGET(self->window));
  gtk_widget_show (GTK_WIDGET(self->toolbox));
  gtk_widget_show (GTK_WIDGET(self->wheel));

  gtk_window_present (GTK_WINDOW (self->window));
}

void
moko_finger_menu_hide (MokoFingerMenu *self)
{
  if (!self)
    return;

  if (GTK_WIDGET_VISIBLE (GTK_WIDGET(self->window)))
    gtk_window_iconify (GTK_WINDOW (self->window));
}

void
moko_finger_menu_build (MokoFingerMenu *self, MokoDesktopItem *item)
{
  if (!self)
    return;

  self->mm->top_item = item;
  moko_main_menu_update_content (self->mm, item);
}

void
moko_finger_menu_update_content (MokoFingerMenu *self, MokoDesktopItem *item)
{
  if (!self || !item)
    return;
  moko_main_menu_update_content (self->mm, item);
}

MokoDesktopItem *
moko_finger_menu_get_current_item (MokoFingerMenu *self)
{
  if (!self->mm->current)
    return NULL;
  else
    return self->mm->current;
}

void
moko_finger_menu_set_current_item (MokoFingerMenu *self, MokoDesktopItem *item)
{
  if(!self || !item)
    return;
  self->mm->current = item;
}

void
moko_finger_menu_move_cursor_up(MokoFingerMenu *self)
{
  if (!self)
    return;
  moko_icon_view_move_cursor_line_up (self->mm->icon_view);
}

void
moko_finger_menu_move_cursor_down(MokoFingerMenu *self)
{
  if (!self)
    return;
  moko_icon_view_move_cursor_line_down (self->mm->icon_view);
}

void
moko_finger_menu_set_app_history(MokoFingerMenu *self, GdkPixbuf *pixbuf, MokoDesktopItem *item)
{
  if (!self || !pixbuf || !item)
    return;
  moko_app_history_set (self->history, pixbuf, item);

}
