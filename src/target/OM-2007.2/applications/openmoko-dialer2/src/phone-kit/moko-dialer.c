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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <moko-gsmd-connection.h>
#include <moko-journal.h>
#include <moko-stock.h>

#include "moko-dialer.h"

#include "moko-contacts.h"
#include "moko-notify.h"
#include "moko-talking.h"
#include "moko-sound.h"

G_DEFINE_TYPE (MokoDialer, moko_dialer, G_TYPE_OBJECT)

#define MOKO_DIALER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_DIALER, MokoDialerPrivate))

struct _MokoDialerPrivate
{
  gint                status;
  gchar               *incoming_clip;

  /* handles user interaction */
  GtkWidget          *talking;

  /* Special objects */
  MokoGsmdConnection *connection;
  MokoJournal        *journal;
  MokoContacts       *contacts;
  MokoNotify         *notify;

  /* The shared MokoJournalEntry which is constantly created */
  MokoJournalEntry   *entry; 
  MokoTime           *time;

  /* Registration variables */
  MokoGsmdConnectionNetregType registered;
  MokoGSMLocation     gsm_location;
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

static MokoGsmdConnection * dialer_init_gsmd (MokoDialer *dialer);

/* DBus functions */

static gboolean
moko_dialer_get_status (MokoDialer *dialer, gint *OUT_status, GError **error)
{
  MokoDialerPrivate *priv;
  
  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);
  priv = dialer->priv;

  *OUT_status = priv->status;

  return TRUE;
}

gboolean
moko_dialer_dial (MokoDialer *dialer, const gchar *number, GError **error)
{

  MokoDialerPrivate *priv;
  MokoContactEntry *entry = NULL;

  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);
  g_return_val_if_fail (number != NULL, FALSE);
  priv = dialer->priv;

  g_debug ("Received dial request: %s", number);


  if (!priv->connection)
    priv->connection = dialer_init_gsmd (dialer);
  
  if (!priv->connection)
  {
    *error = g_error_new (PHONE_KIT_DIALER_ERROR, PK_DIALER_ERROR_GSMD, "Could not connect to gsmd");
    return FALSE;
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

    g_warning ("Cannot dial when dialer is busy. Current status = %s\n", strings[priv->status]);
    
    *error = g_error_new (PHONE_KIT_DIALER_ERROR, PK_DIALER_ERROR_BUSY, "Dialer busy");

    return FALSE;
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
      "Waiting for network registration",
      "Network registration denied",
      "",
      "Roaming network registered"
    };

    g_warning ("Not connected to network.\nCurrent status = %s ", strings[priv->registered]);
    *error = g_error_new (PHONE_KIT_DIALER_ERROR, PK_DIALER_ERROR_NOT_CONNECTED, "No Network");

    /* no point continuing if we're not connected to a network! */
    priv->status = DIALER_STATUS_NORMAL;
    return FALSE;
  }

  entry = moko_contacts_lookup (moko_contacts_get_default (), number);

  /* Prepare a voice journal entry */
  if (priv->journal)
  {
    priv->entry = moko_journal_entry_new (VOICE_JOURNAL_ENTRY);
    priv->time = moko_time_new_today ();
    moko_journal_entry_set_direction (priv->entry, DIRECTION_IN);
    moko_journal_entry_set_dtstart (priv->entry, priv->time);
    moko_journal_entry_set_source (priv->entry, "OpenMoko Dialer");
    moko_journal_voice_info_set_distant_number (priv->entry, number);
    if (entry && entry->contact->uid)
      moko_journal_entry_set_contact_uid (priv->entry, entry->contact->uid);
  }
  moko_talking_outgoing_call (MOKO_TALKING (priv->talking), number, entry);

  moko_gsmd_connection_voice_dial (MOKO_GSMD_CONNECTION (priv->connection), number);

  return TRUE;
}

