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
static GtkEntry* entry;

static GtkTextBuffer* buffer;

#include <gsmd/event.h>

static const char *cprog_names[] = {
    [GSMD_CALLPROG_SETUP]           = "SETUP",
    [GSMD_CALLPROG_DISCONNECT]      = "DISCONNECT",
    [GSMD_CALLPROG_ALERT]           = "ALERT",
    [GSMD_CALLPROG_CALL_PROCEED]    = "PROCEED",
    [GSMD_CALLPROG_SYNC]            = "SYNC",
    [GSMD_CALLPROG_PROGRESS]        = "PROGRESS",
    [GSMD_CALLPROG_CONNECTED]       = "CONNECTED",
    [GSMD_CALLPROG_RELEASE]         = "RELEASE",
    [GSMD_CALLPROG_REJECT]          = "REJECT",
    [GSMD_CALLPROG_UNKNOWN]         = "UNKNOWN",
};

static const char *cdir_names[] = {
    [GSMD_CALL_DIR_MO]              = "Outgoing",
    [GSMD_CALL_DIR_MT]              = "Incoming",
    [GSMD_CALL_DIR_CCBS]            = "CCBS",
    [GSMD_CALL_DIR_MO_REDIAL]       = "Outgoing Redial",
};


#define my_debug(fmt,...) \
  gtk_text_buffer_insert_at_cursor( buffer, g_strdup_printf(fmt,##__VA_ARGS__), -1 ); \
  gtk_text_buffer_insert_at_cursor( buffer, "\n", 1 );

void cb_poweron_clicked( GtkButton* button, MokoFingerWindow* window )
{
    my_debug( "setting antenna power to 1" );
    moko_gsmd_connection_set_antenna_power( gsm, TRUE );
}

void cb_register_clicked( GtkButton* button, MokoFingerWindow* window )
{
    my_debug( "calling network register" );
    moko_gsmd_connection_network_register( gsm );
}

void cb_poweroff_clicked( GtkButton* button, MokoFingerWindow* window )
{
    my_debug( "setting antenna power to 0" );
    moko_gsmd_connection_set_antenna_power( gsm, FALSE );
}

void cb_acceptcall_clicked( GtkButton* button, MokoFingerWindow* window )
{
    my_debug( "accepting voice call" );
    moko_gsmd_connection_voice_accept( gsm );
}

void cb_dial_clicked( GtkButton* button, MokoFingerWindow* window )
{
    my_debug( "dialing %s", gtk_entry_get_text( entry ) );
    moko_gsmd_connection_voice_dial( gsm, gtk_entry_get_text( entry ) );
}

void cb_hangup_clicked( GtkButton* button, MokoFingerWindow* window )
{
    my_debug( "hanging up" );
    moko_gsmd_connection_voice_hangup( gsm );
}

void cb_signal_strength_changed( MokoGsmdConnection* connection, int value, gpointer user_data )
{
    my_debug( "signal strength changed: %d", value );
    gtk_label_set_text( signal, g_strdup_printf( "Signal Strength: %d", value ) );
}

void cb_network_registration( MokoGsmdConnection* connection, int type, int lac, int cell )
{
    my_debug( "network registration %d, %d, %d", type, lac, cell );
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

void cb_incoming_call( MokoGsmdConnection* connection, int type )
{
    my_debug( "incoming call type -- type = %d", type );
}

void cb_incoming_call_progress( MokoGsmdConnection* connection, int code )
{
    my_debug( "call progress -- code = %d (%s)", code, cprog_names[code] );
    gtk_label_set_text( status, cprog_names[code] );
}

void cb_incoming_clip( MokoGsmdConnection* connection, char* number )
{
    my_debug( "incoming number -- number = %s", number );
}

int main( int argc, char** argv )
{
    g_debug( "starting up" );
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

    entry = gtk_entry_new();

    GtkTable* table = gtk_table_new( 4, 3, TRUE );

    GtkButton* button1 = gtk_button_new_with_label( "Power-On" );
    g_signal_connect( G_OBJECT(button1), "clicked", G_CALLBACK(cb_poweron_clicked), window );
    gtk_table_attach_defaults( GTK_BOX(table), GTK_WIDGET(button1), 0, 1, 1, 2 );
    GtkButton* button2 = gtk_button_new_with_label( "Register" );
    g_signal_connect( G_OBJECT(button2), "clicked", G_CALLBACK(cb_register_clicked), window );
    gtk_table_attach_defaults( GTK_BOX(table), GTK_WIDGET(button2), 1, 2, 1, 2 );
    GtkButton* button3 = gtk_button_new_with_label( "Power-Off" );
    g_signal_connect( G_OBJECT(button3), "clicked", G_CALLBACK(cb_poweroff_clicked), window );
    gtk_table_attach_defaults( GTK_BOX(table), GTK_WIDGET(button3), 2, 3, 1, 2 );

    GtkButton* button4 = gtk_button_new_with_label( "Accept Call" );
    g_signal_connect( G_OBJECT(button4), "clicked", G_CALLBACK(cb_acceptcall_clicked), window );
    gtk_table_attach_defaults( GTK_BOX(table), GTK_WIDGET(button4), 0, 1, 0, 1 );
    GtkButton* button5 = gtk_button_new_with_label( "Dial" );
    g_signal_connect( G_OBJECT(button5), "clicked", G_CALLBACK(cb_dial_clicked), window );
    gtk_table_attach_defaults( GTK_BOX(table), GTK_WIDGET(button5), 1, 2, 0, 1 );
    GtkButton* button6 = gtk_button_new_with_label( "Hangup" );
    g_signal_connect( G_OBJECT(button6), "clicked", G_CALLBACK(cb_hangup_clicked), window );
    gtk_table_attach_defaults( GTK_BOX(table), GTK_WIDGET(button6), 2, 3, 0, 1 );

    GtkTextView* textview = gtk_text_view_new();
    buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(textview) );

    gtk_box_pack_start( vbox, GTK_WIDGET(label0), FALSE, FALSE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(network), FALSE, FALSE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(signal), FALSE, FALSE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(status), FALSE, FALSE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(entry), FALSE, FALSE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(table), TRUE, TRUE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(textview), TRUE, TRUE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(label2), FALSE, FALSE, 0 );

    moko_finger_window_set_contents( window, GTK_WIDGET(vbox) );

    gsm = moko_gsmd_connection_new();

    g_signal_connect( G_OBJECT(gsm), "signal-strength-changed", G_CALLBACK(cb_signal_strength_changed), NULL );
    g_signal_connect( G_OBJECT(gsm), "network-registration", G_CALLBACK(cb_network_registration), NULL );
    g_signal_connect( G_OBJECT(gsm), "incoming-call", G_CALLBACK(cb_incoming_call), NULL );
    g_signal_connect( G_OBJECT(gsm), "call-progress", G_CALLBACK(cb_incoming_call_progress), NULL );
    g_signal_connect( G_OBJECT(gsm), "incoming-clip", G_CALLBACK(cb_incoming_clip), NULL );

    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(window) );
    g_debug( "entering main loop" );
    gtk_main();
    g_debug( "left main loop" );

    g_object_unref( G_OBJECT(gsm) );

    g_debug( "shutting down" );
    return 0;
}
