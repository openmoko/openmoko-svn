/*
 *  moko-dialer; a GObject wrapper for the dialer.
 *
 *  Authored by OpenedHand Ltd <info@openedhand.com>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include <gtk/gtk.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <moko-gsmd-connection.h>
#include <moko-journal.h>
#include <moko-stock.h>

#include "moko-dialer.h"

#include "moko-contacts.h"
#include "moko-keypad.h"
#include "moko-talking.h"
#include "moko-history.h"

G_DEFINE_TYPE (MokoDialer, moko_dialer, G_TYPE_OBJECT)

#define MOKO_DIALER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_DIALER, MokoDialerPrivate))

#define GSM_REGISTER_TIMEOUT 5000 /* Five seconds after powering up */

struct _MokoDialerPrivate
{
  gint                status;

  /* Main Widgets */
  GtkWidget          *window;
  GtkWidget          *notebook;
  
  /* Pages of the notebook */
  GtkWidget          *talking;
  GtkWidget          *keypad;
  GtkWidget          *history;

  /* Special objects */
  MokoGsmdConnection *connection;
  MokoJournal        *journal;
  MokoContacts       *contacts;

  /* Registration variables */
  gboolean            reg_request;
  gboolean            registered;
};

enum
{
  INCOMING_CALL,
  OUTGOING_CALL,
  TALKING,
  HUNG_UP,
  REJECTED,
  
  LAST_SIGNAL
};

static guint dialer_signals[LAST_SIGNAL] = {0, };

static void  on_keypad_dial_clicked (MokoKeypad  *keypad,
                                     const gchar *number,
                                     MokoDialer  *dialer);


/* DBus functions */
gboolean
moko_dialer_show_dialer (MokoDialer *dialer, GError *error)
{
  MokoDialerPrivate *priv;
  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);
  priv = dialer->priv;
 
  gtk_widget_show (priv->window);
  gtk_window_present (GTK_WINDOW (priv->window));
  return TRUE;
}


gboolean
moko_dialer_show_missed_calls (MokoDialer *dialer, GError *error)
{
  MokoDialerPrivate *priv;

  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);
  priv = dialer->priv;
  
  /* Filter history on missed calls */
  
  moko_history_set_filter (MOKO_HISTORY (priv->history), HISTORY_FILTER_MISSED);

  gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), -1);
  
  gtk_widget_show (priv->window);
  gtk_window_present (GTK_WINDOW (priv->window));

  return TRUE;
}

static gboolean
moko_dialer_get_status (MokoDialer *dialer, gint *OUT_status, GError *error)
{
  MokoDialerPrivate *priv;
  
  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);
  priv = dialer->priv;

  *OUT_status = priv->status;

  return TRUE;
}

static gboolean
moko_dialer_dial (MokoDialer *dialer, const gchar *number, GError *error)
{
  MokoDialerPrivate *priv;

  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);
  g_return_val_if_fail (number != NULL, FALSE);
  g_return_val_if_fail (moko_dialer_show_dialer (dialer, NULL), FALSE);
  priv = dialer->priv;

  on_keypad_dial_clicked (NULL, number, dialer);
  return TRUE;
}

static gboolean
moko_dialer_hang_up (MokoDialer *dialer, const gchar *message, GError *error)
{
  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);
  
  /* FIXME: Create a dialog and let the user know that another program is
   * requesting the connection be dropped, and why ($message).
   */
  return TRUE;
}

/* </dbus functions> */
 
void
moko_dialer_outgoing_call (MokoDialer *dialer, const gchar *number)
{
  ;
}


void
moko_dialer_talking (MokoDialer *dialer)
{
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  dialer->priv->status = DIALER_STATUS_TALKING;

  g_signal_emit (G_OBJECT (dialer), dialer_signals[TALKING], 0);
}

void
moko_dialer_hung_up (MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv; 
  
  priv->status = DIALER_STATUS_NORMAL;

  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook)) == 3)
    gtk_notebook_remove_page (GTK_NOTEBOOK (priv->notebook), 0);
  
  moko_gsmd_connection_voice_hangup (priv->connection);   
  g_signal_emit (G_OBJECT (dialer), dialer_signals[HUNG_UP], 0);
  
}

void
moko_dialer_rejected (MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;
  
  priv->status = DIALER_STATUS_NORMAL;

  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook)) == 3)
    gtk_notebook_remove_page (GTK_NOTEBOOK (priv->notebook), 0);
  
  moko_gsmd_connection_voice_hangup (priv->connection);  
  
  g_signal_emit (G_OBJECT (dialer), dialer_signals[REJECTED], 0);
}

