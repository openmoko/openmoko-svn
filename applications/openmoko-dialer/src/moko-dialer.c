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

#include <libmokogsmd/moko-gsmd-connection.h>
#include <libmokojournal/moko-journal.h>

#include "moko-dialer.h"

/*
#include "contacts.h"
#include "moko-dialer.h"
#include "dialer-main.h"
#include "dialer-window-dialer.h"
#include "dialer-window-talking.h"
#include "dialer-window-outgoing.h"
#include "dialer-window-incoming.h"
#include "dialer-window-pin.h"
#include "dialer-window-history.h"

#include "dialer-callbacks-connection.h"
*/
G_DEFINE_TYPE (MokoDialer, moko_dialer, G_TYPE_OBJECT)

#define MOKO_DIALER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_DIALER, MokoDialerPrivate))

#define GSM_REGISTER_TIMEOUT 5000 /* Five seconds after powering up */

struct _MokoDialerPrivate
{
  gint            status;
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

/* DBus functions */
gboolean
moko_dialer_show_dialer (MokoDialer *dialer, GError *error)
{
  MokoDialerPrivate *priv;
  //GtkWidget *window;

  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);
  priv = dialer->priv;
 
/*  window = priv->data->window_present;
  if (window == 0)
    window = priv->data->window_dialer;

  if (window == NULL)
    return FALSE;

  gtk_widget_show_all (window);
  gtk_window_present (GTK_WINDOW (window));
*/
  return TRUE;
}


gboolean
moko_dialer_show_missed_calls (MokoDialer *dialer, GError *error)
{
  MokoDialerPrivate *priv;
  GtkWidget *window;

  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);
  priv = dialer->priv;
/*
  window = priv->data->window_history;

  if (!window)
    return FALSE;

  
  //Filter history on missed calls
  
  window_history_filter (priv->data, CALLS_MISSED);

  gtk_widget_show_all (window);
  gtk_window_present (GTK_WINDOW (window));
*/
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

  //window_outgoing_dial (priv->data, number);

  return TRUE;
}

static gboolean
moko_dialer_hang_up (MokoDialer *dialer, const gchar *message, GError *error)
{
  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);
  
  /* FIXME: Create a dialog and let the user know that another program is
   * requesting the connection be dropped, and why ($message).
   */

}

/* </dbus functions> */

void
moko_dialer_outgoing_call (MokoDialer *dialer, const gchar *number)
{
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  dialer->priv->status = DIALER_STATUS_DIALING;

  g_signal_emit (G_OBJECT (dialer), dialer_signals[OUTGOING_CALL], 0, number);
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
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  dialer->priv->status = DIALER_STATUS_NORMAL;

  g_signal_emit (G_OBJECT (dialer), dialer_signals[HUNG_UP], 0);
}

void
moko_dialer_rejected (MokoDialer *dialer)
{
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  dialer->priv->status = DIALER_STATUS_NORMAL;

  g_signal_emit (G_OBJECT (dialer), dialer_signals[REJECTED], 0);
}


/* Callbacks for MokoGsmdConnection */
static void
on_network_registered (MokoGsmdConnection *conn, 
                       int type, 
                       int lac, 
                       int cell,
                       MokoDialer *dialer)
{
  /* Network registered */
}

static void
on_incoming_call (MokoGsmdConnection *conn, int type, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;
  
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  //window_incoming_show (priv->data);

  g_signal_emit (G_OBJECT (dialer), dialer_signals[INCOMING_CALL], 0, NULL);
}

static void
on_pin_requested (MokoGsmdConnection *conn, int type, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  g_debug ("Incoming pin request for type %d", type);
  //gtk_widget_show_all (priv->data->window_pin);
}


static void
on_call_progress_changed (MokoGsmdConnection *conn, 
                          int type, 
                          MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  switch (type) 
  {
    case MOKO_GSMD_PROG_DISCONNECT:
    case MOKO_GSMD_PROG_RELEASE:
      moko_dialer_hung_up (dialer);
      break;
    case MOKO_GSMD_PROG_REJECT:
      moko_dialer_rejected (dialer);
      break;
    default:
      break;
  }
}
static gboolean
register_network_cb (MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_val_if_fail (MOKO_DIALER (dialer), TRUE);
  priv = MOKO_DIALER_GET_PRIVATE (dialer);

  g_debug ("Initial timeout");

  /* Check whether PIN window is currently visible - if not, issue register
  if (GTK_WIDGET_MAPPED (priv->data->window_pin))
  {
    g_debug ("PIN window is visible, delaying call to register");
    return TRUE;
  }
  else
  {
    g_debug ("PIN window not visible, calling register");
    moko_gsmd_connection_network_register (priv->data->connection);
  }
  */
  return FALSE;
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
  //moko_journal_close (priv->data->journal);

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
#if 0
  /* create the dialer_data struct */
  priv->data = g_new0 (MokoDialerData, 1);
  priv->status = DIALER_STATUS_NORMAL;

  /* Initialise the contacts list */
  contact_init_contact_data (&(priv->data->g_contactlist));

  /* Init the gsmd connection, and power it up */
  conn = priv->data->connection = moko_gsmd_connection_new ();
  moko_gsmd_connection_set_antenna_power (conn, TRUE);

  /* Handle network registration a few seconds after powering up the antenna*/ 
  g_timeout_add (GSM_REGISTER_TIMEOUT, 
                 (GSourceFunc)register_network_cb, 
                 dialer);
  
  /* Connect to the gsmd signals */
  g_signal_connect (G_OBJECT (conn), "network-registration", 
                    G_CALLBACK (on_network_registered), (gpointer)dialer);
  g_signal_connect (G_OBJECT (conn), "incoming-call", 
                    G_CALLBACK (on_incoming_call), (gpointer)dialer);
  g_signal_connect (G_OBJECT (conn), "pin-requested", 
                    G_CALLBACK (on_pin_requested), (gpointer)dialer);
  g_signal_connect (G_OBJECT (conn), "call-progress", 
                    G_CALLBACK (on_call_progress_changed), (gpointer)dialer);

  /* Set up the journal */
  priv->data->journal = moko_journal_open_default ();
  moko_journal_load_from_storage (priv->data->journal);

  /* Initialise the dialer windows */
  window_dialer_init (priv->data);
  window_incoming_init (priv->data);
  window_pin_init (priv->data);
  window_outgoing_init (priv->data);
  window_history_init (priv->data);
#endif
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
