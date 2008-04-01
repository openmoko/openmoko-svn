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

#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>		/* gethostbyname, getnetbyname */
#include <net/ethernet.h>	/* struct ether_addr */
#include <sys/time.h>		/* struct timeval */
#include <unistd.h>

#include <linux/if.h>
#include <linux/wireless.h>

#include <libmokopanelui2/moko-panel-applet.h>
#include <gtk/gtk.h>

#define IFACE "eth0"
#define QUERY_FREQ 5 

typedef struct 
{
  GtkWidget *applet;
  int sock; /* socket for doing query on */
  gchar *iface;
  guint timeout_id;
} WifiAppletData;

static gboolean
wifi_applet_is_wireless_available (WifiAppletData *applet)
{
  struct iwreq wrq;

  /* Clear our request and set the interface name */
  memset (&wrq, 0, sizeof (struct iwreq));
  strncpy ((char *)&wrq.ifr_name, applet->iface, IFNAMSIZ);

  /* Feel the power, uhh, do the ioctl() */
  if (ioctl (applet->sock, SIOCGIWTXPOW, &wrq) != 0)
  {
    g_warning ("Error performing ioctl: %s", g_strerror (errno));
    return FALSE;
  }

  return !wrq.u.txpower.disabled;
}

static void
wifi_applet_update_visibility (WifiAppletData *applet)
{
  if (wifi_applet_is_wireless_available (applet))
    gtk_widget_show_all (applet->applet);
  else
    gtk_widget_hide_all (applet->applet);
}

/* Tidy up callback. Don't want stray timeouts. */
static void
wifi_applet_weak_notify_cb (gpointer data, GObject *dead_object)
{
  WifiAppletData *applet = (WifiAppletData *)data;

  g_source_remove (applet->timeout_id);
  close (applet->sock);
  g_free (applet);
}

static gboolean
wifi_applet_timeout_cb (gpointer data)
{
  wifi_applet_update_visibility ((WifiAppletData *)data);

  return TRUE;
}

G_MODULE_EXPORT GtkWidget * 
mb_panel_applet_create (const gchar *id, GtkOrientation orientation)
{
  WifiAppletData *applet;

  applet = g_new0 (WifiAppletData, 1);

  /* Open socket to perform ioctl() on */
  applet->sock = socket (AF_INET, SOCK_DGRAM, 0);

  if (!applet->sock)
  {
    g_warning ("Unable to open socket: %s", g_strerror (errno));
    return FALSE;
  }

  /* Our interface name */
  applet->iface = IFACE;

  applet->applet = moko_panel_applet_new ();

  /* 
   * Weak reference so we can find out when the applet is destroyed to get
   * rid of the timeout and tidy up
   */
  g_object_weak_ref ((GObject *)applet->applet, wifi_applet_weak_notify_cb, applet);

  moko_panel_applet_set_icon (MOKO_PANEL_APPLET (applet->applet),
      PKGDATADIR "/" "Wifi.png");

  wifi_applet_update_visibility (applet);

  applet->timeout_id = g_timeout_add_seconds (QUERY_FREQ, wifi_applet_timeout_cb, 
      applet);

  return applet->applet;
}
