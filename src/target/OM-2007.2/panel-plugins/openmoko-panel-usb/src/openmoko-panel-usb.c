/*  openmoko-panel-usb.c
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
 */
#include <libmokopanelui2/moko-panel-applet.h>

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <gtk/gtkimage.h>
#include <time.h>

typedef struct {
    MokoPanelApplet* mpa;
    int dummy;
} UsbApplet;

static void
usb_applet_free (UsbApplet *applet)
{
    g_slice_free (UsbApplet, applet);
}

static void usb_applet_dbus_signal( void* data )
{
    g_debug( "usb_applet_dbus_signal: received signal. data pointer = %p", data );
}

#define USB_DBUS_SERVICE      "org.freedesktop.PowerManagement"
#define USB_DBUS_PATH         "/org/freedesktop/PowerManagement"
#define USB_DBUS_INTERFACE    "org.freedesktop.PowerManagement"

DBusHandlerResult signal_filter (DBusConnection *bus, DBusMessage *msg, void *user_data)
{
    g_debug( "signal_filter" );
    if ( dbus_message_is_signal( msg, USB_DBUS_INTERFACE, "ChargerConnected" ) )
    {
        g_debug( "connected" );
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if ( dbus_message_is_signal( msg, USB_DBUS_INTERFACE, "ChargerDisconnected" ) )
    {
        g_debug( "disconnected" );
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    g_debug( "(unknown dbus message, ignoring" );
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void usb_applet_init_dbus( UsbApplet* applet )
{
    DBusError error;
    dbus_error_init (&error);

    /* Get a connection to the session bus */
    DBusConnection* bus = dbus_bus_get (DBUS_BUS_SESSION, &error);
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
    dbus_connection_add_filter (bus, signal_filter, NULL, NULL);

}

static void usb_applet_update_status( UsbApplet* applet )
{
    moko_panel_applet_set_icon( applet->mpa, PKGDATADIR "/Usb.png" );

}

G_MODULE_EXPORT GtkWidget*
mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    MokoPanelApplet* mokoapplet = moko_panel_applet_new();

    UsbApplet *applet;
    time_t t;
    struct tm *local_time;

    applet = g_slice_new( UsbApplet );
    applet->mpa = mokoapplet;

    usb_applet_init_dbus( applet );
    usb_applet_update_status( applet );

    gtk_widget_show_all( GTK_WIDGET(mokoapplet) );
    return GTK_WIDGET(mokoapplet);
};
