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

/* Callbacks from widgets */
#if 0

static void
on_keypad_dial_clicked (MokoKeypad  *keypad,
                        const gchar *number,
                        MokoDialer  *dialer)
{
  GtkWidget *dlg;
  MokoDialerPrivate *priv;
  MokoContactEntry *entry = NULL;
  
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  if (!number) {
    gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), 1);
    moko_history_set_filter (MOKO_HISTORY (priv->history), HISTORY_FILTER_DIALED);
    return;
  }

  /* check current dialer state */
  if (0 || priv->status != DIALER_STATUS_NORMAL)
  {
    gchar *strings[] = {
      "Normal",
      "Incoming Call",
      "Dialing",
      "Outgoing Call"
    };
    dlg = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
        "Cannot dial when dialer is busy.\nCurrent status = %s", strings[priv->status]);
    gtk_dialog_run (GTK_DIALOG (dlg));
    gtk_widget_destroy (dlg);

    g_warning ("Cannot dial when dialer is busy: %d\n", priv->status);

    return;
  }
  priv->status = DIALER_STATUS_DIALING;

  /* check for network connection */
  if (priv->registered != MOKO_GSMD_CONNECTION_NETREG_HOME
      && priv->registered != MOKO_GSMD_CONNECTION_NETREG_ROAMING
      && priv->registered != MOKO_GSMD_CONNECTION_NETREG_DENIED)
  {
    gchar *strings[] = {
      "None",
      "Home network registered",
      "Searching for network",
      "Network registration denied",
      "",
      "Roaming network registered"
    };

    dlg = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
        "Cannot dial number. %s", strings[priv->registered]);
    gtk_dialog_run (GTK_DIALOG (dlg));
    gtk_widget_destroy (dlg);

    /* no point continuing if we're not connected to a network! */
    priv->status = DIALER_STATUS_NORMAL;
    return;
  }

  entry = moko_contacts_lookup (moko_contacts_get_default (), number);

  /* Prepare a voice journal entry */
  if (priv->journal)
  {
    priv->entry = moko_journal_entry_new (VOICE_JOURNAL_ENTRY);
    moko_journal_entry_set_direction (priv->entry, DIRECTION_OUT);
    moko_journal_entry_set_source (priv->entry, "Openmoko Dialer");
    moko_journal_entry_set_gsm_location (priv->entry, &priv->gsm_location);
    moko_journal_voice_info_set_distant_number (priv->entry, number);
    if (entry && entry->contact->uid)
      moko_journal_entry_set_contact_uid (priv->entry, entry->contact->uid);
  }
  moko_talking_outgoing_call (MOKO_TALKING (priv->talking), number, entry);

  gtk_notebook_insert_page (GTK_NOTEBOOK (priv->notebook), priv->talking,
                            gtk_image_new_from_file (PKGDATADIR"/phone.png"),
                            0);
  gtk_container_child_set (GTK_CONTAINER (priv->notebook), priv->talking,
                           "tab-expand", TRUE,
                           NULL);
  
  gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), 0);

  gtk_window_present (GTK_WINDOW (priv->window));

  moko_keypad_set_talking (MOKO_KEYPAD (priv->keypad), TRUE);

  moko_gsmd_connection_voice_dial (priv->connection, number);

  g_signal_emit (G_OBJECT (dialer), dialer_signals[OUTGOING_CALL], 0, number);
}

static void
on_history_dial_number (MokoHistory *history,
                        const gchar *number,
                        MokoDialer  *dialer)
{
  on_keypad_dial_clicked (NULL, number, dialer);
}


#endif
int main (int argc, char **argv)
{

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


   /* application object */
  g_set_application_name ("OpenMoko Dialer");
#if 0
  if (show_missed)
    moko_dialer_show_missed_calls (dialer, NULL);
  else if (show_dialer)
    moko_dialer_show_dialer (dialer, NULL);
#endif
  gtk_main ();

  return 0;
}
