/*  openmoko-panel-gps.c
 *
 *  Authored by Michael 'Mickey' Lauer <mickey@openmoko.org>
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
#include <libmokopanelui2/moko-panel-applet.h>
#include <gtk/gtkbox.h>
#include <gtk/gtk.h>
#include <gtk/gtkimage.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define GPS_PWOERON_FILENAME "/sys/bus/platform/drivers/neo1973-pm-gps/neo1973-pm-gps.0/pwron"
#define QUERY_FREQ 5

typedef struct {
    MokoPanelApplet *mokoapplet;
    guint timeout_id;
} GpsApplet;

static int gps_applet_power_get()
{
    char buf[64];
    FILE *f = fopen(GPS_PWOERON_FILENAME, "r");
    int ret;
    if (!f) {
	printf("Open file %s failed!!\n", GPS_PWOERON_FILENAME);
	return 0;
    }
    ret = fread(buf, sizeof(char), sizeof(buf) / sizeof(char), f);
    fclose(f);
    if (ret > 0 && buf[0] == '1') {
	return 1;
    }
    return 0;
}

static void gps_applet_update_visibility(GpsApplet * applet)
{
    if (gps_applet_power_get())
	gtk_widget_show(GTK_WIDGET(applet->mokoapplet));
    else
	gtk_widget_hide(GTK_WIDGET(applet->mokoapplet));
}

static gboolean gps_applet_timeout_cb(gpointer data)
{
    gps_applet_update_visibility((GpsApplet *) data);

    return TRUE;
}

static void gps_applet_weak_notify_cb(gpointer data, GObject * dead_object)
{
    GpsApplet *applet = (GpsApplet *) data;

    g_source_remove(applet->timeout_id);
    g_slice_free(GpsApplet, applet);
}

G_MODULE_EXPORT GtkWidget *mb_panel_applet_create(const char *id,
						  GtkOrientation
						  orientation)
{
    MokoPanelApplet *mokoapplet =
	MOKO_PANEL_APPLET(moko_panel_applet_new());

    GpsApplet *applet;

    applet = g_slice_new(GpsApplet);
    applet->mokoapplet = mokoapplet;

    /* 
     * Weak reference so we can find out when the applet is destroyed to get
     * rid of the timeout and tidy up
     */
    g_object_weak_ref((GObject *) applet->mokoapplet,
		      gps_applet_weak_notify_cb, applet);

    moko_panel_applet_set_icon(applet->mokoapplet,
			       PKGDATADIR "/GPS_Off.png");
    gtk_widget_show_all(GTK_WIDGET(applet->mokoapplet));

    applet->timeout_id =
	g_timeout_add_seconds(QUERY_FREQ, gps_applet_timeout_cb, applet);

    return GTK_WIDGET(mokoapplet);
}
