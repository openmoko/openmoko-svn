/*  openmoko-panel-bt.c
 *
 *  Authored by Michael Lauer <mickey@openmoko.org>
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
#include <libnotify/notify.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkbox.h>
#include <gtk/gtk.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define BT_POWERON_FILENAME "/sys/bus/platform/devices/neo1973-pm-bt.0/power_on"
#define BT_RESET_FILENAME "/sys/bus/platform/devices/neo1973-pm-bt.0/reset"
#define QUERY_FREQ 5 

typedef struct {
  MokoPanelApplet *mokoapplet;
  int state;
  guint timeout_id;
} BtApplet;

static int
read_bt_power(void)
{
  FILE * f = fopen(BT_POWERON_FILENAME, "r+");
  int val;

  if (f == NULL) return -1;

  fscanf(f, "%i", &val);
  fclose(f);
  return val;
}

static int
set_bt_power(int val)
{
  FILE * f = fopen(BT_POWERON_FILENAME, "w");

  if (f == NULL) return -1;

  fprintf(f, "%i\n", val);

  fclose(f);

  return val;
}

static int
reset_bt_power(int val)
{
  FILE * f = fopen(BT_RESET_FILENAME, "w");

  if (f == NULL) return -1;

  fprintf(f, "%i\n", val);

  fclose(f);

  return val;
}

static void
mb_panel_update(BtApplet *applet, int state)
{
    if ( applet->state != state )
    {
        moko_panel_applet_set_icon( applet->mokoapplet, state == 1 ? PKGDATADIR "/Bluetooth_On.png" : PKGDATADIR "/Bluetooth_Off.png");
        applet->state = state;
    }
}

static void
bt_applet_power_on(GtkWidget* menu, BtApplet* applet)
{
    int ret;
    NotifyNotification * nn;

    ret = set_bt_power(1);
    ret = reset_bt_power(0);
    mb_panel_update(applet, 1);
    nn = notify_notification_new ("Bluetooth turned on", NULL, NULL, NULL);
    notify_notification_show (nn, NULL);
}

static void
bt_applet_power_off(GtkWidget* menu, BtApplet* applet)
{
    int ret;
    NotifyNotification * nn;

    ret = set_bt_power(0);
    mb_panel_update(applet, 0);
    nn = notify_notification_new ("Bluetooth turned off", NULL, NULL, NULL);
    notify_notification_show (nn, NULL);
}

static void
bt_applet_status(GtkWidget* menu, BtApplet* applet)
{
    int ret;
    char tmp_string[256];
    NotifyNotification * nn;

    ret = read_bt_power();

    sprintf(tmp_string, "  Bluetooth is %s  \n\n", ret ? "on" : "off");

    nn = notify_notification_new (tmp_string, NULL, NULL, NULL);
    notify_notification_show (nn, NULL);
}

static void
bt_applet_free (BtApplet *applet)
{
    g_slice_free (BtApplet, applet);
}

static void
bt_applet_update_visibility (BtApplet *applet)
{
    mb_panel_update( applet, read_bt_power() );
    gtk_widget_show_all( GTK_WIDGET(applet->mokoapplet) );
}

static gboolean
bt_applet_timeout_cb (gpointer data)
{
  bt_applet_update_visibility ((BtApplet *)data);

  return TRUE;
}

G_MODULE_EXPORT GtkWidget*
mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    BtApplet *applet = g_slice_new (BtApplet);
    MokoPanelApplet* mokoapplet = applet->mokoapplet = moko_panel_applet_new();

    applet->state = 42;
    bt_applet_update_visibility (applet);
    
    GtkMenu* menu = GTK_MENU(gtk_menu_new());
    GtkWidget* item1 = gtk_menu_item_new_with_label("Power-Up Bluetooth radio");
    g_signal_connect(G_OBJECT(item1), "activate", G_CALLBACK(bt_applet_power_on), applet);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item1);
    GtkWidget* item2 = gtk_menu_item_new_with_label("Power-Off Bluetooth radio");
    g_signal_connect(G_OBJECT(item2), "activate", G_CALLBACK(bt_applet_power_off), applet);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item2);
    GtkWidget* item3 = gtk_menu_item_new_with_label("Bluetooth status");
    g_signal_connect(G_OBJECT(item3), "activate", G_CALLBACK(bt_applet_status), applet);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item3);

    gtk_widget_show_all(GTK_WIDGET(menu));
    moko_panel_applet_set_popup( mokoapplet, GTK_WIDGET(menu), MOKO_PANEL_APPLET_CLICK_POPUP);

    applet->timeout_id = g_timeout_add_seconds (QUERY_FREQ, bt_applet_timeout_cb, 
      applet);
      
    return GTK_WIDGET(mokoapplet);
}
