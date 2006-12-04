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
#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>

#include <gtk/gtkalignment.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkvbox.h>

static GtkVBox* vbox = NULL;
static MokoFingerToolBox* tools = NULL;

void cb_orange_button_clicked( GtkButton* button, MokoFingerWindow* window )
{
    g_debug( "openmoko-finger-demo: orange button clicked" );
    static gboolean show = TRUE;

    if ( show )
        gtk_widget_show( GTK_WIDGET(moko_finger_window_get_wheel(window)) );
    else
        gtk_widget_hide( GTK_WIDGET(moko_finger_window_get_wheel(window)) );

    show = !show;
}

void cb_black_button_clicked( GtkButton* button, MokoFingerWindow* window )
{
    g_debug( "openmoko-finger-demo: black button clicked" );
    static gboolean show = TRUE;

    if (!tools)
    {
        tools = moko_finger_window_get_toolbox(window);
        for ( int i = 0; i < 1; ++i )
            moko_finger_tool_box_add_button( tools );
    }

    if ( show )
        gtk_widget_show( GTK_WIDGET(tools) );
    else
        gtk_widget_hide( GTK_WIDGET(tools) );

    show = !show;
}

void cb_dialer_button_clicked( GtkButton* button, MokoFingerWindow* window )
{
    if (!tools) return;
    moko_finger_tool_box_add_button( tools );
}

int main( int argc, char** argv )
{
    g_debug( "openmoko-finger-demo starting up" );
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

    /* application object */
    MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "Finger-Demo" );

    /* main window */
    MokoFingerWindow* window = MOKO_FINGER_WINDOW(moko_finger_window_new());

    /* application menu */
    GtkMenu* appmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* closeitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Close" ));
    g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK(gtk_main_quit), NULL );
    gtk_menu_shell_append( appmenu, closeitem );
    moko_finger_window_set_application_menu( window, appmenu );

    /* connect close event */
    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK(gtk_main_quit), NULL );

    /* contents */
    vbox = gtk_vbox_new( TRUE, 0 );
    GtkLabel* label1 = gtk_label_new( "Populate this area with finger widgets\n \nThere are three types of finger buttons:" );

    GtkLabel* label2 = gtk_label_new( "Orange button toggles finger scrolling wheel\nBlack button toggles finger toolbar\nDialer Button adds a tool button" );

    GtkHBox* hbox = gtk_hbox_new( TRUE, 10 );

    GtkButton* button1 = gtk_button_new();
    g_signal_connect( G_OBJECT(button1), "clicked", G_CALLBACK(cb_orange_button_clicked), window );
    gtk_widget_set_name( GTK_WIDGET(button1), "mokofingerbutton-orange" );
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(button1), TRUE, TRUE, 5 );

    GtkButton* button2 = gtk_button_new();
    //FIXME toggle buttons look odd... needs working on styling
    //gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button2), TRUE);
    g_signal_connect( G_OBJECT(button2), "clicked", G_CALLBACK(cb_dialer_button_clicked), window );
    gtk_widget_set_name( GTK_WIDGET(button2), "mokofingerbutton-dialer" );
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(button2), TRUE, TRUE, 5 );

    GtkButton* button3 = gtk_button_new();
    g_signal_connect( G_OBJECT(button3), "clicked", G_CALLBACK(cb_black_button_clicked), window );
    gtk_widget_set_name( GTK_WIDGET(button3), "mokofingerbutton-black" );
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(button3), TRUE, TRUE, 5 );

    gtk_box_pack_start( vbox, GTK_WIDGET(label1), TRUE, TRUE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(hbox), TRUE, TRUE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(label2), TRUE, TRUE, 0 );

    moko_finger_window_set_contents( window, GTK_WIDGET(vbox) );

    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(window) );
    g_debug( "openmoko-finger-demo entering main loop" );
    gtk_main();
    g_debug( "openmoko-finger-demo left main loop" );

    return 0;
}
