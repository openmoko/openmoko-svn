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
#include <unistd.h>

#include <gtk/gtk.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "moko-keypad.h"
#include "moko-history.h"

typedef struct
{
  GtkWidget *notebook;
  GtkWidget *history;
  
  GtkWidget *main_window;

  DBusGProxy *dialer_proxy;
} DialerData;

static gboolean show_missed;

static GOptionEntry entries[] = {
  {"show-missed", 'm', 0, G_OPTION_ARG_NONE, &show_missed,
   "Show the history window filtered by the missed, none.", "N"},
  {NULL}
};

/* Callbacks from widgets */

static void
dial_clicked_cb (GtkWidget *widget, const gchar *number, DialerData *data)
{
  GError *error = NULL;

  if (!number)
  {
    gtk_notebook_set_current_page (GTK_NOTEBOOK (data->notebook), 1);
    moko_history_set_filter (MOKO_HISTORY (data->history), HISTORY_FILTER_DIALED);
    return;
  }

  g_debug ("Dial %s", number);

  dbus_g_proxy_call (data->dialer_proxy, "Dial", &error, G_TYPE_STRING, number, G_TYPE_INVALID, G_TYPE_INVALID);

  if (error)
  {
    GtkWidget *dlg;
    dlg = gtk_message_dialog_new (GTK_WINDOW (data->main_window), GTK_DIALOG_MODAL,
                                  GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                  "Dialer Error:\n%s", error->message);
    g_warning (error->message);
    gtk_dialog_run (GTK_DIALOG (dlg));
    gtk_widget_destroy (dlg);
  }
  else
  {
    /* the dbus object takes over now */
    gtk_main_quit();
  }
}

static void
program_log (const char *format, ...)
{
  va_list args;
  char *formatted, *str;

  if (!getenv ("OM_PROFILING"))
    return;

  va_start (args, format);
  formatted = g_strdup_vprintf (format, args);
  va_end (args);

  str = g_strdup_printf ("MARK: %s: %s", g_get_prgname(), formatted);
  g_free (formatted);

  access (str, F_OK);
  g_free (str);
}

int main (int argc, char **argv)
{
  GtkWidget *window, *keypad;
  MokoJournal *journal;
  DBusGConnection *connection;
  GError *error = NULL;
  DialerData *data;

  program_log ("start dialer");

  data = g_new0 (DialerData, 1);

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
  program_log ("gtk_init");
  gtk_init (&argc, &argv);

  /* application object */
  g_set_application_name ("OpenMoko Dialer");

  program_log ("open connection to dbus");
  connection = dbus_g_bus_get (DBUS_BUS_SESSION,
                               &error);
  if (connection == NULL)
  {
    GtkWidget *dlg;

    dlg = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
        "Failed to open connection to bus: %s", error->message);
    gtk_dialog_run (GTK_DIALOG (dlg));
    gtk_widget_destroy (dlg);

    g_error_free (error);
    exit (1);
  }

  program_log ("get PhoneKit dbus proxy object");
  data->dialer_proxy = dbus_g_proxy_new_for_name (connection,
      "org.openmoko.PhoneKit",
      "/org/openmoko/PhoneKit/Dialer", "org.openmoko.PhoneKit.Dialer");

  /* Set up the journal */
  program_log ("load journal");
  journal = moko_journal_open_default ();
  if (!journal || !moko_journal_load_from_storage (journal))
  {
    g_warning ("Could not load journal");
    journal = NULL;
  }

  program_log ("create main window");
  data->main_window = window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (window, "openmoko-dialer-window");
  g_signal_connect (window, "delete-event", G_CALLBACK (gtk_main_quit), NULL);
  gtk_window_set_title (GTK_WINDOW (window), "Dialer");

  /* Notebook */
  data->notebook = gtk_notebook_new ();
  gtk_widget_set_name (data->notebook, "openmoko-dialer-notebook");
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (data->notebook), GTK_POS_BOTTOM);
  gtk_container_add (GTK_CONTAINER (window), data->notebook);


  /* Keypad */
  keypad = moko_keypad_new ();
  g_signal_connect (keypad, "dial_number", G_CALLBACK (dial_clicked_cb), data);

  gtk_notebook_append_page (GTK_NOTEBOOK (data->notebook), keypad, gtk_image_new_from_file (PKGDATADIR"/dtmf.png"));
  gtk_container_child_set (GTK_CONTAINER (data->notebook), keypad, "tab-expand", TRUE, NULL);

  /* History */
  program_log ("create history widget");
  data->history = moko_history_new (journal);
  g_signal_connect (data->history, "dial_number", G_CALLBACK (dial_clicked_cb), data);
  gtk_notebook_append_page (GTK_NOTEBOOK (data->notebook), data->history,
                            gtk_image_new_from_icon_name ("moko-call-history",
                                                      GTK_ICON_SIZE_BUTTON));
  gtk_container_child_set (GTK_CONTAINER (data->notebook), data->history,
                           "tab-expand", TRUE,
                           NULL);

  program_log ("show window");
  gtk_widget_show_all (window);

  if (show_missed)
    gtk_notebook_set_current_page (GTK_NOTEBOOK (data->notebook), 1);
  else
    gtk_notebook_set_current_page (GTK_NOTEBOOK (data->notebook), 0);

  program_log ("enter main loop");
  gtk_main ();

  g_free (data);
  return 0;
}
