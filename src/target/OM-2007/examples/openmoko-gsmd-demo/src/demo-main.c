/*
 *  Gsmd-Demo -- OpenMoko Demo Application
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2007 OpenMoko, Inc.
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

#include <libmokogsmd/moko-gsmd-connection.h>

#include <gtk/gtkalignment.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtktable.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkvbox.h>

static GtkVBox* vbox = NULL;
static MokoFingerToolBox* tools = NULL;

static MokoGsmdConnection* gsm = NULL;

static GtkLabel* network;
static GtkLabel* signal;
static GtkLabel* status;

void cb_poweron_clicked( GtkButton* button, MokoFingerWindow* window )
{
    moko_gsmd_connection_set_antenna_power( gsm, TRUE );
}

void cb_register_clicked( GtkButton* button, MokoFingerWindow* window )
{
    moko_gsmd_connection_network_register( gsm );
}

void cb_poweroff_clicked( GtkButton* button, MokoFingerWindow* window )
{
    moko_gsmd_connection_set_antenna_power( gsm, FALSE );
}

void cb_signal_strength_changed( MokoGsmdConnection* connection, int value, gpointer user_data )
{
    g_debug( "openmoko-gsmd-demo: signal strength changed" );
    gtk_label_set_text( signal, g_strdup_printf( "Signal Strength: %d", value ) );
}

void cb_network_registration( MokoGsmdConnection* connection, int type, int lac, int cell )
{
    g_debug( "openmoko-gsmd-demo: network registration" );
    if ( type == 0 )
        gtk_label_set_text( status, "Not Searching..." );
    else
    if ( type == 2 )
        gtk_label_set_text( status, "Searching..." );
    else if ( type == 3 )
        gtk_label_set_text( status, "Denied! :(" );
    else if ( type == 1 )
    {
        gtk_label_set_text( status, "Registered (Home)" );
        gtk_label_set_text( network, g_strdup_printf( "LocationAreaCode: 0x%04X\nCellID: 0x%04X", lac, cell ) );
    }
    else if ( type == 5 )
    {
        gtk_label_set_text( status, "Registered (Roaming)" );
        gtk_label_set_text( network, g_strdup_printf( "LocationAreaCode: 0x%04X\nCellID: 0x%04X", lac, cell ) );
    }
}

int main( int argc, char** argv )
{
    g_debug( "openmoko-gsmd-demo starting up" );
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

    /* application object */
    MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "Gsmd-Demo" );

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

    GtkLabel* label0 = gtk_label_new( "<big>A Vanille Media Production</big>" );
    gtk_label_set_use_markup( label0, TRUE );

    /* contents */
    vbox = gtk_vbox_new( FALSE, 10 );
    network = gtk_label_new( "<unknown>" );
    signal = gtk_label_new( "<signal strength>" );
    status = gtk_label_new( "Idle" );
    GtkLabel* label2 = gtk_label_new( "Press buttons to experiment..." );

    GtkTable* table = gtk_table_new( 4, 3, TRUE );

    GtkButton* button1 = gtk_button_new_with_label( "Power-On" );
    g_signal_connect( G_OBJECT(button1), "clicked", G_CALLBACK(cb_poweron_clicked), window );
    gtk_table_attach_defaults( GTK_BOX(table), GTK_WIDGET(button1), 0, 1, 0, 1 );

    GtkButton* button2 = gtk_button_new_with_label( "Register" );
    g_signal_connect( G_OBJECT(button2), "clicked", G_CALLBACK(cb_register_clicked), window );
    gtk_table_attach_defaults( GTK_BOX(table), GTK_WIDGET(button2), 1, 2, 0, 1 );

    GtkButton* button3 = gtk_button_new_with_label( "Power-Off" );
    g_signal_connect( G_OBJECT(button3), "clicked", G_CALLBACK(cb_poweroff_clicked), window );
    gtk_table_attach_defaults( GTK_BOX(table), GTK_WIDGET(button3), 2, 3, 0, 1 );

    gtk_box_pack_start( vbox, GTK_WIDGET(label0), FALSE, FALSE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(network), FALSE, FALSE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(signal), FALSE, FALSE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(status), FALSE, FALSE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(table), TRUE, TRUE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(label2), FALSE, FALSE, 0 );

    moko_finger_window_set_contents( window, GTK_WIDGET(vbox) );

    gsm = moko_gsmd_connection_new();

    g_signal_connect( G_OBJECT(gsm), "signal-strength-changed", G_CALLBACK(cb_signal_strength_changed), NULL );
    g_signal_connect( G_OBJECT(gsm), "network-registration", G_CALLBACK(cb_network_registration), NULL );

    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(window) );
    g_debug( "openmoko-gsmd-demo entering main loop" );
    gtk_main();
    g_debug( "openmoko-gsmd-demo left main loop" );

    g_object_unref( G_OBJECT(gsm) );

    return 0;
}
