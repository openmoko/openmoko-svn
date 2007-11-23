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

#include <libgsmd/libgsmd.h>
#include <libgsmd/event.h>
#include <libgsmd/misc.h>
#include <libgsmd/sms.h>
#include <libgsmd/voicecall.h>
#include <libgsmd/phonebook.h>
#include <gsmd/usock.h>

#include <libjana/jana.h>
#include <libjana-ecal/jana-ecal.h>

#include "moko-dialer.h"

#include "moko-contacts.h"
#include "moko-notify.h"
#include "moko-talking.h"
#include "moko-sound.h"
#include "moko-pin.h"

#include "moko-dialer-mcc-dc.h"

G_DEFINE_TYPE (MokoDialer, moko_dialer, G_TYPE_OBJECT)

#define MOKO_DIALER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_DIALER, MokoDialerPrivate))

typedef struct {
  GSource source;
  GPollFD pollfd;
  struct lgsm_handle *handle;
} MokoDialerSource;

struct _MokoDialerPrivate
{
  gint               status;
  gchar              *incoming_clip;
  gchar              *own_number;
  gchar              *network_name;
  gchar              *network_number;
  gchar              *imsi;

  /* handles user interaction */
  GtkWidget          *talking;

  /* gsmd connection variables */
  struct lgsm_handle *handle;
  MokoDialerSource   *source;
  
  /* Storage objects */
  JanaStore          *sms_store;
  gboolean           sms_store_open;
	JanaNote           *last_msg;
  MokoJournal        *journal;
  MokoContacts       *contacts;
  
  /* Notification handling object */
  MokoNotify         *notify;

  /* The shared MokoJournalEntry which is constantly created */
  MokoJournalEntry   *entry; 
  MokoTime           *time;

  /* Registration variables */
  enum lgsm_netreg_state registered;
  MokoGSMLocation    gsm_location;
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

static void dialer_init_gsmd (MokoDialer *dialer);

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
  struct lgsm_addr addr;
  MokoContactEntry *entry = NULL;

  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);
  g_return_val_if_fail (number != NULL, FALSE);
  priv = dialer->priv;

  g_debug ("Received dial request: %s", number);


  if (!priv->handle) dialer_init_gsmd (dialer);
  
  if (!priv->handle)
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
  if (priv->registered != GSMD_NETREG_REG_HOME
      && priv->registered != GSMD_NETREG_REG_ROAMING
      && priv->registered != GSMD_NETREG_DENIED)
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
  lgsm_voice_out_init (priv->handle, &addr);

  return TRUE;
}

static gboolean
moko_dialer_hang_up (MokoDialer *dialer, const gchar *message, GError **error)
{
  MokoDialerPrivate *priv;

  priv = dialer->priv; 
  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);

  /* check for gsmd connection */
  if (!priv->handle) dialer_init_gsmd (dialer);
  
  if (!priv->handle)
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

  lgsm_voice_hangup (priv->handle);
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

  lgsm_voice_hangup (priv->handle);

  g_signal_emit (G_OBJECT (dialer), dialer_signals[REJECTED], 0);
}

static void
on_talking_accept_call (MokoTalking *talking, MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;
  
  if (priv->status != DIALER_STATUS_INCOMING)
    return;

  lgsm_voice_in_accept (priv->handle);
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

  lgsm_voice_hangup (priv->handle);
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
  
  lgsm_voice_hangup (priv->handle);
  
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

static void
on_keypad_digit_pressed (MokoTalking *talking,
                         const gchar digit,
                         MokoDialer *dialer)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  if ((digit == '+') || (digit == 'w') || (digit == 'p'))
    return;

  if (priv->status == DIALER_STATUS_TALKING)
  {
    lgsm_voice_dtmf (priv->handle, digit);
  }
}

