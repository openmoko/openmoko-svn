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

#define USB_DBUS_SERVICE      "com.burtonini"
#define USB_DBUS_PATH         "/com/burtonini"
#define USB_DBUS_INTERFACE    "com.burtonini"

static void usb_applet_init_dbus( UsbApplet* applet )
{
    GError* error = NULL;
    DBusGConnection* bus = dbus_g_bus_get( DBUS_BUS_SESSION, &error );

    if (error)
    {
        g_warning( "%s: Error acquiring session bus: %s", G_STRLOC, error->message );
        return;
    }

    DBusGProxy* usb_control_interface = dbus_g_proxy_new_for_name( bus, USB_DBUS_SERVICE, USB_DBUS_PATH, USB_DBUS_INTERFACE );
    if ( !usb_control_interface )
    {
        g_warning( "Could not connect to USB dbus service" );
        return;
    }

    dbus_g_proxy_add_signal( usb_control_interface, "SignalTest", G_TYPE_INVALID );
    dbus_g_proxy_connect_signal( usb_control_interface, "SignalTest", G_CALLBACK(usb_applet_dbus_signal), NULL, NULL );
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