/* Callbacks from widgets */
static void
on_keypad_dial_clicked (MokoKeypad  *keypad,
                        const gchar *number,
                        MokoDialer  *dialer)
{
  MokoDialerPrivate *priv;
  MokoContactEntry *entry = NULL;
  
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  if (0 || priv->status != DIALER_STATUS_NORMAL)
  {
    g_warning ("Cannot dial when dialer is busy: %d\n", priv->status);
    return;
  }
  priv->status = DIALER_STATUS_DIALING;

  entry = moko_contacts_lookup (moko_contacts_get_default (), number);
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
on_talking_accept_call (MokoTalking *talking, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  if (priv->status != DIALER_STATUS_INCOMING)
    return;
  
  priv->status = DIALER_STATUS_TALKING;

  moko_talking_accepted_call (MOKO_TALKING (priv->talking), NULL, NULL);
  moko_gsmd_connection_voice_accept (priv->connection);

  g_signal_emit (G_OBJECT (dialer), dialer_signals[TALKING], 0);
}

static void
on_talking_reject_call (MokoTalking *talking, MokoDialer *dialer)
{

  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  priv->status = DIALER_STATUS_NORMAL;

  gtk_notebook_remove_page (GTK_NOTEBOOK (priv->notebook), 0);
  moko_gsmd_connection_voice_hangup (priv->connection);

  g_signal_emit (G_OBJECT (dialer), dialer_signals[REJECTED], 0);
  moko_keypad_set_talking (MOKO_KEYPAD (priv->keypad), FALSE);
}

static void
on_talking_cancel_call (MokoTalking *talking, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  priv->status = DIALER_STATUS_NORMAL;

  gtk_notebook_remove_page (GTK_NOTEBOOK (priv->notebook), 0);
  moko_gsmd_connection_voice_hangup (priv->connection);

  g_signal_emit (G_OBJECT (dialer), dialer_signals[HUNG_UP], 0);
  moko_keypad_set_talking (MOKO_KEYPAD (priv->keypad), FALSE);
}

static void
on_talking_speaker_toggle (MokoTalking *talking, 
                           gboolean     speaker_phone,
                           MokoDialer  *dialer)
{
  /* Toggle speaker phone */
  g_print ("Speaker toggled\n");
}

static void
on_keypad_digit_pressed (MokoKeypad *keypad,
                         const gchar digit,
                         MokoDialer *dialer)
{
  /* If in call, dtmf it, otherwise ignore */
}

static void
on_history_dial_number (MokoHistory *history,
                        const gchar *number,
                        MokoDialer  *dialer)
{
  on_keypad_dial_clicked (NULL, number, dialer);
}

/* Callbacks for MokoGsmdConnection */
static void
on_network_registered (MokoGsmdConnection *conn, 
                       int type, 
                       int lac,  
                       int cell,
                       MokoDialer *dialer)
{
  MokoDialerPrivate *priv;
  
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  priv->registered = TRUE;
}

static void
on_incoming_call (MokoGsmdConnection *conn, int type, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;
  
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  /* We sometimes get the signals multiple times */
  if (priv->status == DIALER_STATUS_INCOMING)
    return;

  priv->status = DIALER_STATUS_INCOMING;

  moko_talking_incoming_call (MOKO_TALKING (priv->talking), NULL, NULL);

  gtk_notebook_insert_page (GTK_NOTEBOOK (priv->notebook), priv->talking,
                            gtk_image_new_from_file (PKGDATADIR"/phone.png"),
                            0);
  gtk_container_child_set (GTK_CONTAINER (priv->notebook), priv->talking,
                           "tab-expand", TRUE,
                           NULL); 
  
  gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), 0);

  gtk_window_present (GTK_WINDOW (priv->window));

  g_signal_emit (G_OBJECT (dialer), dialer_signals[INCOMING_CALL], 0, NULL);
}

static void
on_incoming_clip (MokoGsmdConnection *conn,
                  const gchar        *number,
                  MokoDialer         *dialer)
{
  g_print ("Number = %s\n", number);
}

static void
on_pin_requested (MokoGsmdConnection *conn, int type, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  g_print ("Incoming pin request for type %d", type);
}


