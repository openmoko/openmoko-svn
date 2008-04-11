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
#include <unistd.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <moko-journal.h>
#include <moko-stock.h>

#include <libjana/jana.h>
#include <libjana-ecal/jana-ecal.h>

#include "moko-dialer.h"
#include "moko-listener.h"

#include "moko-contacts.h"
#include "moko-notify.h"
#include "moko-talking.h"
#include "moko-sound.h"
#include "moko-pin.h"

#include "moko-headset.h" 

static void
listener_interface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (MokoDialer, moko_dialer, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (MOKO_TYPE_LISTENER,
                                                listener_interface_init))

#define MOKO_DIALER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_DIALER, MokoDialerPrivate))

struct _MokoDialerPrivate
{
  PhoneKitDialerStatus    status;
  gchar                   *incoming_clip;

  /* handles user interaction */
  GtkWidget               *talking;

  /* gsmd connection variables */
  MokoNetwork             *network;
  enum lgsm_netreg_state  registered;
  MokoGSMLocation         gsm_location;
  
  /* Storage objects */
  MokoJournal             *journal;
  MokoContacts            *contacts;
  
  /* Notification handling object */
  MokoNotify              *notify;

  /* The shared MokoJournalEntry which is constantly created */
  MokoJournalEntry        *entry; 
  MokoTime                *time;
};

enum {
  PROP_STATUS = 1,
  PROP_NETWORK,
};

enum
{
  INCOMING_CALL,
  OUTGOING_CALL,
  TALKING,
  HUNG_UP,
  REJECTED,
  STATUS_CHANGED,
  
  LAST_SIGNAL
};

static guint dialer_signals[LAST_SIGNAL] = {0, };

/* DBus functions */

