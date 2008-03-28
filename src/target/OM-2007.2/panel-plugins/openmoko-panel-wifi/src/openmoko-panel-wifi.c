/*
 *  Authored by Rob Bradford <rob@o-hand.com>
 *  Copyright (C) 2008 OpenMoko Inc.
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
 */

#include <libmokopanelui2/moko-panel-applet.h>
#include <gtk/gtk.h>

static gboolean
wifi_applet_is_wireless_available ()
{
  /* Not implemented yet */
  return TRUE;
}

static void
wifi_applet_update_visibility (GtkWidget *applet)
{
  if (wifi_applet_is_wireless_available ())
    gtk_widget_show_all (applet);
  else
    gtk_widget_hide_all (applet);
}

G_MODULE_EXPORT GtkWidget * 
mb_panel_applet_create (const gchar *id, GtkOrientation orientation)
{
    GtkWidget *applet;

    applet = moko_panel_applet_new ();

    moko_panel_applet_set_icon (MOKO_PANEL_APPLET (applet),
        PKGDATADIR "/" "Wifi.png");

    wifi_applet_update_visibility (applet);

    return applet;
}
