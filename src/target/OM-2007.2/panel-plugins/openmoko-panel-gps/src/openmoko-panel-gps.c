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
    MokoPanelApplet* mokoapplet;
    int state;
    guint timeout_id;
} GpsApplet;

static void
gps_applet_free (GpsApplet *applet)
{
    g_slice_free (GpsApplet, applet);
}

static int 
gps_applet_power_get() 
{
    char buf[64];
    FILE * f = fopen(GPS_PWOERON_FILENAME, "r");
    int ret;
    if (!f) {
	    printf("Open file %s failed!!\n",GPS_PWOERON_FILENAME);
	    return 0;
    }
    ret = fread(buf,sizeof(char),sizeof(buf)/sizeof(char),f);
    fclose(f);
    if (ret > 0 && buf[0]=='1') {
	    return 1;
    }
    return 0;
}

static int 
gps_applet_power_set(int on) 
{
    char buf[64];
    FILE * f = fopen(GPS_PWOERON_FILENAME, "w");
    int ret;
    if (!f) {
	    printf("Open file %s failed!!\n",GPS_PWOERON_FILENAME);
	    return 0;
    }
    sprintf(buf,"%d",on ==0 ? 0 : 1);
    ret = fwrite(buf,sizeof(char),sizeof(buf)/sizeof(char),f);
    fclose(f);
    if (ret <= 0) {
	    printf("Write date into device failed!!\n");
	    return 0;
    }
    return on ==0 ? 0 : 1;
}

static void
mb_panel_update (GpsApplet *applet, int state) 
{
	moko_panel_applet_set_icon( applet->mokoapplet, state == 1 ? PKGDATADIR "/GPS_On.png" : PKGDATADIR "/GPS_Off.png");
	 applet->state = state;
	printf("GPS State is %d\n", applet->state);
}

static void
gps_applet_power_on (GtkWidget* menu, GpsApplet* applet) {
    mb_panel_update(applet,gps_applet_power_set(1));
}
static void
gps_applet_power_off (GtkWidget* menu, GpsApplet* applet) {
    mb_panel_update(applet,gps_applet_power_set(0));
}

static void
gps_applet_update_visibility (GpsApplet *applet)
{
    moko_panel_applet_set_icon(applet->mokoapplet,PKGDATADIR "/GPS_Off.png");
    mb_panel_update(applet, gps_applet_power_get());
    gtk_widget_show_all( GTK_WIDGET(applet->mokoapplet) );
}

static gboolean
gps_applet_timeout_cb (gpointer data)
{
  gps_applet_update_visibility ((GpsApplet *)data);

  return TRUE;
}

G_MODULE_EXPORT GtkWidget*
mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    MokoPanelApplet* mokoapplet = moko_panel_applet_new();

    GpsApplet *applet;
    time_t t;
    struct tm *local_time;

    applet = g_slice_new (GpsApplet);
    applet->mokoapplet = mokoapplet;
    applet->state=-100;

    gps_applet_update_visibility (applet);
    
    GtkMenu* menu = GTK_MENU(gtk_menu_new());
    GtkWidget* item1 = gtk_menu_item_new_with_label("Power-Up GPS");
    GtkWidget* item2 = gtk_menu_item_new_with_label("Power-Off GPS");
    g_signal_connect(G_OBJECT(item1), "activate", G_CALLBACK(gps_applet_power_on), applet);
    g_signal_connect(G_OBJECT(item2), "activate", G_CALLBACK(gps_applet_power_off), applet);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item1);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item2);
    gtk_widget_show_all(GTK_WIDGET(menu));
    moko_panel_applet_set_popup( mokoapplet, GTK_WIDGET(menu), MOKO_PANEL_APPLET_CLICK_POPUP);

    applet->timeout_id = g_timeout_add_seconds (QUERY_FREQ, gps_applet_timeout_cb, 
      applet);
    
    return GTK_WIDGET(mokoapplet);
};
