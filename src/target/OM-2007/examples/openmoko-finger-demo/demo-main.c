/*
 *  Finger-Demo -- OpenMoko Demo Application
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

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-pixmap-container.h>

#include <gtk/gtkactiongroup.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkfixed.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenutoolbutton.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktoolbutton.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkentry.h>

#include <stdlib.h>

int main( int argc, char** argv )
{
    g_debug( "openmoko-finger-demo starting up" );
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

    /* application object */
    MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "OpenMoko-Finger-Demo" );

    /* main window */
    //MokoFingerWindow* window = MOKO_FINGER_WINDOW(moko_finger_window_new());
    GtkWindow* window = gtk_window_new( GTK_WINDOW_TOPLEVEL );

    /* application menu */
    GtkMenu* appmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* closeitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Close" ));
    g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK(gtk_main_quit), NULL );
    gtk_menu_shell_append( appmenu, closeitem );
    //moko_finger_window_set_application_menu( window, appmenu );

    GtkVBox* vbox = gtk_vbox_new( TRUE, 10 );

    gtk_container_add( GTK_CONTAINER(window), vbox );
    /* connect close event */
    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK( gtk_main_quit ), NULL );

#if 0

    g_print( "gdkwindow for vbox = %p", GTK_WIDGET(vbox)->window );
    gtk_widget_set_name( GTK_WIDGET(vbox), "mywidget" );


    /* navigation area */

    GtkNotebook* nb = gtk_notebook_new();
    gtk_notebook_append_page( nb, button1, NULL );
    gtk_notebook_append_page( nb, button2, NULL );

    gtk_box_pack_start( vbox, GTK_WIDGET(nb), TRUE, TRUE, 20 );
#endif

    GtkEntry* entry = gtk_entry_new();
    gtk_entry_set_text( entry, "This is a line of text" );

    MokoPixmapContainer* fixed = moko_pixmap_container_new();
    moko_pixmap_container_set_cargo( fixed, GTK_WIDGET(entry) );

    gtk_box_pack_start( vbox, GTK_WIDGET(fixed), TRUE, TRUE, 10 );

    //moko_finger_window_set_contents( window, GTK_WIDGET(nb) );

    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(window) );
    g_debug( "openmoko-finger-demo entering main loop" );
    gtk_main();
    g_debug( "openmoko-finger-demo left main loop" );

    return 0;
}
