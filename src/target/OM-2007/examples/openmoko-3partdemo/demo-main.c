/*
 *  3 Part Demo -- OpenMoko Demo Application
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
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

#include <mokoui/moko-application.h>
#include <mokoui/moko-window.h>

#include <gtk/gtkmain.h>
#include <gtk/gtkbutton.h>

int main( int argc, char** argv )
{
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

    MokoApplication* app = MOKO_APPLICATION( moko_application_get_instance() );
    g_set_application_name( "Hello OpenMoko!" );

    MokoWindow* window = MOKO_WINDOW( moko_window_new() );
    // moko_application_set_main_window( window );

    GtkButton* button = gtk_button_new_with_label( "Hello World!" );
    gtk_container_add( GTK_CONTAINER(window), button );

    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK( gtk_main_quit ), NULL );

    gtk_widget_show_all( GTK_WIDGET(window) );
    gtk_main();

    return 0;
}