static void
moko_dialer_get_property (GObject *object, guint property_id,
                          GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
    case PROP_STATUS :
      g_value_set_int (value, moko_dialer_get_status (MOKO_DIALER (object)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
moko_dialer_set_property (GObject *object, guint property_id,
                          const GValue *value, GParamSpec *pspec)
{
  MokoDialerPrivate *priv = MOKO_DIALER (object)->priv;
  
  switch (property_id) {
    case PROP_NETWORK :
      if (priv->network) {
        moko_network_remove_listener (priv->network, MOKO_LISTENER (object));
        g_object_unref (priv->network);
      }
      priv->network = g_value_dup_object (value);
      moko_network_add_listener (priv->network, MOKO_LISTENER (object));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

PhoneKitDialerStatus
moko_dialer_get_status (MokoDialer *dialer)
{
  MokoDialerPrivate *priv = dialer->priv;
  return priv->status;
}

gboolean
moko_dialer_dial (MokoDialer *dialer, const gchar *number, GError **error)
{

  MokoDialerPrivate *priv;
  struct lgsm_addr addr;
  struct lgsm_handle *handle;
  MokoContactEntry *entry = NULL;
  PhoneKitDialerStatus status;

  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);
  g_return_val_if_fail (number != NULL, FALSE);
  priv = dialer->priv;
  status = priv->status;

  g_debug ("Received dial request: %s", number);

  if (!moko_network_get_lgsm_handle (priv->network, &handle, error))
    return FALSE;

  /* check current dialer state */
  if (0 || priv->status != PK_DIALER_NORMAL)
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

  /* dial *#06# to get IMEI code */
  if(g_strcasecmp(number,"*#06#") == 0) {
	  g_debug("dial *#06# !!");
	  lgsm_get_serial (handle);
	  
	  return TRUE;
  }

  priv->status = PK_DIALER_DIALING;

  /* check for network connection */
  if (priv->registered != GSMD_NETREG_REG_HOME
      && priv->registered != GSMD_NETREG_REG_ROAMING
      && priv->registered != GSMD_NETREG_DENIED 
      && g_strcasecmp(number,"112")!=0)
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
    *error = g_error_new (PHONE_KIT_NETWORK_ERROR, PK_NETWORK_ERROR_NOT_CONNECTED, "No Network");

    /* no point continuing if we're not connected to a network! */
    priv->status = PK_DIALER_NORMAL;
    if (status != priv->status) {
      g_signal_emit (G_OBJECT (dialer), dialer_signals[STATUS_CHANGED], 0,
                     priv->status);
    }
    return FALSE;
  }

  entry = moko_contacts_lookup (moko_contacts_get_default (), number);

  /* Prepare a voice journal entry */
  if (priv->journal)
  {
    priv->entry = moko_journal_entry_new (VOICE_JOURNAL_ENTRY);
    priv->time = moko_time_new_today ();
    moko_journal_entry_set_direction (priv->entry, DIRECTION_OUT);
    moko_journal_entry_set_dtstart (priv->entry, priv->time);
    moko_journal_entry_set_source (priv->entry, "OpenMoko Dialer");
    moko_journal_voice_info_set_distant_number (priv->entry, number);
    if (entry && entry->contact->uid)
      moko_journal_entry_set_contact_uid (priv->entry, entry->contact->uid);
  }
  moko_talking_outgoing_call (MOKO_TALKING (priv->talking), number, entry);

  /* TODO: No idea where '129' comes from, taken from libmokogsmd - refer to 
   * libgsmd.h in gsmd - It says "Refer to GSM 04.08 [8] subclause 10.5.4.7"
   */
  addr.type = 129;
  g_stpcpy (&addr.addr[0], number);
  lgsm_voice_out_init (handle, &addr);

  if (status != priv->status) {
    g_signal_emit (G_OBJECT (dialer), dialer_signals[STATUS_CHANGED], 0,
                   priv->status);
  }

  return TRUE;
}

static gboolean
moko_dialer_hang_up (MokoDialer *dialer, const gchar *message, GError **error)
{
  MokoDialerPrivate *priv;
  struct lgsm_handle *handle;

  priv = dialer->priv; 
  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);

  /* check for gsmd connection */
  if (!moko_network_get_lgsm_handle (priv->network, &handle, error))
    return FALSE;

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
  
  if (dialer->priv->status == PK_DIALER_TALKING) return;
  
  dialer->priv->status = PK_DIALER_TALKING;
  g_signal_emit (G_OBJECT (dialer), dialer_signals[STATUS_CHANGED], 0,
                 dialer->priv->status);
  g_signal_emit (G_OBJECT (dialer), dialer_signals[TALKING], 0);
}

void
moko_dialer_hung_up (MokoDialer *dialer)
{
  MokoDialerPrivate *priv;
  struct lgsm_handle *handle;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv; 
  
  g_warning("moko_dialer_hung_up");
  if (!moko_network_get_lgsm_handle (priv->network, &handle, NULL))
    return;
  if (priv->status != PK_DIALER_NORMAL) {
    priv->status = PK_DIALER_NORMAL;
    g_signal_emit (G_OBJECT (dialer), dialer_signals[STATUS_CHANGED], 0,
                   priv->status);
  }
  /* Send ATH to hang up the call from gsmd */
  lgsm_voice_hangup (handle);
  /* Stop the notification */
  moko_notify_stop (priv->notify);
  moko_talking_hide_window (MOKO_TALKING (priv->talking));
  
  g_signal_emit (G_OBJECT (dialer), dialer_signals[HUNG_UP], 0);
  
}

void
moko_dialer_rejected (MokoDialer *dialer)
{
  MokoDialerPrivate *priv;
  struct lgsm_handle *handle;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  g_warning("moko_dialer_rejected");
  if (!moko_network_get_lgsm_handle (priv->network, &handle, NULL))
    return;
  if (priv->status != PK_DIALER_NORMAL) {
    priv->status = PK_DIALER_NORMAL;
    g_signal_emit (G_OBJECT (dialer), dialer_signals[STATUS_CHANGED], 0,
                   priv->status);
  }

  /* Send ATH to hang up the call from gsmd */
  lgsm_voice_hangup (handle);
  /* Stop the notification */
  moko_notify_stop (priv->notify);
  moko_talking_hide_window (MOKO_TALKING (priv->talking));
  
  g_signal_emit (G_OBJECT (dialer), dialer_signals[REJECTED], 0);
}

static void
on_talking_accept_call (MokoTalking *talking, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;
  struct lgsm_handle *handle;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;
  
  if (priv->status != PK_DIALER_INCOMING)
    return;

  if (!moko_network_get_lgsm_handle (priv->network, &handle, NULL))
    return;
  
  lgsm_voice_in_accept (handle);
  if (priv->status != PK_DIALER_TALKING) {
    priv->status = PK_DIALER_TALKING;
    g_signal_emit (G_OBJECT (dialer), dialer_signals[STATUS_CHANGED], 0,
                   priv->status);
  }

  /* Stop the notification */
  moko_notify_stop (priv->notify);  
  
  g_signal_emit (G_OBJECT (dialer), dialer_signals[TALKING], 0);
}

static void
on_talking_reject_call (MokoTalking *talking, MokoDialer *dialer)
{

  MokoDialerPrivate *priv;
  struct lgsm_handle *handle;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  if (!moko_network_get_lgsm_handle (priv->network, &handle, NULL))
    return;
  lgsm_voice_hangup (handle);

  if (priv->status != PK_DIALER_NORMAL) {
    priv->status = PK_DIALER_NORMAL;
    g_signal_emit (G_OBJECT (dialer), dialer_signals[STATUS_CHANGED], 0,
                   priv->status);
  }

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
  struct lgsm_handle *handle;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;
  
  if (!moko_network_get_lgsm_handle (priv->network, &handle, NULL))
    return;
  lgsm_voice_hangup (handle);
  
  if (priv->status != PK_DIALER_NORMAL) {
    priv->status = PK_DIALER_NORMAL;
    g_signal_emit (G_OBJECT (dialer), dialer_signals[STATUS_CHANGED], 0,
                   priv->status);
  }
  
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
  {
    if ( HEADSET_STATUS_IN == moko_headset_status_get() ) 
      moko_sound_profile_set(SOUND_PROFILE_GSM_HEADSET);
    else
      moko_sound_profile_set(SOUND_PROFILE_GSM_HANDSET);
  }  
  else
    moko_sound_profile_set(SOUND_PROFILE_GSM_SPEAKER_OUT);

  on_speaker = !on_speaker;

  g_debug ("Speaker toggled");
}

static void
on_keypad_digit_pressed (MokoTalking *talking,
                         const gchar digit,
                         MokoDialer *dialer)
{
  MokoDialerPrivate *priv;
  struct lgsm_handle *handle;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  if (!moko_network_get_lgsm_handle (priv->network, &handle, NULL))
    return;
  if ((digit == '+') || (digit == 'w') || (digit == 'p'))
    return;

  if (priv->status == PK_DIALER_TALKING)
  {
    lgsm_voice_dtmf (handle, digit);
  }
}

/* Callbacks for gsmd events */
static void
on_network_registered (MokoListener *listener,
                       struct lgsm_handle *handle,
                       int type, 
                       int lac,  
                       int cell)
{
  MokoDialer *dialer = (MokoDialer *)listener;
  MokoDialerPrivate *priv;
  
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;
  
  if ((type == GSMD_NETREG_REG_HOME) || (type == GSMD_NETREG_REG_ROAMING)) {
    priv->gsm_location.lac = lac;
    priv->gsm_location.cid = cell;
  }
  
  priv->registered = type;
}

static void
on_incoming_call (MokoListener *listener, struct lgsm_handle *handle,
                  int type)
{
  MokoDialer *dialer = (MokoDialer *)listener;
  MokoDialerPrivate *priv;
  
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  /* We sometimes get the signals multiple times */
  if (priv->status == PK_DIALER_INCOMING  
        || priv->status == PK_DIALER_TALKING)
  {
    g_debug ("We are already showing the incoming page");
    return;
  }
  
  if (priv->status != PK_DIALER_INCOMING) {
    priv->status = PK_DIALER_INCOMING;
    g_signal_emit (G_OBJECT (dialer), dialer_signals[STATUS_CHANGED], 0,
                   priv->status);
  }

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
on_incoming_clip (MokoListener *listener, struct lgsm_handle *handle,
                  const gchar *number)
{
  MokoDialer *dialer = (MokoDialer *)listener;
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
on_call_progress (MokoListener *listener, struct lgsm_handle *handle,
                  int type)
{
  MokoDialer *dialer = (MokoDialer *)listener;
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;
 
  switch (type) 
  {
    case GSMD_CALLPROG_DISCONNECT:
      g_debug ("mokogsmd disconnect");
      break;
    
    case GSMD_CALLPROG_RELEASE:
      /* Finalise and add the journal entry */
      if (priv->journal && priv->entry)
      {
        priv->time = moko_time_new_today ();
        moko_journal_entry_set_dtend (priv->entry, priv->time);

        if (priv->status == PK_DIALER_INCOMING)
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

      g_debug ("mokogsmd release");
      break;
    
    case GSMD_CALLPROG_REJECT:
      moko_dialer_rejected (dialer);
      g_debug ("mokogsmd reject");
      break;
    
    case GSMD_CALLPROG_CONNECTED:
      if (priv->status != PK_DIALER_TALKING)
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
    case GSMD_CALLPROG_SETUP:
      g_debug ("mokogsmd setup");
      break;
    case GSMD_CALLPROG_ALERT:
      g_debug ("mokogsmd alert");
      break;
    case  GSMD_CALLPROG_CALL_PROCEED:
      g_debug ("mokogsmd proceed");
      break;
    case GSMD_CALLPROG_SYNC:
      g_debug ("mokogsmd sync");
      break;
    case GSMD_CALLPROG_PROGRESS:
      g_debug ("mokogsmd progress");
      break;
    case GSMD_CALLPROG_UNKNOWN:
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
    priv->journal = NULL;
  }
  
  G_OBJECT_CLASS (moko_dialer_parent_class)->dispose (object);
}

static void
moko_dialer_finalize (GObject *object)
{
  MokoDialer *dialer;
  MokoDialerPrivate *priv;
  
  dialer = MOKO_DIALER (object);
  priv = dialer->priv;

  G_OBJECT_CLASS (moko_dialer_parent_class)->finalize (object);
}

#include "moko-dialer-glue.h"

static void
moko_dialer_class_init (MokoDialerClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->get_property = moko_dialer_get_property;
  obj_class->set_property = moko_dialer_set_property;
  obj_class->finalize = moko_dialer_finalize;
  obj_class->dispose = moko_dialer_dispose;

  /* Add class properties */
  g_object_class_install_property (obj_class,
                                   PROP_STATUS,
                                   g_param_spec_int (
                                   "status",
                                   "PhoneKitDialerStatus",
                                   "The current SMS status.",
                                   PK_DIALER_NORMAL,
                                   PK_DIALER_TALKING,
                                   PK_DIALER_NORMAL,
                                   G_PARAM_READABLE));

  g_object_class_install_property (obj_class,
                                   PROP_NETWORK,
                                   g_param_spec_object (
                                   "network",
                                   "MokoNetwork *",
                                   "The parent MokoNetwork object.",
                                   MOKO_TYPE_NETWORK,
                                   G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

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

  dialer_signals[STATUS_CHANGED] =
    g_signal_new ("status_changed", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoDialerClass, status_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__INT,
                  G_TYPE_NONE, 
                  1, G_TYPE_INT);

  g_type_class_add_private (obj_class, sizeof (MokoDialerPrivate)); 
  dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass), 
                                   &dbus_glib_moko_dialer_object_info);
}

static void
listener_interface_init (gpointer g_iface, gpointer iface_data)
{
  MokoListenerInterface *iface = (MokoListenerInterface *)g_iface;
  
  iface->on_network_registered = on_network_registered;
  iface->on_incoming_call = on_incoming_call;
  iface->on_incoming_clip = on_incoming_clip;
  iface->on_call_progress = on_call_progress;
}

static void
moko_dialer_init (MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  priv = dialer->priv = MOKO_DIALER_GET_PRIVATE (dialer);

  /* create the dialer_data struct */
  priv->status = PK_DIALER_NORMAL;
  
  /* clear incoming clip */
  priv->incoming_clip = NULL;

  /* Initialise the contacts list */
  //contact_init_contact_data (&(priv->data->g_contactlist));

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
  priv->notify = moko_notify_get_default ();


  /* Talking: This is the object that handles interaction with the user */
  priv->talking = moko_talking_new ();
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
  g_signal_connect (priv->talking, "dtmf_key_press",
                    G_CALLBACK (on_keypad_digit_pressed), dialer);
}

MokoDialer*
moko_dialer_get_default (MokoNetwork *network)
{
  static MokoDialer *dialer = NULL;
  if (dialer)
    return dialer;

  dialer = g_object_new (MOKO_TYPE_DIALER, "network", network, NULL);

  return dialer;
}