static void
on_call_progress_changed (MokoGsmdConnection *conn, 
                          int type, 
                          MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;
  g_print ("mokogsmd call progress changed");
  switch (type) 
  {
    case MOKO_GSMD_PROG_DISCONNECT:
    case MOKO_GSMD_PROG_RELEASE:
      moko_dialer_hung_up (dialer);
      moko_keypad_set_talking (MOKO_KEYPAD (priv->keypad), FALSE);
      g_print ("mokogsmd disconnect");
      break;
    
    case MOKO_GSMD_PROG_REJECT:
      moko_dialer_rejected (dialer);
      moko_keypad_set_talking (MOKO_KEYPAD (priv->keypad), FALSE);
      g_print ("mokogsmd reject");
      break;
    
    case MOKO_GSMD_PROG_CONNECTED:
      moko_keypad_set_talking (MOKO_KEYPAD (priv->keypad), TRUE);
      g_print ("mokogsmd connected");
      break;
    case MOKO_GSMD_PROG_SETUP:
      g_print ("mokogsmd setup");
      break;
    case MOKO_GSMD_PROG_ALERT:
      g_print ("mokogsmd alert");
      break;
    case  MOKO_GSMD_PROG_CALL_PROCEED:
      g_print ("mokogsmd proceed");
      break;
    case MOKO_GSMD_PROG_SYNC:
      g_print ("mokogsmd sync");
      break;
    case  MOKO_GSMD_PROG_PROGRESS:
      g_print ("mokogsmd progress");
      break;
    case MOKO_GSMD_PROG_UNKNOWN:
    default:
      g_print ("mokogsmd unknown");
      break;
  }
}
static gboolean
register_network_cb (MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_val_if_fail (MOKO_DIALER (dialer), TRUE);
  priv = MOKO_DIALER_GET_PRIVATE (dialer);

  if (!priv->reg_request)
  {
    /* We have yet to request registration, so lets do it */
    /* FIXME: do the pin stuff */
    g_print ("Requesting registration");
    moko_gsmd_connection_network_register (priv->connection);
  }
  else 
  {
    /* We check whether we've been registered yet, otherwise keep poking 
     * gsmd
     */
    if (priv->registered)
    {
      g_print ("Netwok Registered");
      return FALSE;
    }
    else
    {
      g_print ("Requesting registration");
      moko_gsmd_connection_network_register (priv->connection);
    }
  }
  
  return TRUE;
}

/* GObject functions */
static void
moko_dialer_dispose (GObject *object)
{
  MokoDialer *dialer;
  MokoDialerPrivate *priv;

  dialer = MOKO_DIALER (object);
  priv = dialer->priv;

  /* Close journal */
  moko_journal_close (priv->journal);

  /* Free contacts list */
  //contact_release_contact_list (&(priv->data->g_contactlist));

  G_OBJECT_CLASS (moko_dialer_parent_class)->dispose (object);
}

static void
moko_dialer_finalize (GObject *dialer)
{
  G_OBJECT_CLASS (moko_dialer_parent_class)->finalize (dialer);
}

#include "moko-dialer-glue.h"

static void
moko_dialer_class_init (MokoDialerClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = moko_dialer_finalize;
  obj_class->dispose = moko_dialer_dispose;

  /* add class signals */
  dialer_signals[INCOMING_CALL] =
    g_signal_new ("incoming_call", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoDialerClass, incoming_call),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 
                  1, G_TYPE_STRING);

  dialer_signals[OUTGOING_CALL] =
    g_signal_new ("outgoing_call", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoDialerClass, outgoing_call),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 
                  1, G_TYPE_STRING);

   dialer_signals[TALKING] =
    g_signal_new ("talking", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoDialerClass, talking),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

   dialer_signals[HUNG_UP] =
    g_signal_new ("hung_up", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoDialerClass, hung_up),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

   dialer_signals[REJECTED] =
    g_signal_new ("rejected", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoDialerClass, rejected),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  g_type_class_add_private (obj_class, sizeof (MokoDialerPrivate)); 
  dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass), 
                                   &dbus_glib_moko_dialer_object_info);
}

