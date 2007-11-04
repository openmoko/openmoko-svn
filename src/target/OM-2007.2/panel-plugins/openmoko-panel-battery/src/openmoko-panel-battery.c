/*  openmoko-panel-battery.c
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *  Copyright (C) 2007 OpenMoko Inc.
 *
 *  Based on Battery Applet for matchbox-panel-2 by
 *  Jorn Baayen <jorn@openedhand.com>
 *  (C) 2006 OpenedHand Ltd.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author: mickey $]
 */


#include <libmokopanelui2/moko-panel-applet.h>

#include <gtk/gtklabel.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <apm.h>
#include <string.h>
#include <time.h>

#define JUICE_PIXMAPS 6

typedef struct {
    MokoPanelApplet* mokoapplet;
    guint timeout_id;
} BatteryApplet;

static gboolean
timeout (BatteryApplet *applet);

/* applets cannot be unloaded yet */
#if 0
static void
battery_applet_free (BatteryApplet *applet)
{
    g_source_remove (applet->timeout_id);
    g_slice_free (BatteryApplet, applet);
}
#endif

#define CHARGER_DBUS_SERVICE      "org.freedesktop.PowerManagement"
#define CHARGER_DBUS_PATH         "/org/freedesktop/PowerManagement"
#define CHARGER_DBUS_INTERFACE    "org.freedesktop.PowerManagement"

DBusHandlerResult signal_filter (DBusConnection *bus, DBusMessage *msg, void *user_data)
{
    g_debug( "signal_filter" );
    if ( dbus_message_is_signal( msg, CHARGER_DBUS_INTERFACE, "ChargerConnected" ) )
    {
        g_debug( "connected" );
        timeout( user_data );
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if ( dbus_message_is_signal( msg, CHARGER_DBUS_INTERFACE, "ChargerDisconnected" ) )
    {
        g_debug( "disconnected" );
        timeout( user_data );
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    g_debug( "(unknown dbus message, ignoring)" );
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void battery_applet_init_dbus( BatteryApplet* applet )
{
    DBusError error;
    dbus_error_init (&error);

    /* Get a connection to the system bus */
    DBusConnection* bus = dbus_bus_get (DBUS_BUS_SYSTEM, &error);
    if (!bus)
    {
        gchar buffer[100];
        sprintf (buffer, "Failed to connect to the D-BUS daemon: %s", error.message);
        g_critical (buffer);
        dbus_error_free (&error);
        return ;
    }
    g_debug("Connection to bus successfully made");

    dbus_connection_setup_with_g_main (bus, NULL);

    dbus_bus_add_match (bus, "type='signal'", &error);
    dbus_connection_add_filter (bus, signal_filter, applet, NULL);
}


/* Called frequently */
static gboolean
timeout (BatteryApplet *applet)
{
    char* icon;
    static int last_status = -123; /* the status last time we checked */

    apm_info info;
    // How about g_new0 here?
    memset (&info, 0, sizeof (apm_info));
    apm_read (&info);

    /* don't do any update if status is the same as the last time */
    if (last_status == info.battery_status)
    {
        return TRUE;
    }

    //FIXME Can we actually find out, when the battery is full?

    last_status = info.battery_status;

    if ( info.battery_status == BATTERY_STATUS_ABSENT ||
         info.battery_status == BATTERY_STATUS_CHARGING )
    {
         icon = PKGDATADIR "/Battery_AC.png";
    }
    else
    {
        if (info.battery_percentage < 10)
            icon = PKGDATADIR "/Battery_00.png";
        else if (info.battery_percentage < 30)
            icon = PKGDATADIR "/Battery_01.png";
        else if (info.battery_percentage < 50)
            icon = PKGDATADIR "/Battery_02.png";
        else if (info.battery_percentage < 70)
            icon = PKGDATADIR "/Battery_03.png";
        else if (info.battery_percentage < 90)
            icon = PKGDATADIR "/Battery_04.png";
        else
            icon = PKGDATADIR "/Battery_05.png";
    }

    moko_panel_applet_set_icon( applet->mokoapplet, icon );

    return TRUE;
}

G_MODULE_EXPORT GtkWidget* mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    BatteryApplet *applet = g_slice_new (BatteryApplet);
    MokoPanelApplet* mokoapplet = applet->mokoapplet = MOKO_PANEL_APPLET(moko_panel_applet_new());

    time_t t;
    struct tm *local_time;
    t = time( NULL );
    local_time = localtime(&t);

    timeout( applet );
    battery_applet_init_dbus( applet );
    applet->timeout_id = g_timeout_add_seconds( 60, (GSourceFunc) timeout, applet);
    gtk_widget_show_all( GTK_WIDGET(mokoapplet) );
    return GTK_WIDGET(mokoapplet);
}
