/**
 *  @file dbus-conn.c
 *  @brief dbus connection and message send for openmoko mainmenu
 *  
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *  
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 */
#include "dbus-conn.h"

static DBusConnection *bus;
static DBusError error;

gboolean
moko_dbus_connect_init (void)
{
    /* Get a connection to the session bus */
    dbus_error_init (&error);
    bus = dbus_bus_get (DBUS_BUS_SESSION, &error);

    if (!bus)
    {
        g_warning ("Failed to connect to the D-BUS daemon: %s", error.message);
        return FALSE;
    }
  
    if (dbus_error_is_set (&error))
    {
        fprintf(stdout, "Connection Error (%s)\n", error.message);
        dbus_error_free (&error);
    }

    return TRUE;
}

gboolean
moko_dbus_send_message (const char *str)
{
  DBusMessage *message;
  //char *str = "openmoko taskmanager";

  /* Create a new signal on the "org.openmoko.dbus.TaskManager" interface,
   * from the object "/org/openmoko/footer". */
  message = dbus_message_new_signal ("/org/openmoko/footer",
                                     "org.openmoko.dbus.TaskManager", "push_statusbar_message");
  /* Append the string "openmoko taskmanager" to the signal */
  dbus_message_append_args (message,
  			    DBUS_TYPE_STRING, &str,
                            DBUS_TYPE_INVALID);
  /* Send the signal */
  dbus_connection_send (bus, message, NULL);

  /* Free the signal now we have finished with it */
  dbus_message_unref (message);
  
  /* Tell the user we send a signal */
  g_print("send signal\n");
  
  /* Return TRUE to tell the event loop we want to be called again */
  return TRUE;
}
