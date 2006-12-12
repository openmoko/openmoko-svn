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

int main( int argc, char **argv )
{
    OMTaskManager* app;
    DBusError error;

    GError* err = NULL;
    GdkPixbuf *pixbuf;
    GtkStyle *style;    
    GdkPixmap *pixmap;
    GdkBitmap *bitmap;
    
    dbus_error_init(&error);

    if (!(app = g_malloc ( sizeof (OMTaskManager)))){
    		fprintf (stderr,"Openmoko-taskmanager: footer UI initialized failed, app space malloc failed!");
    		exit (-1);
    	}

    app->bus = dbus_bus_get (DBUS_BUS_SESSION, &error);
    if (!app->bus) {
        g_warning ("Failed to connect to the D-BUS daemon: %s", error.message);
        dbus_error_free(&error);
        return 1;
    }
	app->loop = g_main_loop_new( NULL, FALSE );
    
    gtk_init (&argc, &argv);

///initialize TOP LEVEL WINDOW 
    app->toplevel_win = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_widget_set_name (app->toplevel_win, "bg_footer");
    gtk_window_set_title (app->toplevel_win, "OpenMoko Task Manager");
    gtk_window_set_type_hint (GTK_WINDOW(app->toplevel_win), GDK_WINDOW_TYPE_HINT_DOCK);
    gtk_window_set_default_size (app->toplevel_win, FOOTER_PROPERTY_WIDTH, FOOTER_PROPERTY_HEIGHT);
    gtk_widget_set_uposition (app->toplevel_win, FOOTER_PROPERTY_X, FOOTER_PROPERTY_Y);
    gtk_widget_show (app->toplevel_win);

//modify toplevel_win background
    pixbuf = gdk_pixbuf_new_from_file ( PKGDATADIR "/bg_footer.png", &err );
    gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, &bitmap, NULL);
    style = gtk_rc_get_style (app->toplevel_win);
    style->bg_pixmap[GTK_STATE_NORMAL] = pixmap;
    gtk_style_set_background (style, app->toplevel_win->window, GTK_STATE_NORMAL);

///initialize OpenMoko Footer Widget
    app->footer = FOOTER(footer_new()); 
    gtk_widget_show_all (app->footer);
    g_signal_connect ( G_OBJECT (app->footer->LeftEventBox), "button_press_event",
    					G_CALLBACK (footer_leftbutton_clicked), app);
    g_signal_connect ( G_OBJECT (app->footer->RightEventBox), "button_press_event",
    					G_CALLBACK (footer_rightbutton_clicked), app);
   
///Add OpenMoko Footer to Top Level windonw
    gtk_container_add( GTK_CONTAINER(app->toplevel_win), GTK_WIDGET(app->footer) );
    // this violates the privacy concept, but it's a demo for now...
  
    dbus_connection_setup_with_g_main (app->bus, NULL);
    dbus_bus_add_match (app->bus, "type='signal',interface='org.openmoko.dbus.TaskManager'", &error );
    dbus_connection_add_filter (app->bus, signal_filter, app, NULL );

    gtk_widget_show_all (app->toplevel_win);

    g_main_loop_run ( app->loop );
            
    if (pixbuf)
    	  g_free (pixbuf);
    if (pixmap)
    	  g_free (pixmap);
    if (bitmap)
    	  g_free (bitmap);
    g_free (app );
    
    return 0;
}
