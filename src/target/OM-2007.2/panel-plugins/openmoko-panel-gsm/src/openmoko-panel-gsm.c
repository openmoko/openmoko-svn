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
#include <libmokogsmd/moko-gsmd-connection.h>
#include <libmokopanelui2/moko-panel-applet.h>

#include <gtk/gtkimage.h>
#include <gtk/gtkbox.h>
#include <gtk/gtk.h>
#include <time.h>

typedef struct {
    GtkImage* image;
    gboolean gprs_mode;
    MokoGsmdConnection* gsm;
} GsmApplet;

static void gsm_applet_free(GsmApplet *applet)
{
    g_slice_free( GsmApplet, applet );
}

static void gsm_applet_update_signal_strength(MokoGsmdConnection* connection, int strength, GsmApplet* applet)
{
    g_debug( "gsm_applet_update_signal_strength: signal strength = %d", strength );
    //TODO calibrate
    int pixmap = 0;
    if ( strength > 30 )
        pixmap = 5;
    else if ( strength > 20 )
        pixmap = 4;
    else if ( strength > 17 )
        pixmap = 3;
    else if ( strength > 13 )
        pixmap = 2;
    else if ( strength > 10 )
        pixmap = 1;

    const char* imagestring = g_strdup_printf( "%s/SignalStrength%s%02d.png", PKGDATADIR, applet->gprs_mode ? "25g_" : "_", pixmap );
    gtk_image_set_from_file( GTK_IMAGE(applet->image), imagestring );
}

static void gsm_applet_power_up_antenna(GtkWidget* menu, GsmApplet* applet)
{
    //TODO notify user
    moko_gsmd_connection_set_antenna_power( applet->gsm, TRUE );
}

static void gsm_applet_autoregister_network(GtkWidget* menu, GsmApplet* applet)
{
    moko_gsmd_connection_network_register( applet->gsm );
}

static void gsm_applet_power_down_antenna(GtkWidget* menu, GsmApplet* applet)
{
    //TODO notify user
    moko_gsmd_connection_set_antenna_power( applet->gsm, FALSE );
}

G_MODULE_EXPORT GtkWidget* mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    MokoPanelApplet* mokoapplet = MOKO_PANEL_APPLET(moko_panel_applet_new());

    GsmApplet* applet;
    applet = g_slice_new(GsmApplet);
    applet->image = GTK_IMAGE(gtk_image_new_from_file( PKGDATADIR "/SignalStrength_NR.png" ) );
    applet->gprs_mode = FALSE;
    gtk_widget_set_name( GTK_WIDGET(applet->image), "openmoko-gsm-applet" );
    g_object_weak_ref( G_OBJECT(applet->image), (GWeakNotify) gsm_applet_free, applet );
    moko_panel_applet_set_widget( mokoapplet, GTK_WIDGET(applet->image) );
    gtk_widget_show_all( GTK_WIDGET(mokoapplet) );

    applet->gsm = moko_gsmd_connection_new();
    g_signal_connect( G_OBJECT(applet->gsm), "signal-strength-changed", G_CALLBACK(gsm_applet_update_signal_strength), applet );

    // tap-with-hold menu (NOTE: temporary: left button atm.)
    GtkMenu* menu = gtk_menu_new();
    GtkWidget* item1 = gtk_menu_item_new_with_label( "Power-Up GSM Antenna" );
    g_signal_connect( G_OBJECT(item1), "activate", G_CALLBACK(gsm_applet_power_up_antenna), applet );
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), item1 );
    GtkWidget* item2 = gtk_menu_item_new_with_label( "Autoregister with Network" );
    g_signal_connect( G_OBJECT(item1), "activate", G_CALLBACK(gsm_applet_autoregister_network), applet );
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), item2 );
    GtkWidget* item3 = gtk_menu_item_new_with_label( "Power-Down GSM Antenna" );
    g_signal_connect( G_OBJECT(item1), "activate", G_CALLBACK(gsm_applet_power_down_antenna), applet );
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), item3 );
    gtk_widget_show_all( GTK_WIDGET(menu) );

    moko_panel_applet_set_popup( mokoapplet, menu, MOKO_PANEL_APPLET_CLICK_POPUP );
    return GTK_WIDGET(mokoapplet);
};