static gboolean
moko_dialer_hang_up (MokoDialer *dialer, const gchar *message, GError **error)
{
  MokoDialerPrivate *priv;

  priv = dialer->priv; 
  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);

  /* check for gsmd connection */
  if (!priv->connection)
    priv->connection = dialer_init_gsmd (dialer);
  
  if (!priv->connection)
  {
    *error = g_error_new (PHONE_KIT_DIALER_ERROR, PK_DIALER_ERROR_GSMD, "Could not connect to gsmd");
    return FALSE;
  }

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

  /* Stop the notification */
  moko_notify_stop (priv->notify);

  moko_gsmd_connection_voice_hangup (priv->connection);

  g_signal_emit (G_OBJECT (dialer), dialer_signals[REJECTED], 0);
}


#if 0
static void
on_keypad_pin_entry (MokoKeypad  *keypad,
                     const gchar *in_pin,
                     MokoDialer  *dialer)
{
  MokoDialerPrivate *priv;
  gchar *pin;
  
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  pin = g_strdup (in_pin);

  g_debug ("Sending pin %s", pin);
  moko_gsmd_connection_send_pin (priv->connection, pin);

  moko_keypad_set_pin_mode (MOKO_KEYPAD (priv->keypad), FALSE);
  g_free (pin);
}
#endif

static void
on_talking_accept_call (MokoTalking *talking, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;
  
  if (priv->status != DIALER_STATUS_INCOMING)
    return;

  moko_gsmd_connection_voice_accept (priv->connection);
  priv->status = DIALER_STATUS_TALKING;

  /* Stop the notification */
  moko_notify_stop (priv->notify);  
  
  g_signal_emit (G_OBJECT (dialer), dialer_signals[TALKING], 0);
}

static void
on_talking_reject_call (MokoTalking *talking, MokoDialer *dialer)
{

  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  moko_gsmd_connection_voice_hangup (priv->connection);
  priv->status = DIALER_STATUS_NORMAL;

  /* Finalise and add the journal entry */
  if (priv->journal && priv->entry)
  {
    priv->time = moko_time_new_today ();
    moko_journal_entry_set_dtstart (priv->entry, priv->time);
    moko_journal_entry_set_dtend (priv->entry, priv->time);
    moko_journal_voice_info_set_was_missed (priv->entry, TRUE);
    moko_journal_add_entry (priv->journal, priv->entry);
    moko_journal_write_to_storage (priv->journal);
    priv->entry = NULL;
    priv->time = NULL;
  }

  g_signal_emit (G_OBJECT (dialer), dialer_signals[REJECTED], 0);
  moko_notify_stop (priv->notify);
}

static void
on_talking_cancel_call (MokoTalking *talking, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;
  
  moko_gsmd_connection_voice_hangup (priv->connection);
  
  priv->status = DIALER_STATUS_NORMAL;
  
  g_signal_emit (G_OBJECT (dialer), dialer_signals[HUNG_UP], 0);
}

static void
on_talking_silence (MokoTalking *talking, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  moko_notify_stop (priv->notify);
}

static void
on_talking_speaker_toggle (MokoTalking *talking, 
                           gboolean     speaker_phone,
                           MokoDialer  *dialer)
{
  /* Toggle speaker phone */
  static int on_speaker = FALSE;

  if (on_speaker)
    moko_sound_profile_set(SOUND_PROFILE_GSM_HANDSET);
  else
    moko_sound_profile_set(SOUND_PROFILE_GSM_SPEAKER_OUT);

  on_speaker = !on_speaker;

  g_debug ("Speaker toggled");
}
#if 0
static void
on_keypad_digit_pressed (MokoKeypad *keypad,
                         const gchar digit,
                         MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  if ((digit == '+') || (digit == 'w') || (digit == 'p'))
    return;

  if (priv->status == DIALER_STATUS_TALKING)
    moko_gsmd_connection_voice_dtmf (priv->connection, digit);
}
#endif
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

  switch (type)
  {
    case MOKO_GSMD_CONNECTION_NETREG_NONE:
    case MOKO_GSMD_CONNECTION_NETREG_SEARCHING:
      /* Do nothing */
      g_debug ("Searching for network");
      break;
    case MOKO_GSMD_CONNECTION_NETREG_DENIED:
      /* This may be a pin issue*/
      break;
    case MOKO_GSMD_CONNECTION_NETREG_HOME:
    case MOKO_GSMD_CONNECTION_NETREG_ROAMING:
      g_debug ("Network registered: LocationAreaCode: %x. CellID: %x.", lac, cell);
      priv->gsm_location.lac = lac;
      priv->gsm_location.cid = cell;
      break;
    default:
      g_warning ("Unhandled register event type = %d\n", type);
   };

  priv->registered = type;
}

