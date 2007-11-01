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
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <moko-stock.h>

// #include "moko-dialer.h"
#include "moko-keypad.h"
#include "moko-history.h"

static gboolean show_missed;

static GOptionEntry entries[] = {

  {"show-missed", 'm', 0, G_OPTION_ARG_NONE, &show_missed,
   "Show the history window filtered by the missed, none.", "N"},

  {NULL}
};

/* Callbacks from widgets */

static void
on_keypad_dial_clicked (MokoKeypad  *keypad,
                        const gchar *number,
                        DBusGProxy  *proxy)
{
  GError *error = NULL;

  if (!number)
  {
    /*
    gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), 1);
    moko_history_set_filter (MOKO_HISTORY (priv->history), HISTORY_FILTER_DIALED);
    */
    return;
  }

  g_debug ("Dial %s", number);

  dbus_g_proxy_call (proxy, "Dial", &error, G_TYPE_STRING, number, G_TYPE_INVALID, G_TYPE_INVALID);

  if (error)
  {
    g_warning (error->message);
  }
  else
  {
    /* the dbus object takes over now */
    gtk_main_quit();
  }
}

static void
on_history_dial_number (MokoHistory *history,
                        const gchar *number,
                        DBusGProxy  *proxy)
{
  on_keypad_dial_clicked (NULL, number, proxy);
}

int main (int argc, char **argv)
{
  GtkWidget *window, *notebook, *keypad, *history;
  MokoJournal *journal;

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


  DBusGConnection *connection;
  GError *error;
  DBusGProxy *proxy;

  g_type_init ();

  error = NULL;
  connection = dbus_g_bus_get (DBUS_BUS_SESSION,
                               &error);
  if (connection == NULL)
    {
      g_printerr ("Failed to open connection to bus: %s\n",
                  error->message);
      g_error_free (error);
      exit (1);
    }

  proxy = dbus_g_proxy_new_for_name (connection, "org.openmoko.Dialer", "/org/openmoko/Dialer", "org.openmoko.Dialer");

   /* application object */
  g_set_application_name ("OpenMoko Dialer");

  /* Set up the journal */
  journal = moko_journal_open_default ();
  if (!journal || !moko_journal_load_from_storage (journal))
  {
    g_warning ("Could not load journal");
    journal = NULL;
  }


  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (window), "delete-event",
                    (GCallback) gtk_widget_hide_on_delete, NULL);
  gtk_window_set_title (GTK_WINDOW (window), "Dialer");

  /* Notebook */
  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_BOTTOM);
  gtk_container_add (GTK_CONTAINER (window), notebook);


  /* Keypad */
  keypad = moko_keypad_new ();
  g_signal_connect (keypad, "dial_number", G_CALLBACK (on_keypad_dial_clicked), proxy);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), keypad, gtk_image_new_from_file (PKGDATADIR"/dtmf.png"));
  gtk_container_child_set (GTK_CONTAINER (notebook), keypad, "tab-expand", TRUE, NULL);

  /* History */
  history = moko_history_new (journal);
  g_signal_connect (history, "dial_number", G_CALLBACK (on_history_dial_number), proxy);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), history,
                            gtk_image_new_from_stock (MOKO_STOCK_CALL_HISTORY,
                                                      GTK_ICON_SIZE_BUTTON));
  gtk_container_child_set (GTK_CONTAINER (notebook), history,
                           "tab-expand", TRUE,
                           NULL);

  gtk_widget_show_all (window);
  if (show_missed)
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 1);
  else
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);

  gtk_main ();

  return 0;
}
