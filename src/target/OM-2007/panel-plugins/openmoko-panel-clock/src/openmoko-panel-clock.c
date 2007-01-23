/*  moko-panel-applet.c
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *  Copyright (C) 2007 OpenMoko Inc.
 *
 *  Based on Clock Applet for matchbox-panel-2 by Jorn Baayen <jorn@openedhand.com>
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
} ClockApplet;

static void
clock_applet_free (ClockApplet *applet)
{
        g_source_remove (applet->timeout_id);

        g_slice_free (ClockApplet, applet);
}

/* Called every minute */
static gboolean
timeout (ClockApplet *applet)
{
        time_t t;
        char str[6];

        /* Update label */
        t = time (NULL);
        strftime (str, 6, "%H:%M", localtime (&t));

        gtk_label_set_text (applet->label, str);

        /* Keep going */
        return TRUE;
}

/* Called on the next minute after applet creation */
static gboolean
initial_timeout (ClockApplet *applet)
{
        /* Update label */
        timeout (applet);

        /* Install a new timeout that is called every minute */
        applet->timeout_id = g_timeout_add (60 * 1000,
                                            (GSourceFunc) timeout,
                                            applet);

        /* Don't call this again */
        return FALSE;
}

G_MODULE_EXPORT GtkWidget* mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    MokoPanelApplet* mokoapplet = moko_panel_applet_new();

    ClockApplet *applet;
    time_t t;
    struct tm *local_time;

    applet = g_slice_new (ClockApplet);

    applet->label = GTK_LABEL(gtk_label_new (NULL));
    gtk_widget_set_name( applet->label, "MatchboxPanelClock" );
    g_object_weak_ref( G_OBJECT(applet->label), (GWeakNotify) clock_applet_free, applet );

    t = time( NULL );
    local_time = localtime(&t);
    applet->timeout_id = g_timeout_add( (60 - local_time->tm_sec) * 1000, (GSourceFunc) initial_timeout, applet);
    timeout(applet);

    moko_panel_applet_set_widget( GTK_CONTAINER(mokoapplet), applet->label );
    gtk_widget_show_all( GTK_WIDGET(mokoapplet) );
    return GTK_WIDGET(mokoapplet);
};
