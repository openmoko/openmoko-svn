/*  openmoko-dialer.c
 *
 *  Authored by Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
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
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */
#include <gtk/gtk.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "error.h"
#include "errno.h"
#include "moko-dialer.h"
#include "dialer-main.h"
#include "dialer-window-history.h"

#define DIALER_NAMESPACE "org.openmoko.Dialer"
#define DIALER_OBJECT "/org/openmoko/Dialer"

static MokoDialerData *p_dialer_data = 0;

MokoDialerData *
moko_get_app_data ()
{
  return p_dialer_data;
}


static DBusGConnection *
dbus_setup(gboolean *p_already_running)
{
  DBusGConnection *connection;
  DBusGProxy *proxy;
  GError *error = NULL;
  guint32 ret;

  if (p_already_running)
    *p_already_running = FALSE;

  connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (connection == NULL)
  {
    g_warning ("Failed to make a connection to the session bus: %s", 
               error->message);
    g_error_free (error);
    return NULL;
  }

  proxy = dbus_g_proxy_new_for_name (connection, 
                                     DBUS_SERVICE_DBUS,
                                     DBUS_PATH_DBUS, 
                                     DBUS_INTERFACE_DBUS);
  if (proxy == NULL)
  {
    g_warning ("Error getting a DBus Proxy\n");
    dbus_g_connection_unref (connection);
    return NULL;
  }

  if (!org_freedesktop_DBus_request_name (proxy,
                                          DIALER_NAMESPACE,
                                          0, &ret, &error))
  {
    /* Error requesting the name */
    g_warning ("There was an error requesting the name: %s\n",error->message);
    g_error_free (error);

    dbus_g_connection_unref (connection);
    
    return NULL;
  }
  if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
  {
    if (p_already_running)
      *p_already_running = TRUE;
  }

  return connection;
}


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

static gboolean show_dialer;
static gboolean show_missed;

static GOptionEntry entries[] = {
  {"show-dialer", 's', 0, G_OPTION_ARG_NONE, &show_dialer,
   "Show the dialer at startup", "N"},
  {"show-missed", 'm', 0, G_OPTION_ARG_NONE, &show_missed,
   "Show the history window filtered by the missed, none.", "N"},
  {NULL}
};

int
main (int argc, char **argv)
{
  MokoDialer *dialer;
  DBusGConnection *connection;
  gboolean already_running;

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
  moko_stock_register ();

  /* Try and setup our DBus service */
  connection = dbus_setup(&already_running);
  if (connection == NULL)
  {
    /*
     * If no dbus, we can't get a remote signal to show
     * the dialer, so just show it immediately
     */
    show_dialer = TRUE;
  }

  if (already_running)
  {
    /* Someone else hase registere dthe object */
    g_warning ("Another instance is running\n");

    if (show_missed)
      _show_missed (connection);
    else
      _show_dialer (connection);
    
    dbus_g_connection_unref (connection);
    

    gdk_init(&argc, &argv);
    gdk_notify_startup_complete ();
    return 1;
  }
  
  /* Create the MokoDialer object */
  dialer = moko_dialer_get_default ();

  p_dialer_data = moko_dialer_get_data (dialer);

  /* Add the object onto the bus */
  dbus_g_connection_register_g_object (connection, 
                                       DIALER_OBJECT,
                                       G_OBJECT (dialer));

  /* application object */
  g_set_application_name ("OpenMoko Dialer");
 
  if (show_dialer)
    moko_dialer_show_dialer (dialer, NULL);
  else if (show_missed)
    moko_dialer_show_missed_calls (dialer, NULL);

  gtk_main ();
  
  return 0;
}