static void
on_incoming_call (MokoGsmdConnection *conn, int type, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;
  
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  /* We sometimes get the signals multiple times */
  if (priv->status == DIALER_STATUS_INCOMING  
        || priv->status == DIALER_STATUS_TALKING)
  {
    /*g_debug ("We are already showing the incoming page");*/
    return;
  }
  priv->status = DIALER_STATUS_INCOMING;

  if (priv->incoming_clip)
    g_free (priv->incoming_clip);
  priv->incoming_clip = NULL;

  /* Prepare a voice journal entry */
  if (priv->journal)
  {
    priv->entry = moko_journal_entry_new (VOICE_JOURNAL_ENTRY);
    moko_journal_entry_set_direction (priv->entry, DIRECTION_IN);
    moko_journal_entry_set_source (priv->entry, "Openmoko Dialer");
    moko_journal_entry_set_gsm_location (priv->entry, &priv->gsm_location);
  }
  /* Set up the user interface */
  moko_talking_incoming_call (MOKO_TALKING (priv->talking), NULL, NULL);

  /* Start the notification */
  moko_notify_start (priv->notify);

  g_signal_emit (G_OBJECT (dialer), dialer_signals[INCOMING_CALL], 0, NULL);
}

static void
on_incoming_clip (MokoGsmdConnection *conn,
                  const gchar        *number,
                  MokoDialer         *dialer)
{
  MokoDialerPrivate *priv;
  MokoContactEntry *entry;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  if (priv->incoming_clip && (strcmp (number, priv->incoming_clip) == 0))
  {
    return;
  }

  priv->incoming_clip = g_strdup (number);
  
  entry = moko_contacts_lookup (moko_contacts_get_default (), number);
  moko_talking_set_clip (MOKO_TALKING (priv->talking), number, entry);

  /* Add the info to the journal entry */
  if (priv->journal && priv->entry)
  {
    moko_journal_voice_info_set_distant_number (priv->entry, number);
    if (entry)
      moko_journal_entry_set_contact_uid (priv->entry, entry->contact->uid);
  }
  g_signal_emit (G_OBJECT (dialer), dialer_signals[INCOMING_CALL], 
                 0, number);
  g_debug ("Incoming Number = %s", number);
}

static void
on_pin_requested (MokoGsmdConnection *conn, int type, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;
  
  g_debug ("Pin Requested");
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
      /* Finalise and add the journal entry */
      if (priv->journal && priv->entry)
      {
        priv->time = moko_time_new_today ();
        moko_journal_entry_set_dtend (priv->entry, priv->time);

        if (priv->status == DIALER_STATUS_INCOMING)
        {
          moko_journal_entry_set_dtstart (priv->entry, priv->time);
          moko_journal_voice_info_set_was_missed (priv->entry, TRUE);
        }

        moko_journal_add_entry (priv->journal, priv->entry);
        moko_journal_write_to_storage (priv->journal);
        priv->entry = NULL;
        priv->time = NULL;
      }

      moko_dialer_hung_up (dialer);

      if (priv->incoming_clip)
        g_free (priv->incoming_clip);
      priv->incoming_clip = NULL;

      moko_notify_stop (priv->notify);
      g_debug ("mokogsmd disconnect");
      break;
    
    case MOKO_GSMD_PROG_REJECT:
      moko_dialer_rejected (dialer);
      g_debug ("mokogsmd reject");
      break;
    
    case MOKO_GSMD_PROG_CONNECTED:
      if (priv->status != DIALER_STATUS_TALKING)
        moko_dialer_talking (dialer);
      moko_talking_accepted_call (MOKO_TALKING (priv->talking), NULL, NULL);

      /* Update a journal entry */
      if (priv->journal && priv->entry)
      {
        priv->time = moko_time_new_today ();
        moko_journal_entry_set_dtstart (priv->entry, priv->time);
      }
      g_debug ("mokogsmd connected");
      break;
    case MOKO_GSMD_PROG_SETUP:
      g_debug ("mokogsmd setup");
      break;
    case MOKO_GSMD_PROG_ALERT:
      g_debug ("mokogsmd alert");
      break;
    case  MOKO_GSMD_PROG_CALL_PROCEED:
      g_debug ("mokogsmd proceed");
      break;
    case MOKO_GSMD_PROG_SYNC:
      g_debug ("mokogsmd sync");
      break;
    case  MOKO_GSMD_PROG_PROGRESS:
      g_debug ("mokogsmd progress");
      break;
    case MOKO_GSMD_PROG_UNKNOWN:
    default:
      g_debug ("mokogsmd unknown");
      break;
  }
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
  if (priv->journal)
  {
    moko_journal_write_to_storage (priv->journal);
    moko_journal_close (priv->journal);
  }
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
dialer_display_error (GError *err)
{
  if (!err)
    return;
  g_warning (err->message);
}