static void
moko_dialer_init (MokoDialer *dialer)
{
  MokoDialerPrivate *priv;
  MokoGsmdConnection *conn;

  priv = dialer->priv = MOKO_DIALER_GET_PRIVATE (dialer);

  /* create the dialer_data struct */
  priv->status = DIALER_STATUS_NORMAL;

  /* Initialise the contacts list */
  //contact_init_contact_data (&(priv->data->g_contactlist));

  /* Init the gsmd connection, and power it up */
  conn = priv->connection = moko_gsmd_connection_new ();
  moko_gsmd_connection_set_antenna_power (conn, TRUE);

  /* Handle network registration a few seconds after powering up the 
   * antenna*/ 
  priv->reg_request = TRUE;
  priv->registered = FALSE;
  g_timeout_add (GSM_REGISTER_TIMEOUT, 
                 (GSourceFunc)register_network_cb, 
                 dialer);
  
  /* Connect to the gsmd signals */
  g_signal_connect (G_OBJECT (conn), "network-registration", 
                    G_CALLBACK (on_network_registered), (gpointer)dialer);
  g_signal_connect (G_OBJECT (conn), "incoming-call", 
                    G_CALLBACK (on_incoming_call), (gpointer)dialer);
  g_signal_connect (G_OBJECT (conn), "incoming-clip",
                    G_CALLBACK (on_incoming_clip), (gpointer)dialer);
  g_signal_connect (G_OBJECT (conn), "pin-requested", 
                    G_CALLBACK (on_pin_requested), (gpointer)dialer);
  g_signal_connect (G_OBJECT (conn), "call-progress", 
                    G_CALLBACK (on_call_progress_changed), (gpointer)dialer);

  /* Set up the journal */
  priv->journal = moko_journal_open_default ();
  moko_journal_load_from_storage (priv->journal);

  /* Load the contacts store */
  priv->contacts = moko_contacts_get_default ();

  /* Create the window */
  priv->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (priv->window), "delete-event",
                    (GCallback) gtk_main_quit, NULL);
  gtk_window_set_title (GTK_WINDOW (priv->window), "Dialer");

  /* Notebook */
  priv->notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (priv->notebook), GTK_POS_BOTTOM);
  gtk_container_add (GTK_CONTAINER (priv->window), priv->notebook);

  /* Talking: We don't actually add it to the notebook yet, as it is only added
   * as/when needed. Therefore we just create it, and ref it (so it will 
   * survive reparenting.
   */
  priv->talking = moko_talking_new (priv->journal);
  g_object_ref (G_OBJECT (priv->talking));
  gtk_widget_show_all (priv->talking);
  g_signal_connect (G_OBJECT (priv->talking), "accept_call",
                    G_CALLBACK (on_talking_accept_call), (gpointer)dialer);
  g_signal_connect (G_OBJECT (priv->talking), "reject_call",
                    G_CALLBACK (on_talking_reject_call), (gpointer)dialer);
  g_signal_connect (G_OBJECT (priv->talking), "cancel_call",
                    G_CALLBACK (on_talking_cancel_call), (gpointer)dialer);
  g_signal_connect (G_OBJECT (priv->talking), "speaker_toggle",
                    G_CALLBACK (on_talking_speaker_toggle), (gpointer)dialer);

  /* Keypad */
  priv->keypad = moko_keypad_new ();
  g_signal_connect (G_OBJECT (priv->keypad), "dial_number",
                    G_CALLBACK (on_keypad_dial_clicked), (gpointer)dialer);
  g_signal_connect (G_OBJECT (priv->keypad), "digit_pressed",
                    G_CALLBACK (on_keypad_digit_pressed), (gpointer)dialer);
  gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook), priv->keypad,
                            gtk_image_new_from_file (PKGDATADIR"/dtmf.png"));
  gtk_container_child_set (GTK_CONTAINER (priv->notebook), priv->keypad,
                          "tab-expand", TRUE,
                          NULL);

  /* History */
  priv->history = moko_history_new (priv->journal);
  g_signal_connect (G_OBJECT (priv->history), "dial_number",
                    G_CALLBACK (on_history_dial_number), (gpointer)dialer);
  gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook), priv->history,
                            gtk_image_new_from_stock (MOKO_STOCK_CALL_HISTORY,
                                                      GTK_ICON_SIZE_BUTTON));
  gtk_container_child_set (GTK_CONTAINER (priv->notebook), priv->history,
                           "tab-expand", TRUE,
                           NULL);

  gtk_widget_show_all (priv->notebook);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), 0);
}

MokoDialer*
moko_dialer_get_default (void)
{
  static MokoDialer *dialer = NULL;
  if (dialer)
    return dialer;
  
  dialer = g_object_new (MOKO_TYPE_DIALER, NULL);

  return dialer;
}
