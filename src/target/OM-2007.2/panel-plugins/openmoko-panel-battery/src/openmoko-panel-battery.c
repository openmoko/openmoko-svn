/* vim: set sts=4 sw=4 expandtab: */
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

#include <apm.h>
#include <string.h>
#include <time.h>

#define JUICE_PIXMAPS 6

typedef struct {
    MokoPanelApplet* mokoapplet;
    guint timeout_id;
} BatteryApplet;

/* applets cannot be unloaded yet */
#if 0
static void
battery_applet_free (BatteryApplet *applet)
{
    g_source_remove (applet->timeout_id);
    g_slice_free (BatteryApplet, applet);
}
#endif

/* Called every 5 minutes */
static gboolean
timeout (BatteryApplet *applet)
{
    char* icon;
    static int last_status = -255; /* the status last time we checked */

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

    //FIXME Add source watching for charger insertion event on /dev/input/event1

    /* should use g_timeout_add_seconds() here to save power, but it is only
     * available in glib >= 2.14
     */
    applet->timeout_id = g_timeout_add ( 10 * 1000, (GSourceFunc) timeout, applet);
    timeout(applet);

    moko_panel_applet_set_icon( mokoapplet, PKGDATADIR "/Battery_00.png" );
    gtk_widget_show_all( GTK_WIDGET(mokoapplet) );
    return GTK_WIDGET(mokoapplet);
}
