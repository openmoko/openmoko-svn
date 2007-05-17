/*
 *  Footer - Task manager application
 *
 *  Authored by Daniel Willmann <daniel@totalueberwachung.de>
 *
 *  Copyright (C) 2007 OpenMoko, Inc.
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
 */

#include "taskmanager.h"
#include "callbacks.h"

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>
#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-pixmap-button.h>
#include <gtk/gtk.h>
#include <glib.h>

void taskmanager_init (MokoTaskManager *tm)
{
  Display *dpy;
  GtkWidget *image;

  dpy = GDK_DISPLAY();

  tm->app = MOKO_APPLICATION(moko_application_get_instance());
  g_set_application_name( "Taskmanager" );


  tm->window = moko_finger_window_new();
  moko_application_set_main_window(tm->app, tm->window);

  gtk_window_set_title(GTK_WINDOW (tm->window), "OpenMoko Tasks");

  tm->wheel = moko_finger_window_get_wheel(tm->window);
  tm->toolbox = moko_finger_window_get_toolbox(tm->window);

  tm->go_to = moko_finger_tool_box_add_button_without_label(tm->toolbox);
  image = gtk_image_new_from_file (PKGDATADIR"/active_task.png");
  moko_pixmap_button_set_finger_toolbox_btn_center_image (tm->go_to, image);

  tm->kill = moko_finger_tool_box_add_button_without_label(tm->toolbox);
  image = gtk_image_new_from_file (PKGDATADIR"/close.png");
  moko_pixmap_button_set_finger_toolbox_btn_center_image (tm->kill, image);

  tm->kill_all = moko_finger_tool_box_add_button_without_label(tm->toolbox);
  image = gtk_image_new_from_file (PKGDATADIR"/close_all.png");
  moko_pixmap_button_set_finger_toolbox_btn_center_image (tm->kill_all, image);

  tm->quit = moko_finger_tool_box_add_button_without_label(tm->toolbox);
  image = gtk_image_new_from_file (PKGDATADIR"/exit.png");
  moko_pixmap_button_set_finger_toolbox_btn_center_image (tm->quit, image);

  tm->table = gtk_table_new( 3, 3, TRUE );

  moko_finger_window_set_contents( tm->window, GTK_WIDGET(tm->table) );
}
