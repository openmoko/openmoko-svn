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

#include <libmokoui/moko-panel-applet.h>

#include <gtk/gtklabel.h>

#include <apm.h>
#include <time.h>

#define JUICE_PIXMAPS 6

#define DEBUG_THIS_FILE

typedef struct {
    GdkPixbuf* enclosing;
    GdkPixbuf* plug;
    GdkPixbuf* juice[JUICE_PIXMAPS];
    GdkPixbuf* indicator;
    GtkImage *image;
    guint timeout_id;
} BatteryApplet;

static void
battery_applet_free (BatteryApplet *applet)
{
    g_source_remove (applet->timeout_id);
    g_slice_free (BatteryApplet, applet);
}

/* Called every 5 minutes */
static gboolean
timeout (BatteryApplet *applet)
{
    g_debug( "update battery applet" );

    apm_info info;
    // How about g_new0 here?
    memset (&info, 0, sizeof (apm_info));
    apm_read (&info);

    /* compose new pixmap and set label */

    guint w = gdk_pixbuf_get_width( applet->indicator );
    guint h = gdk_pixbuf_get_height( applet->indicator );

    gdk_pixbuf_copy_area( applet->enclosing, 0, 0, w, h, applet->indicator, 0, 0 );

    GdkPixbuf* icon;

    //FIXME Can we actually find out, when the battery is full?

    if ( info.battery_status == BATTERY_STATUS_ABSENT ||
         info.battery_status == AC_LINE_STATUS_ON )
    {
         icon = applet->plug;
    }
    else
    {
        if (info.battery_percentage < 10)
            icon = applet->juice[0];
        else if (info.battery_percentage < 30)
            icon = applet->juice[1];
        else if (info.battery_percentage < 50)
            icon = applet->juice[2];
        else if (info.battery_percentage < 70)
            icon = applet->juice[3];
        else if (info.battery_percentage < 90)
            icon = applet->juice[4];
        else
            icon = applet->juice[5];
    }

    //FIXME Check whether we actually need to update

    gdk_pixbuf_composite( icon, applet->indicator, 0, 0, w, h, 0, 0, 1, 1, GDK_INTERP_NEAREST, 255 );
    gtk_image_set_from_pixbuf( applet->image, applet->indicator );

    return TRUE;
}

G_MODULE_EXPORT GtkWidget* mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    MokoPanelApplet* mokoapplet = moko_panel_applet_new();

    BatteryApplet *applet;
    time_t t;
    struct tm *local_time;

    applet = g_slice_new (BatteryApplet);

    applet->image = GTK_IMAGE(gtk_image_new());
    gtk_widget_set_name( GTK_WIDGET(applet->image), "MatchboxPanelBattery" );
    g_object_weak_ref( G_OBJECT(applet->image), (GWeakNotify) battery_applet_free, applet );

    /* preload pixbufs */
    guint i = 0;
    applet->juice[i++] = gdk_pixbuf_new_from_file( PKGDATADIR "/Battery_00.png", NULL );
    applet->juice[i++] = gdk_pixbuf_new_from_file( PKGDATADIR "/Battery_01.png", NULL );
    applet->juice[i++] = gdk_pixbuf_new_from_file( PKGDATADIR "/Battery_02.png", NULL );
    applet->juice[i++] = gdk_pixbuf_new_from_file( PKGDATADIR "/Battery_03.png", NULL );
    applet->juice[i++] = gdk_pixbuf_new_from_file( PKGDATADIR "/Battery_04.png", NULL );
    applet->juice[i++] = gdk_pixbuf_new_from_file( PKGDATADIR "/Battery_05.png", NULL );
    applet->enclosing = gdk_pixbuf_new_from_file( PKGDATADIR "/Battery.png", NULL );
    applet->plug = gdk_pixbuf_new_from_file( PKGDATADIR "/Battery_Plug.png", NULL );

    applet->indicator = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8,
                                      gdk_pixbuf_get_width( applet->enclosing ),
                                      gdk_pixbuf_get_height( applet->enclosing ) );

    gtk_widget_set_size_request( GTK_WIDGET(applet->image),
                                 gdk_pixbuf_get_width( applet->indicator ),
                                 gdk_pixbuf_get_height( applet->indicator ) );

    t = time( NULL );
    local_time = localtime(&t);
#ifndef DEBUG_THIS_FILE
    applet->timeout_id = g_timeout_add( 60 * 1000 * 5, (GSourceFunc) timeout, applet);
    timeout(applet);
    //FIXME Add source watching for charger insertion event on /dev/input/event1
#else
    applet->timeout_id = g_timeout_add( 1000, (GSourceFunc) timeout, applet);
#endif
    moko_panel_applet_set_widget( MOKO_PANEL_APPLET(mokoapplet), GTK_WIDGET(applet->image) );
    gtk_widget_show_all( GTK_WIDGET(mokoapplet) );
    return GTK_WIDGET(mokoapplet);
};