/* Callbacks for gsmd events */
static void
on_network_registered (MokoDialer *dialer,
                       int type, 
                       int lac,  
                       int cell)
{
  MokoDialerPrivate *priv;
  
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  switch (type)
  {
    case GSMD_NETREG_UNREG:
    case GSMD_NETREG_UNREG_BUSY:
      /* Do nothing */
      g_debug ("Searching for network");
      break;
    case GSMD_NETREG_DENIED:
      /* This may be a pin issue*/
      break;
    case GSMD_NETREG_REG_HOME:
    case GSMD_NETREG_REG_ROAMING:
      g_debug ("Network registered: LocationAreaCode: %x. CellID: %x.", lac, cell);
      priv->gsm_location.lac = lac;
      priv->gsm_location.cid = cell;
      
      if ((priv->registered != GSMD_NETREG_REG_HOME) &&
          (priv->registered != GSMD_NETREG_REG_ROAMING)) {
        /* Retrieve operator name */
        lgsm_oper_get (priv->handle);
        
        /* Retrieve operator list to get current country code */
        lgsm_opers_get (priv->handle);
        
        /* Retrieve IMSI to get home country code */
        lgsm_get_imsi (priv->handle);
      }
      
      break;
    default:
      g_warning ("Unhandled register event type = %d\n", type);
   };

  priv->registered = type;
}

