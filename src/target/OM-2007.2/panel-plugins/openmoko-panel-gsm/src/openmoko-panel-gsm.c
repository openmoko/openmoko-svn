/*  openmoko-panel-gsm.c
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *  Copyright (C) 2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author: mickey $]
 */
#include <libmokogsmd2/moko-gsmd-connection.h>
#include <libmokopanelui2/moko-panel-applet.h>

#include <libnotify/notify.h>

#include <gtk/gtkimage.h>
#include <gtk/gtkbox.h>
#include <gtk/gtk.h>
#include <time.h>

/* Just change this is gsmd changes */
#define _MAX_SIGNAL 30.0

typedef struct {
    MokoPanelApplet* mokoapplet;
    gboolean gprs_mode;
    MokoGsmdConnection* gsm;
    int strength;
    int type;
    int lac;
    int cell;
    char operator_name[255];
    GtkMenuItem* information;
} GsmApplet;

static GsmApplet* theApplet = NULL;

static void gsm_applet_show_status(GtkWidget* menu, GsmApplet* applet);

static void
gsm_applet_free(GsmApplet *applet)
{
    g_slice_free( GsmApplet, applet );
}

static void
gsm_applet_update_signal_strength(MokoGsmdConnection* connection,
                                  int strength,
                                  GsmApplet* applet)
{
    gint pixmap = 0;
    gchar *image = NULL;
    applet->strength = strength;

    g_debug( "gsm_applet_update_signal_strength: signal strength = %d",
              strength );

    if ( strength == 99 )
    {
        moko_panel_applet_set_icon( applet->mokoapplet, PKGDATADIR "/SignalStrength_NR.png" );
        return;
    }

    gfloat percent = (strength / _MAX_SIGNAL) * 100;

    if ( percent == 0 )
      pixmap = 0;
    else if ( percent < 20 )
      pixmap = 1;
    else if ( percent < 40 )
      pixmap = 2;
    else if ( percent < 60 )
      pixmap = 3;
    else if ( percent < 80 )
      pixmap = 4;
    else
      pixmap = 5;

    image = g_strdup_printf( "%s/SignalStrength%s%02d.png",
                             PKGDATADIR,
                             applet->gprs_mode ? "25g_" : "_", pixmap );

    moko_panel_applet_set_icon( applet->mokoapplet, image );

    g_free( image );
}

static void gsm_applet_network_current_operator_cb(MokoGsmdConnection *self, const gchar* name)
{
    if ( strcmp( name, theApplet->operator_name ) != 0 )
    {
        strcpy( theApplet->operator_name, name );
        gsm_applet_update_signal_strength( self, 0, theApplet );
        gsm_applet_show_status( 0, theApplet );
    }
}

static void
gsm_applet_network_registration_cb(MokoGsmdConnection *self,
                                  int type,
                                  int lac,
                                  int cell)
{
    g_debug( "gsm_applet_network_registration_cb: updating netreg values" );
    theApplet->type = type;
    theApplet->lac = lac;
    theApplet->cell = cell;

    if ( (type == MOKO_GSMD_CONNECTION_NETREG_HOME) || (type == MOKO_GSMD_CONNECTION_NETREG_ROAMING) )
    {
        g_debug( "gsm_applet_network_registration_cb: NETREG type = %d", type );
        moko_gsmd_connection_trigger_current_operator_event( self );
    }
}

static void
gsm_applet_show_status(GtkWidget* menu, GsmApplet* applet)
{
    const gchar* summary = 0;
    const gchar* details = 0;
    switch ( applet->type )
    {
        case 0:
            summary = g_strdup( "Not searching" );
        break;
        
        case 1:
            summary = g_strdup_printf( "Connected to '%s'", applet->operator_name );
            details = g_strdup_printf( "Type: Home Network\nCell ID: %04x : %04x\nSignal: %i dbM", applet->lac, applet->cell, -113 + applet->strength*2 );
        break;
        
        case 2: summary = g_strdup( "Searching for Service" );
        break;
        
        case 3: summary = g_strdup( "Registration Denied" );
        break;
        
        case 5:
            summary = g_strdup_printf( "Connected to '%s'", applet->operator_name );
            details = g_strdup_printf( "Type: Roaming\nCell ID: %04x : %04x\nSignal: %i dbM", applet->lac, applet->cell, -113 + applet->strength*2 );
        break;
        default: summary = g_strdup( "Unknown" );
    }

    notify_notification_show( notify_notification_new( summary, details, NULL, NULL ), NULL );
}