static MokoGsmdConnection *
dialer_init_gsmd (MokoDialer *dialer)
{
  GError *err = NULL;
  MokoGsmdConnection *conn;
  MokoDialerPrivate *priv;
  priv = dialer->priv = MOKO_DIALER_GET_PRIVATE (dialer);

  /* Init the gsmd connection, and power it up */
  conn = moko_gsmd_connection_new ();
  moko_gsmd_connection_set_antenna_power (conn, TRUE, &err);

  dialer_display_error (err);
  if (err && err->code == MOKO_GSMD_ERROR_CONNECT)
  {
    g_object_unref (conn);
    return NULL;
  }

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

  /* FIXME:
   *  moko_gsmd_connection_get_network_status always seems to return 0 here */
  priv->registered = MOKO_GSMD_CONNECTION_NETREG_SEARCHING;
  moko_gsmd_connection_network_register (conn);

  return conn;
}

static void
moko_dialer_init (MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  priv = dialer->priv = MOKO_DIALER_GET_PRIVATE (dialer);

  /* create the dialer_data struct */
  priv->status = DIALER_STATUS_NORMAL;
  
  /* clear incoming clip */
  priv->incoming_clip = NULL;

  /* Initialise the contacts list */
  //contact_init_contact_data (&(priv->data->g_contactlist));

  priv->connection = dialer_init_gsmd (dialer);

  /* Set up the journal */
  priv->journal = moko_journal_open_default ();
  if (!priv->journal || !moko_journal_load_from_storage (priv->journal))
  {
    g_warning ("Cannot load journal");
    priv->journal = NULL;
  }
  else
    g_debug ("Journal Loaded");

  /* Load the contacts store */
  priv->contacts = moko_contacts_get_default ();

  /* Load the notification object */
  priv->notify = moko_notify_new ();


  /* Talking: This is the object that handles interaction with the user */
  priv->talking = moko_talking_new (priv->journal);
  g_object_ref (G_OBJECT (priv->talking));
  gtk_widget_show_all (priv->talking);
  g_signal_connect (G_OBJECT (priv->talking), "accept_call",
                    G_CALLBACK (on_talking_accept_call), (gpointer)dialer);
  g_signal_connect (G_OBJECT (priv->talking), "reject_call",
                    G_CALLBACK (on_talking_reject_call), (gpointer)dialer);
  g_signal_connect (G_OBJECT (priv->talking), "cancel_call",
                    G_CALLBACK (on_talking_cancel_call), (gpointer)dialer);
  g_signal_connect (G_OBJECT (priv->talking), "silence",
                    G_CALLBACK (on_talking_silence), (gpointer)dialer);
  g_signal_connect (G_OBJECT (priv->talking), "speaker_toggle",
                    G_CALLBACK (on_talking_speaker_toggle), (gpointer)dialer);


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
