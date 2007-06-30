/**
 * @file main.c
 * @brief openmoko-taskmanager based on main.c.
 * @author Sun Zhiyong
 * @date 2006-10
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
 */

#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "callbacks.h"
#include "footer.h"

#include <glib/gmain.h>
#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>

#include <X11/Xlib.h>
#include <gdk/gdkx.h>

int 
main( int argc, char **argv )
{
  MokoFooterApp *app;
  GdkWindow *root_window;

  app = g_new0 (MokoFooterApp, 1);

  gtk_init (&argc, &argv);

  root_window = gdk_get_default_root_window ();

  gdk_window_set_events (root_window, gdk_window_get_events (root_window) | GDK_PROPERTY_CHANGE_MASK);
  gdk_window_add_filter (root_window, (GdkFilterFunc)root_window_event_filter_cb, app);

  app->toplevel_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name( app->toplevel_window, "openmoko-footer");
  gtk_window_set_title (GTK_WINDOW (app->toplevel_window), "OpenMoko Task Manager");
  gtk_window_set_type_hint (GTK_WINDOW (app->toplevel_window), GDK_WINDOW_TYPE_HINT_DOCK);
  gtk_window_set_default_size (GTK_WINDOW (app->toplevel_window), FOOTER_PROPERTY_WIDTH, FOOTER_PROPERTY_HEIGHT);
  gtk_widget_set_uposition (GTK_WIDGET (app->toplevel_window), FOOTER_PROPERTY_X, FOOTER_PROPERTY_Y);
  gtk_widget_show (app->toplevel_window);

  app->footer = footer_new (); 
  gtk_widget_show_all (GTK_WIDGET (app->footer));

  app->taskmenu = g_new0 (MokoTaskMenu, 1);
  moko_taskmenu_init (app->taskmenu);

  // FIXME: We want the menu to pop up on button press, but disappear after the second button press...
  g_signal_connect (FOOTER (app->footer)->LeftEventBox, "button_release_event", G_CALLBACK (footer_leftbutton_clicked), app);

  g_signal_connect (FOOTER (app->footer)->RightEventBox, "button_press_event", G_CALLBACK (footer_rightbutton_clicked), app);

  gtk_container_add (GTK_CONTAINER (app->toplevel_window), GTK_WIDGET(app->footer));

  gtk_main ();

  g_free (app);

  return 0;
}