static void
on_incoming_call (MokoDialer *dialer, int type)
{
  MokoDialerPrivate *priv;
  
  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;

  /* We sometimes get the signals multiple times */
  if (priv->status == DIALER_STATUS_INCOMING  
        || priv->status == DIALER_STATUS_TALKING)
  {
    g_debug ("We are already showing the incoming page");
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
on_incoming_clip (MokoDialer *dialer, const gchar *number)
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

static gboolean
register_to_network (MokoDialer *dialer)
{
  g_return_val_if_fail (MOKO_IS_DIALER (dialer), FALSE);

  lgsm_netreg_register (dialer->priv->handle, "");
  return FALSE;
}

static void
on_pin_requested (MokoDialer *dialer, int type)
{
  MokoDialerPrivate *priv;
  gchar *pin;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  g_debug ("Pin Requested");
  priv = dialer->priv;
  
  pin = get_pin_from_user ();
  if (!pin)
    return;
  
  lgsm_pin (priv->handle, 1, pin, NULL);
  g_free (pin);
  
  /* temporary delay before we try registering
   * FIXME: this should check if pin was OK */
  g_timeout_add_seconds (1, (GSourceFunc) register_to_network, dialer);
}

static void
on_call_progress_changed (MokoDialer *dialer, int type)
{
  MokoDialerPrivate *priv;

  g_return_if_fail (MOKO_IS_DIALER (dialer));
  priv = dialer->priv;
 
  switch (type) 
  {
    case GSMD_CALLPROG_DISCONNECT:
    case GSMD_CALLPROG_RELEASE:
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
    
    case GSMD_CALLPROG_REJECT:
      moko_dialer_rejected (dialer);
      g_debug ("mokogsmd reject");
      break;
    
    case GSMD_CALLPROG_CONNECTED:
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

  if (priv->handle) {
    lgsm_exit (priv->handle);
    priv->handle = NULL;
  }

  if (priv->source) {
    g_source_destroy ((GSource *)priv->source);
    priv->source = NULL;
  }

  if (priv->sms_store) {
    g_object_unref (priv->sms_store);
    priv->sms_store = NULL;
  }

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

  g_free (priv->own_number);
  g_free (priv->network_name);
  g_free (priv->network_number);
  g_free (priv->imsi);
  
  G_OBJECT_CLASS (moko_dialer_parent_class)->finalize (object);
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
status_report_added_cb (JanaStoreView *view, GList *components, gchar *ref)
{
  MokoDialerPrivate *priv = moko_dialer_get_default ()->priv;

  for (; components; components = components->next) {
    gchar *compref;
    JanaComponent *comp = JANA_COMPONENT (components->data);
    
    compref = jana_component_get_custom_prop (
      comp, "X-PHONEKIT-SMS-REF");
    if (compref && (strcmp (compref, ref) == 0)) {
      jana_utils_component_remove_category (comp, "Sending");
      jana_utils_component_insert_category (comp, "Sent", 0);
      jana_store_modify_component (priv->sms_store, comp);
      g_debug ("Setting message status to confirmed sent");
    }
    g_free (ref);
  }
}

static void
status_report_progress_cb (JanaStoreView *view, gint percent, gchar *ref)
{
  if (percent != 100) return;
  
  g_object_unref (view);
  g_free (ref);
}

static void
store_sms (MokoDialer *dialer, struct gsmd_sms_list *sms)
{
  gchar *message;

  MokoDialerPrivate *priv = dialer->priv;

  /* Return if we haven't opened the journal yet - signals will be re-fired 
   * when the journal is opened anyway.
   */
  if (!priv->sms_store_open) return;

  /* Ignore voicemail notifications */
  if (sms->payload.is_voicemail) return;

  /* TODO: Verify type of message for journal (sent/received) - 
   *       Assuming received for now, as messages sent with phone-kit 
   *       will be marked already.
   */
  message = NULL;
  switch (sms->payload.coding_scheme) {
  case ALPHABET_DEFAULT :
    g_debug ("Decoding 7-bit ASCII message");
    message = g_malloc0 (GSMD_SMS_DATA_MAXLEN);
    unpacking_7bit_character (&sms->payload, message);
    break;
  case ALPHABET_8BIT :
    /* TODO: Verify: Is this encoding just UTF-8? (it is on my Samsung phone) */
    g_debug ("Decoding UTF-8 message");
    message = g_strdup (sms->payload.data);
    break;
  case ALPHABET_UCS2 :
    g_debug ("Decoding UCS-2 message");
    message = g_utf16_to_utf8 ((const gunichar2 *)sms->payload.data,
                               sms->payload.length, NULL, NULL, NULL);
    break;
  }
  
  /* Store message in the journal */
  if (message) {
    struct lgsm_sms_delete sms_del;
    gchar *author;
    JanaNote *note = jana_ecal_note_new ();
    
    g_debug ("Moving message to journal:\n\"%s\"", message);
    
    author = g_strconcat (((sms->addr.type & __GSMD_TOA_TON_MASK) ==
                          GSMD_TOA_TON_INTERNATIONAL) ? "+" : "",
                          sms->addr.number, NULL);
    jana_note_set_author (note, author);
    g_free (author);
    
    jana_note_set_recipient (note, priv->own_number);
    
    jana_note_set_body (note, message);
    
    /* TODO: Set creation time from SMS timestamp */
    
    /* Add SMS to store */
    jana_store_add_component (priv->sms_store, JANA_COMPONENT (note));
    
    /* Delete SMS from internal storage */
    sms_del.index = sms->index;
    sms_del.delflg = LGSM_SMS_DELFLG_INDEX;
    lgsm_sms_delete (priv->handle, &sms_del);
    
    g_free (message);
  }
}

static int 
gsmd_eventhandler (struct lgsm_handle *lh, int evt_type,
                   struct gsmd_evt_auxdata *aux)
{
  MokoDialer *dialer = moko_dialer_get_default ();
  MokoDialerPrivate *priv = dialer->priv;
  
  switch (evt_type) {
  case GSMD_EVT_IN_CALL :
    on_incoming_call (dialer, aux->u.call.type);
    break;
  case GSMD_EVT_IN_SMS : /* Incoming SMS */
    /* TODO: Read UDH for multi-part messages */
    g_debug ("Received incoming SMS");
    if (aux->u.sms.inlined) {
      struct gsmd_sms_list * sms = (struct gsmd_sms_list *)aux->data;
      g_debug ("Message inline");
      store_sms (dialer, sms);
    } else {
      g_debug ("Message stored on SIM, reading...");
      lgsm_sms_read (lh, aux->u.sms.index);
    }
    break;
  case GSMD_EVT_IN_DS : /* SMS status report */
    if (aux->u.ds.inlined) {
      struct gsmd_sms_list *sms = (struct gsmd_sms_list *) aux->data;
      
      /* TODO: I'm not entirely sure of the spec when if 
       *       storing an unsent message means it failed?
       */
      if (sms->payload.coding_scheme == LGSM_SMS_STO_SENT) {
        gchar *ref = g_strdup_printf ("%d", sms->index);
        JanaStoreView *view = jana_store_get_view (priv->sms_store);
        
        g_debug ("Received sent SMS status report");
        jana_store_view_add_match (view, JANA_STORE_VIEW_CATEGORY, "Sending");
        g_signal_connect (view, "added",
                          G_CALLBACK (status_report_added_cb), ref);
        g_signal_connect (view, "progress",
                          G_CALLBACK (status_report_progress_cb), ref);
      }
    } else {
      g_warning ("Not an in-line event, unhandled");
    }
    break;
  case GSMD_EVT_IN_CLIP :
    on_incoming_clip (dialer, aux->u.clip.addr.number);
    break;
  case GSMD_EVT_NETREG :
    on_network_registered (dialer, aux->u.netreg.state,
                           aux->u.netreg.lac, aux->u.netreg.ci);
    break;
  case GSMD_EVT_PIN :
    on_pin_requested (dialer, aux->u.pin.type);
    break;
  case GSMD_EVT_OUT_STATUS :
    on_call_progress_changed (dialer, aux->u.call_status.prog);
    break;
  default :
    g_warning ("Unhandled gsmd event (%d)", evt_type);
  }
  
  return 0;
}

static int
sms_msghandler (struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
  MokoDialer *dialer = moko_dialer_get_default ();
  MokoDialerPrivate *priv = dialer->priv;

  /* Store sent messages */
  if ((gmh->msg_subtype == GSMD_SMS_SEND) && priv->last_msg) {
    int *result = (int *) ((void *) gmh + sizeof(*gmh));
    gchar *uid = jana_component_get_uid (
      JANA_COMPONENT (priv->last_msg));
    
    if (*result >= 0) {
      gchar *ref = g_strdup_printf ("%d", *result);
      jana_component_set_custom_prop (JANA_COMPONENT (priv->last_msg),
                                      "X-PHONEKIT-SMS-REF", ref);
      g_free (ref);
      g_debug ("Sent message accepted");
    } else {
      g_debug ("Sent message rejected");
      jana_utils_component_remove_category (JANA_COMPONENT(priv->last_msg),
                                            "Sending");
      jana_utils_component_insert_category (JANA_COMPONENT(priv->last_msg),
                                            "Rejected", 0);
      /* TODO: Add error codes? 42 = congestion? */
    }
    jana_store_modify_component (priv->sms_store,
                                 JANA_COMPONENT (priv->last_msg));
    
    g_free (uid);
    g_object_unref (priv->last_msg);
    priv->last_msg = NULL;
  } else if ((gmh->msg_subtype == GSMD_SMS_LIST) ||
             (gmh->msg_subtype == GSMD_SMS_READ)) {
    struct gsmd_sms_list *sms_list = (struct gsmd_sms_list *)
                                     ((void *) gmh + sizeof(*gmh));

    g_debug ("Storing message on SIM");
    store_sms (dialer, sms_list);
  } else {
    return -EINVAL;
  }
  
  return 0;
}

static int
net_msghandler (struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
  MokoDialer *dialer = moko_dialer_get_default ();
  MokoDialerPrivate *priv = dialer->priv;

	const char *oper = (char *) gmh + sizeof(*gmh);
  const struct gsmd_own_number *num = (struct gsmd_own_number *)
                                      ((void *) gmh + sizeof(*gmh));
	const struct gsmd_msg_oper *opers = (struct gsmd_msg_oper *)
		((void *) gmh + sizeof(*gmh));

  switch (gmh->msg_subtype) {
    case GSMD_NETWORK_GET_NUMBER :
      g_free (priv->own_number);
      priv->own_number = g_strdup (num->addr.number);
      break;
    case GSMD_NETWORK_OPER_GET :
      g_free (priv->network_name);
      if (oper[0]) priv->network_name = g_strdup (oper);
      else priv->network_name = NULL;
      break;
    case GSMD_NETWORK_OPER_LIST :
      for (; !opers->is_last; opers++) {
        if (opers->stat == GSMD_OPER_CURRENT) {
          g_free (priv->network_number);
          priv->network_number = g_strndup (opers->opname_num,
                                            sizeof(opers->opname_num));
          break;
        }
      }
      break;
    default :
      return -EINVAL;
  }
  
  return 0;
}

static int
pb_msghandler (struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
  MokoDialer *dialer = moko_dialer_get_default ();
  MokoDialerPrivate *priv = dialer->priv;

  switch (gmh->msg_subtype) {
    case GSMD_PHONEBOOK_GET_IMSI :
      priv->imsi = g_strdup ((char *)gmh + sizeof (*gmh));
      break;
    default :
      return -EINVAL;
  }
  
  return 0;
}

static gboolean 
connection_source_prepare (GSource* self, gint* timeout)
{
    return FALSE;
}

static gboolean
connection_source_check (GSource* source)
{
  MokoDialerSource *self = (MokoDialerSource *)source;
  return self->pollfd.revents & G_IO_IN;
}

static gboolean 
connection_source_dispatch (GSource *source, GSourceFunc callback,
                            gpointer data)
{
  char buf[1025];
  int size;

  MokoDialerSource *self = (MokoDialerSource *)source;

  size = read (self->pollfd.fd, &buf, sizeof(buf));
  if (size < 0) {
    g_warning ("moko_gsmd_connection_source_dispatch:%s %s",
               "read error from libgsmd:", strerror (errno));
  } else {
    if (size == 0) /* EOF */
      return FALSE;
    lgsm_handle_packet (self->handle, buf, size);
  }

  return TRUE;
}

static void
sms_store_opened_cb (JanaStore *store, MokoDialer *self)
{
  MokoDialerPrivate *priv = self->priv;
  priv->sms_store_open = TRUE;

  g_debug ("SMS store opened");
  
  if (!priv->handle) return;
  
  /* Register SMS handling callback */
  lgsm_register_handler (priv->handle, GSMD_MSG_SMS, &sms_msghandler);

  /* List all messages to move to journal */
  lgsm_sms_list (priv->handle, GSMD_SMS_ALL);
}

static void
dialer_init_gsmd (MokoDialer *dialer)
{
  static GSourceFuncs funcs = {
    connection_source_prepare,
    connection_source_check,
    connection_source_dispatch,
    NULL,
  };

  MokoDialerPrivate *priv;
  priv = dialer->priv;

  /* Get a gsmd handle */
  if (!(priv->handle = lgsm_init (LGSMD_DEVICE_GSMD))) {
    g_warning ("Error connecting to gsmd");
    return;
  }
  
  /* Power the gsm modem up */
  if (lgsm_phone_power (priv->handle, 1) == -1) {
    g_warning ("Error powering up gsm modem");
    lgsm_exit (priv->handle);
    priv->handle = NULL;
    return;
  }
  
  /* Add event handlers */
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_CALL, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_CLIP, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_SMS, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_DS, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_NETREG, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_OUT_STATUS, gsmd_eventhandler);
  lgsm_register_handler (priv->handle, GSMD_MSG_NETWORK, &net_msghandler);
  lgsm_register_handler (priv->handle, GSMD_MSG_PHONEBOOK, &pb_msghandler);

  /* Register with network */
  priv->registered = GSMD_NETREG_UNREG;
  lgsm_netreg_register (priv->handle, "");

  /* Get phone number */
  lgsm_get_subscriber_num (priv->handle);
  
  /* Start polling for events */
  priv->source = (MokoDialerSource *)
    g_source_new (&funcs, sizeof (MokoDialerSource));
  priv->source->handle = priv->handle;
  priv->source->pollfd.fd = lgsm_fd (priv->handle);
  priv->source->pollfd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
  priv->source->pollfd.revents = 0;
  g_source_add_poll ((GSource*)priv->source, &priv->source->pollfd);
  g_source_attach ((GSource*)priv->source, NULL);
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

  dialer_init_gsmd (dialer);

  /* Set up the journal */
  priv->journal = moko_journal_open_default ();
  if (!priv->journal || !moko_journal_load_from_storage (priv->journal))
  {
    g_warning ("Cannot load journal");
    priv->journal = NULL;
  }
  else
    g_debug ("Journal Loaded");

  /* Get the SMS note store */
  priv->sms_store = jana_ecal_store_new (JANA_COMPONENT_NOTE);
  g_signal_connect (priv->sms_store, "opened",
                    G_CALLBACK (sms_store_opened_cb), dialer);
  jana_store_open (priv->sms_store);

  /* Load the contacts store */
  priv->contacts = moko_contacts_get_default ();

  /* Load the notification object */
  priv->notify = moko_notify_new ();


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
moko_dialer_get_default (void)
{
  static MokoDialer *dialer = NULL;
  if (dialer)
    return dialer;

  dialer = g_object_new (MOKO_TYPE_DIALER, NULL);

  return dialer;
}

static gboolean
moko_dialer_check_gsmd (MokoDialer *self, GError **error)
{
  MokoDialerPrivate *priv = self->priv;
  
  if (!priv->handle) {
    *error = g_error_new (PHONE_KIT_DIALER_ERROR, PK_DIALER_ERROR_GSMD,
                          "Failed to connect to gsmd");
    return FALSE;
  }
  
  return TRUE;
}

static gboolean
moko_dialer_check_registration (MokoDialer *self, GError **error)
{
  MokoDialerPrivate *priv = self->priv;
  
  if ((priv->registered != GSMD_NETREG_REG_HOME) &&
      (priv->registered != GSMD_NETREG_REG_ROAMING)) {
    *error = g_error_new (PHONE_KIT_DIALER_ERROR, PK_DIALER_ERROR_NOT_CONNECTED,
                          "Not registered to a network");
    return FALSE;
  }
  
  return TRUE;
}

static const gchar *
moko_dialer_cc_from_mcc (gchar *mcc)
{
  gint i;
  for (i = 0; mcc_to_dc[i][0]; i++) {
    if (strncmp (mcc, mcc_to_dc[i][0], 3) == 0) {
      return mcc_to_dc[i][1];
    }
  }
  
  return NULL;
}

gboolean
moko_dialer_send_sms (MokoDialer *self, const gchar *number,
                      const gchar *message, gchar **uid, GError **error)
{
  MokoDialerPrivate *priv;
  struct lgsm_sms sms;
  gint msg_length, c;
  gboolean ascii;
  JanaNote *note;
  
  g_assert (self && number && message);

  if (!moko_dialer_check_gsmd (self, error)) return FALSE;
  if (!moko_dialer_check_registration (self, error)) return FALSE;
  priv = self->priv;
  
  if (!priv->sms_store_open) {
    *error = g_error_new (PHONE_KIT_DIALER_ERROR, PK_DIALER_ERROR_SMS_STORE,
                          "SMS store not opened");
    return FALSE;
  }
  
  /* Ask for delivery report */
  sms.ask_ds = 1;
  
  /* Set destination number */
  if (strlen (number) > GSMD_ADDR_MAXLEN + 1) {
    *error = g_error_new (PHONE_KIT_DIALER_ERROR, PK_DIALER_ERROR_NO_TOOLONG,
                          "Number too long");
    return FALSE;
  } else {
    strcpy (sms.addr, number);
  }
  
  /* Set message */
  /* Check if the text is ascii (and pack in 7 bits if so) */
  ascii = TRUE;
  for (c = 0; message[c] != '\0'; c++) {
    if (((guint8)message[c]) > 0x7F) {
      ascii = FALSE;
      break;
    }
  }
  
  /* TODO: Multi-part messages using UDH */
  msg_length = strlen (message);
  if ((ascii && (msg_length > 160)) || (msg_length > 140)) {
      *error = g_error_new (PHONE_KIT_DIALER_ERROR, PK_DIALER_ERROR_SMS_TOOLONG,
                            "Message too long");
      return FALSE;
  }
  if (ascii) {
    packing_7bit_character (message, &sms);
  } else {
    sms.alpha = ALPHABET_8BIT;
    sms.length = strlen (message);
    strcpy ((gchar *)sms.data, message);
  }
  
  /* Send message */
  lgsm_sms_send (priv->handle, &sms);
  
  /* Store sent message in journal */
  note = jana_ecal_note_new ();
  jana_note_set_recipient (note, number);
  jana_note_set_author (note, priv->own_number);
  
  jana_note_set_body (note, message);
  jana_component_set_categories (JANA_COMPONENT (note),
    (const gchar *[]){ "Sending", NULL});
  
  jana_store_add_component (priv->sms_store,
    JANA_COMPONENT (note));
  if (uid) *uid = jana_component_get_uid (JANA_COMPONENT (note));
  
  if (priv->last_msg) {
    g_warning ("Confirmation not received for last sent SMS, "
      "delivery report will be lost.");
    g_object_unref (priv->last_msg);
    priv->last_msg = NULL;
  }
  priv->last_msg = note;
  
  return TRUE;
}

gboolean
moko_dialer_get_provider_name (MokoDialer *self, gchar **name, GError **error)
{
  MokoDialerPrivate *priv;
  
  if (!moko_dialer_check_gsmd (self, error)) return FALSE;
  if (!moko_dialer_check_registration (self, error)) return FALSE;
  priv = self->priv;
  
  if (!priv->network_name) {
    *error = g_error_new (PHONE_KIT_DIALER_ERROR, PK_DIALER_ERROR_NO_PROVIDER,
                          "No current provider");
    return FALSE;
  }

  if (name) *name = g_strdup (priv->network_name);
  return TRUE;
}

gboolean
moko_dialer_get_subscriber_number (MokoDialer *self, gchar **number,
                                   GError **error)
{
  MokoDialerPrivate *priv;
  
  if (!moko_dialer_check_gsmd (self, error)) return FALSE;
  priv = self->priv;
  
  if (!priv->own_number) {
    *error = g_error_new (PHONE_KIT_DIALER_ERROR, PK_DIALER_ERROR_NO_NUMBER,
                          "Unable to retrieve subscriber number");
    return FALSE;
  }
  
  if (number) *number = g_strdup (priv->own_number);
  return TRUE;
}

gboolean
moko_dialer_get_country_code (MokoDialer *self, gchar **dial_code,
                              GError **error)
{
  MokoDialerPrivate *priv;
  
  if (!moko_dialer_check_gsmd (self, error)) return FALSE;
  if (!moko_dialer_check_registration (self, error)) return FALSE;
  priv = self->priv;
  
  if (!priv->network_number) {
    *error = g_error_new (PHONE_KIT_DIALER_ERROR,
                          PK_DIALER_ERROR_NO_PROVIDER_NUM,
                          "Unable to retrieve provider number");
    return FALSE;
  }
  
  if (dial_code)
    *dial_code = g_strdup (moko_dialer_cc_from_mcc (priv->network_number));
  
  return TRUE;
}

gboolean
moko_dialer_get_home_country_code (MokoDialer *self, gchar **dial_code,
                                   GError **error)
{
  MokoDialerPrivate *priv;
  
  if (!moko_dialer_check_gsmd (self, error)) return FALSE;
  if (!moko_dialer_check_registration (self, error)) return FALSE;
  priv = self->priv;
  
  if (!priv->network_number) {
    *error = g_error_new (PHONE_KIT_DIALER_ERROR,
                          PK_DIALER_ERROR_NO_IMSI,
                          "Unable to retrieve IMSI");
    return FALSE;
  }
  
  if (dial_code)
    *dial_code = g_strdup (moko_dialer_cc_from_mcc (priv->imsi));
  
  return TRUE;
}