static void
gsm_applet_power_up_antenna(GtkWidget* menu, GsmApplet* applet)
{
    //TODO notify user
    moko_gsmd_connection_set_antenna_power( applet->gsm, TRUE, NULL );
}

static void
gsm_applet_autoregister_network(GtkWidget* menu, GsmApplet* applet)
{
    moko_gsmd_connection_network_register( applet->gsm );
}

static void
gsm_applet_power_down_antenna(GtkWidget* menu, GsmApplet* applet)
{
    //TODO notify user
    moko_gsmd_connection_set_antenna_power( applet->gsm, FALSE, NULL );
}

static void
gsm_applet_test_operation(GtkWidget* menu, GsmApplet* applet)
{
    moko_gsmd_connection_trigger_current_operator_event( applet->gsm );
}

G_MODULE_EXPORT GtkWidget*
mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    GsmApplet* applet = g_slice_new0(GsmApplet);
    theApplet = applet; // nasty global variable
    strcpy( applet->operator_name, "<unknown>" );
    MokoPanelApplet* mokoapplet = applet->mokoapplet = MOKO_PANEL_APPLET(moko_panel_applet_new());

    notify_init ("GSM Applet");

    moko_panel_applet_set_icon( mokoapplet, PKGDATADIR "/SignalStrength_NR.png" );

    applet->gprs_mode = FALSE;
    gtk_widget_show_all( GTK_WIDGET(mokoapplet) );

    applet->gsm = moko_gsmd_connection_new();
    g_signal_connect( G_OBJECT(applet->gsm), "signal-strength-changed", G_CALLBACK(gsm_applet_update_signal_strength), applet );
    g_signal_connect( G_OBJECT(applet->gsm), "network-registration", G_CALLBACK(gsm_applet_network_registration_cb), applet );
    g_signal_connect( G_OBJECT(applet->gsm), "network-current-operator", G_CALLBACK(gsm_applet_network_current_operator_cb), applet );

    // tap-with-hold menu (NOTE: temporary: left button atm.)
    GtkMenu* menu = GTK_MENU (gtk_menu_new());

    GtkWidget* title = gtk_frame_new( "GSM Network" );
    gtk_frame_set_label_align( GTK_FRAME(title), 0.5, 0.5 );
    gtk_frame_set_shadow_type( GTK_FRAME(title), GTK_SHADOW_IN );
    gtk_widget_set_name( title, "GsmAppletMenu" );

    GtkWidget* titleitem = gtk_menu_item_new();
    g_signal_connect( G_OBJECT(titleitem), "activate", G_CALLBACK(gsm_applet_show_status), applet );
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), titleitem );
    gtk_container_add( GTK_CONTAINER(titleitem), title );

    GtkWidget* item1 = gtk_menu_item_new_with_label( "Power-Up GSM Antenna" );
    g_signal_connect( G_OBJECT(item1), "activate", G_CALLBACK(gsm_applet_power_up_antenna), applet );
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), item1 );
    GtkWidget* item2 = gtk_menu_item_new_with_label( "Autoregister with Network" );
    g_signal_connect( G_OBJECT(item2), "activate", G_CALLBACK(gsm_applet_autoregister_network), applet );
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), item2 );
    GtkWidget* item3 = gtk_menu_item_new_with_label( "Power-Down GSM Antenna" );
    g_signal_connect( G_OBJECT(item3), "activate", G_CALLBACK(gsm_applet_power_down_antenna), applet );
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), item3 );
#if 0
    GtkWidget* item4 = gtk_menu_item_new_with_label( "Trigger Operation (TEST)" );
    g_signal_connect( G_OBJECT(item4), "activate", G_CALLBACK(gsm_applet_test_operation), applet );
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), item4 );
#endif
    gtk_widget_show_all( GTK_WIDGET(menu) );
    moko_panel_applet_set_popup( mokoapplet, GTK_WIDGET (menu), MOKO_PANEL_APPLET_CLICK_POPUP );
    return GTK_WIDGET(mokoapplet);
}
