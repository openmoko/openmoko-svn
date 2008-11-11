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

#include <libnotify/notify.h>

#include <gtk/gtklabel.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <apm.h>
#include <string.h>
#include <time.h>

#define JUICE_PIXMAPS 6

typedef struct {
    MokoPanelApplet* mokoapplet;
    const char *last_icon;
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

static gboolean battery_applet_usb_timeout( BatteryApplet* applet )
{
    timeout( applet );
    return FALSE;
}

DBusHandlerResult signal_filter( DBusConnection *bus, DBusMessage *msg, BatteryApplet* applet )
{
    if ( !g_strcmp0( dbus_message_get_interface(msg), CHARGER_DBUS_INTERFACE ) )
    {
	    g_debug( "battery_applet: signal_filter" );
	    if ( dbus_message_is_signal( msg, CHARGER_DBUS_INTERFACE, "ChargerConnected" ) )
	    {
		g_debug( "charger connected" );
		// NOTE Bus Enumeration and entering Charging Mode takes a while. If we immediately
		// call timeout here, we will most likely not yet have entered charging mode
		g_timeout_add_seconds( 5, (GSourceFunc) battery_applet_usb_timeout, applet );
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	    }
	    else if ( dbus_message_is_signal( msg, CHARGER_DBUS_INTERFACE, "ChargerDisconnected" ) )
	    {
		g_debug( "charger disconnected" );
		timeout( applet );
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	    }

	    g_debug( "(unknown dbus message, ignoring)" );
    }

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

static void battery_applet_notification (BatteryApplet *applet,
				         apm_info *info, int type)
{
    NotifyNotification *n;
    static gint displayed = 0;
    const gchar *summary;
    const gchar *details;
    gint urgency;

    /* battery status information */
    if (type == 1)
    {
	switch (info->battery_status)
	{
	    case BATTERY_STATUS_CHARGING:
		details = g_strdup ("<b>Status:</b> Charging");
		break;
	    case BATTERY_STATUS_ABSENT:
		details = g_strdup ("<b>Status:</b> Missing");
		break;
	    default:
		details = g_strdup_printf ("<b>Status</b>: Discharging\n"
					   "<b>Percentage charge:</b> %d%%",
					   info->battery_percentage);
		break;
	}
	summary = g_strdup ("Battery info");
	urgency = NOTIFY_URGENCY_NORMAL;
    }
    else
    /* battery status notifications */
    {
	if (info->battery_status == BATTERY_STATUS_ABSENT ||
    	    info->battery_status == BATTERY_STATUS_CHARGING)
	{
	    displayed = 0;
	    return;
	}
	/* 
	 display battery critically low notification only if battery
	 level <= 3 and notification wasn't show yet
	*/
	else if ((info->battery_percentage <= 3) && !(displayed & 0x1))
	{
	    summary = g_strdup ("Phone battery critically low");
	    details = g_strdup ("The battery is below the critical level and "
		    		"this phone will <b>power-off</b> when the "
		    		"battery becomes completely empty.");
	    urgency = NOTIFY_URGENCY_CRITICAL;
	    displayed |= 0x1;
	}
	/* 
         display battery critically low notification only if battery
         level <= 10 and notification wasn't show yet
	*/
	else if ((info->battery_percentage <= 10) && !(displayed & 0x2))
	{
	    summary = g_strdup ("Phone battery low");
	    details = g_strdup_printf ("You have approximately <b>%d%%</b> "
				       "of remaining battery life.",
				       info->battery_percentage);
	    urgency = NOTIFY_URGENCY_NORMAL;
	    displayed |= 0x2;
	}
	else
	    return;
    }
    
    /* FIXME: add battery icon to notification popup */
    n = notify_notification_new (summary, details, NULL, GTK_WIDGET(applet->mokoapplet));
    
    notify_notification_set_urgency (n, urgency);

    if (type == 1)
        /* display status information 3 sec */
	notify_notification_set_timeout (n, 3000);
    else
	/* display notification forever */
	notify_notification_set_timeout (n, 0);

    if (!notify_notification_show (n, NULL)) 
        g_debug ("failed to send notification"); 
}

/* Called frequently */
static gboolean timeout( BatteryApplet *applet )
{
    g_debug( "battery_applet: timeout" );
    char* icon;

    apm_info info;
    // How about g_new0 here?
    memset (&info, 0, sizeof (apm_info));
    apm_read (&info);

    //FIXME Can we actually find out, when the battery is full?

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

    if (icon == applet->last_icon)
	return TRUE;

    applet->last_icon = icon;

    battery_applet_notification (applet, &info, 0);
    moko_panel_applet_set_icon( applet->mokoapplet, icon );

    return TRUE;
}

static void
on_clicked_cb (GtkButton *button, BatteryApplet *applet)
{
    apm_info info;

    memset (&info, 0, sizeof (apm_info));
    apm_read (&info);

    battery_applet_notification (applet, &info, 1);
}

G_MODULE_EXPORT GtkWidget* mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    BatteryApplet *applet = g_slice_new (BatteryApplet);
    MokoPanelApplet* mokoapplet = applet->mokoapplet = MOKO_PANEL_APPLET(moko_panel_applet_new());

    time_t t;
    struct tm *local_time;
    t = time( NULL );
    local_time = localtime(&t);

    applet->last_icon = NULL; 

    notify_init ("Battery Applet");

    timeout( applet );
    battery_applet_init_dbus( applet );
    applet->timeout_id = g_timeout_add_seconds( 60, (GSourceFunc) timeout, applet);
    gtk_widget_show_all( GTK_WIDGET(mokoapplet) );
    
    g_signal_connect (mokoapplet, "clicked", G_CALLBACK (on_clicked_cb), applet);

    return GTK_WIDGET(mokoapplet);
}
