/*  openmoko-panel-battery.c
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *  Copyright (C) 2007 OpenMoko Inc.
 *
 *  Based on Battery Applet for matchbox-panel-2 by Jorn Baayen <jorn@openedhand.com>
 *  (C) 2006 OpenedHand Ltd.
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

#include <libmokoui/moko-panel-applet.h>

#include <gtk/gtklabel.h>
#include <time.h>

typedef struct {
        GtkLabel *label;
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
        // draw...
        /* Keep going */
        return TRUE;
}

G_MODULE_EXPORT GtkWidget* mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    MokoPanelApplet* mokoapplet = moko_panel_applet_new();

    BatteryApplet *applet;
    time_t t;
    struct tm *local_time;

    applet = g_slice_new (BatteryApplet);

    applet->label = GTK_LABEL(gtk_label_new (NULL));
    gtk_widget_set_name( applet->label, "MatchboxPanelBattery" );
    g_object_weak_ref( G_OBJECT(applet->label), (GWeakNotify) battery_applet_free, applet );

    t = time( NULL );
    local_time = localtime(&t);
    applet->timeout_id = g_timeout_add( 60 * 1000 * 5, (GSourceFunc) timeout, applet);
    timeout(applet);

    moko_panel_applet_set_widget( GTK_CONTAINER(mokoapplet), applet->label );
    gtk_widget_show_all( GTK_WIDGET(mokoapplet) );
    return GTK_WIDGET(mokoapplet);
};
