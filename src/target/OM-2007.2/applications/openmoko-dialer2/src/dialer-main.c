/*  
 *  Copyright (C) 2007 Openmoko, Inc.
 *
 *  Authored by OpenedHand Ltd <info@openedhand.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <moko-stock.h>

#include "moko-dialer.h"

#define DIALER_NAMESPACE "org.openmoko.Dialer"
#define DIALER_OBJECT "/org/openmoko/Dialer"

static gboolean show_dialer;
static gboolean show_missed;
static gchar *number = NULL;

static GOptionEntry entries[] = {
  {"show-dialer", 's', 0, G_OPTION_ARG_NONE, &show_dialer,
   "Show the dialer at startup", "N"},

  {"show-missed", 'm', 0, G_OPTION_ARG_NONE, &show_missed,
   "Show the history window filtered by the missed, none.", "N"},
  
  {"dial", 'd', 0, G_OPTION_ARG_STRING, &number,
   "Dial the specified number.", "N"},

  {NULL}
};


static void
_show_dialer (DBusGConnection *conn)
{
  DBusGProxy *proxy = NULL;
  GError *error = NULL;

  proxy = dbus_g_proxy_new_for_name (conn,
                                      DIALER_NAMESPACE,
                                      DIALER_OBJECT,
                                      DIALER_NAMESPACE);

  if (!proxy)
    return;
  
  dbus_g_proxy_call (proxy, "ShowDialer", &error,
                     G_TYPE_INVALID, G_TYPE_INVALID);
  if (error)
    g_warning (error->message);

}

static void
_show_missed (DBusGConnection *conn)
{
  DBusGProxy *proxy = NULL;
  GError *error = NULL;

  proxy = dbus_g_proxy_new_for_name (conn,
                                      DIALER_NAMESPACE,
                                      DIALER_OBJECT,
                                      DIALER_NAMESPACE);

  if (!proxy)
    return;
  
  dbus_g_proxy_call (proxy, "ShowMissedCalls", &error,
                     G_TYPE_INVALID, G_TYPE_INVALID);
  if (error)
    g_warning (error->message);

}

static void
_dial_number (DBusGConnection *conn)
{
  DBusGProxy *proxy = NULL;
  GError *error = NULL;

  proxy = dbus_g_proxy_new_for_name (conn,
                                      DIALER_NAMESPACE,
                                      DIALER_OBJECT,
                                      DIALER_NAMESPACE);

  if (!proxy)
    return;
  
  dbus_g_proxy_call (proxy, "Dial", &error,
                     G_TYPE_STRING, number,
                     G_TYPE_INVALID, G_TYPE_INVALID);
  if (error)
    g_warning (error->message);

}
int
main (int argc, char **argv)
{
  MokoDialer *dialer;
  DBusGConnection *connection;
  DBusGProxy *proxy;
  GError *error = NULL;
  guint32 ret;
  /*gchar *out = NULL, *err = NULL;*/

  if (argc != 1)
  {
    /* Add init code. */
    GError *error = NULL;
    GOptionContext *context = g_option_context_new ("");

    g_option_context_add_main_entries (context, entries, NULL);
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    g_option_context_parse (context, &argc, &argv, &error);

    g_option_context_free (context);
  }

  /* Initialize Threading & GTK+ */
  gtk_init (&argc, &argv);
  gst_init (&argc, &argv);
  moko_stock_register ();

  /* Try and setup our DBus service */
  connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (connection == NULL)
  {
    g_warning ("Failed to make a connection to the session bus: %s", 
               error->message);
    g_error_free (error);
    return 1;
  }
  proxy = dbus_g_proxy_new_for_name (connection, 
                                     DBUS_SERVICE_DBUS,
                                     DBUS_PATH_DBUS, 
                                     DBUS_INTERFACE_DBUS);
  if (!org_freedesktop_DBus_request_name (proxy,
                                          DIALER_NAMESPACE,
                                          0, &ret, &error))
  {
    /* Error requesting the name */
    g_warning ("There was an error requesting the name: %s\n",error->message);
    g_error_free (error);
    
    gdk_init(&argc, &argv);
    gdk_notify_startup_complete ();

    return 1;
  }
  if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
  {
    /* Someone else hase registere dthe object */
    g_warning ("Another instance is running\n");

    if (show_missed)
      _show_missed (connection);
    else if (number)
      _dial_number (connection);
    else
      _show_dialer (connection);
    
    dbus_g_connection_unref (connection);
    

    gdk_init(&argc, &argv);
    gdk_notify_startup_complete ();
    return 0;
  }

  /* So we are creating a new dialer, one of the first things we sould do is
   * resart gsmd, as we cannot guarentee it is running. This also solves the
   * problem of when it hangs.
   * FIXME: This shouldn't be left up to the dialer, and we cannot guarentee
   * to always have root access, but it'll work for most embedded devices.
   */
  /*
  g_debug ("(re)starting gsmd\n");
  g_spawn_command_line_sync ("/etc/init.d/gsmd stop",
                             &out, &err, NULL, NULL);
  g_spawn_command_line_sync ("/etc/init.d/gsmd start",
                             &out, &err, NULL, NULL);
  */
  /* Create the MokoDialer object */
  dialer = moko_dialer_get_default ();

  /* Add the object onto the bus */
  dbus_g_connection_register_g_object (connection, 
                                       DIALER_OBJECT,
                                       G_OBJECT (dialer));

  /* application object */
  g_set_application_name ("OpenMoko Dialer");
 
  if (number)
    moko_dialer_dial (dialer, number, NULL);
  else if (show_missed)
    moko_dialer_show_missed_calls (dialer, NULL);
  else if (show_dialer)
    moko_dialer_show_dialer (dialer, NULL);

  gtk_main ();
  
  return 0;
}
