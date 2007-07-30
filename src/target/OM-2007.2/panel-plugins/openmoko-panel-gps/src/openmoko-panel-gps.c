/*  openmoko-panel-gps.c
 *
 *  Authored by
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

#include <gtk/gtkimage.h>
#include <time.h>

typedef struct {
    GtkImage *image;
} GpsApplet;

static void
gps_applet_free (GpsApplet *applet)
{
    g_slice_free (GpsApplet, applet);
}

G_MODULE_EXPORT GtkWidget* 
mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    MokoPanelApplet* mokoapplet = moko_panel_applet_new();

    GpsApplet *applet;
    time_t t;
    struct tm *local_time;

    applet = g_slice_new (GpsApplet);

    applet->image = GTK_IMAGE(gtk_image_new_from_file ( PKGDATADIR "/GPS.png"));
    gtk_widget_set_name( applet->image, "openmoko-gps-applet" );
    g_object_weak_ref( G_OBJECT(applet->image), (GWeakNotify) gps_applet_free, applet );

    moko_panel_applet_set_widget( GTK_CONTAINER(mokoapplet), applet->image );
    gtk_widget_show_all( GTK_WIDGET(mokoapplet) );
    return GTK_WIDGET(mokoapplet);
};
