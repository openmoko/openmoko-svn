/*
 *  moko-sms; a GObject wrapper for phone-kit that exports method and
 *  signals over dbus relating to SMS messaging
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

#include <string.h>
#include <sys/vfs.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libjana/jana.h>
#include <libjana-ecal/jana-ecal.h>

#include <libnotify/notification.h>

#include <libmokoui2/moko-stock.h>

#include "moko-sms.h"
#include "moko-network.h"
#include "moko-listener.h"
#include "moko-notify.h"
#include "moko-marshal.h"

static void
listener_interface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (MokoSms, moko_sms, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (MOKO_TYPE_LISTENER,
                                                listener_interface_init))

#define MOKO_SMS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_SMS, MokoSmsPrivate))

enum {
  PROP_STATUS = 1,
  PROP_NETWORK,
};

enum
{
  STATUS_CHANGED,
  SIM_FULL,
  MEMORY_FULL,
  
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0, };

struct _MokoSmsPrivate
{
  MokoNetwork        *network;
  /*gboolean           got_subscriber_number;*/
  gboolean           handling_sms;
  
  JanaStore          *sms_store;
  gboolean           sms_store_open;
  JanaNote           *last_msg;
  
  GList              *unread_uids;
  NotifyNotification *notification;
  MokoNotify         *notify;
  
  gboolean           sim_full;
  gboolean           memory_full;
  
  guint              memory_idle;
};

static void start_handling_sms (MokoSms *sms);
static void stop_handling_sms (MokoSms *sms);
static void open_sms_store (MokoSms *sms);

static void
moko_sms_get_property (GObject *object, guint property_id,
                       GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
    case PROP_STATUS :
      g_value_set_int (value, moko_sms_get_status (MOKO_SMS (object)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

/*static void
subscriber_number_changed_cb (MokoNetwork *network, const gchar *number,
                              MokoSms *sms)
{
  MokoSmsPrivate *priv = sms->priv;

  if (number) {
    if (!priv->got_subscriber_number) {
      priv->got_subscriber_number = TRUE;
      if (!priv->handling_sms) start_handling_sms (sms);
      if (priv->sms_store_open)
        g_signal_emit (sms, signals[STATUS_CHANGED], 0, PK_SMS_READY);
    }
  } else {
    if (priv->got_subscriber_number) {
      priv->got_subscriber_number = FALSE;
      moko_network_remove_listener (priv->network, MOKO_LISTENER (sms));
      priv->handling_sms = FALSE;
      if (priv->sms_store_open)
        g_signal_emit (sms, signals[STATUS_CHANGED], 0, PK_SMS_NOTREADY);
    }
  }
}*/

static void
moko_sms_set_property (GObject *object, guint property_id,
                       const GValue *value, GParamSpec *pspec)
{
  /*gchar *number;*/
  MokoSms *sms = MOKO_SMS (object);
  MokoSmsPrivate *priv = sms->priv;

  switch (property_id) {
    case PROP_NETWORK :
      if (priv->network) {
        stop_handling_sms (sms);
        g_object_unref (priv->network);
      }
      priv->network = g_value_dup_object (value);
      /*if (!moko_network_get_subscriber_number (priv->network, &number, NULL)) {
        subscriber_number_changed_cb (priv->network, NULL, MOKO_SMS (object));
      } else {
        subscriber_number_changed_cb (priv->network, number, MOKO_SMS (object));
      }
      g_signal_connect (priv->network, "subscriber_number_changed",
                        G_CALLBACK (subscriber_number_changed_cb), object);*/
      /* moko_network_add_listener happens in start_handling_sms */
      if ((!priv->sim_full) && (!priv->memory_full)) start_handling_sms (sms);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
moko_sms_dispose (GObject *object)
{
  MokoSms *sms;
  MokoSmsPrivate *priv;

  sms = MOKO_SMS (object);
  priv = sms->priv;
  
  if (priv->memory_idle) {
    g_source_remove (priv->memory_idle);
    priv->memory_idle = 0;
  }

  while (g_source_remove_by_user_data (object)) moko_notify_stop (priv->notify);
  
  if (priv->sms_store) {
    g_object_unref (priv->sms_store);
    priv->sms_store = NULL;
  }
  
  if (priv->last_msg) {
    g_object_unref (priv->last_msg);
    priv->last_msg = NULL;
  }

  G_OBJECT_CLASS (moko_sms_parent_class)->dispose (object);
}

static void
moko_sms_finalize (GObject *object)
{
  MokoSms *sms;
  MokoSmsPrivate *priv;
  
  sms = MOKO_SMS (object);
  priv = sms->priv;

  G_OBJECT_CLASS (moko_sms_parent_class)->finalize (object);
}

#include "moko-sms-glue.h"

static void
moko_sms_class_init (MokoSmsClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->get_property = moko_sms_get_property;
  obj_class->set_property = moko_sms_set_property;
  obj_class->finalize = moko_sms_finalize;
  obj_class->dispose = moko_sms_dispose;
  
  /* add class properties */
  g_object_class_install_property (obj_class,
                                   PROP_STATUS,
                                   g_param_spec_int (
                                   "status",
                                   "PhoneKitSmsStatus",
                                   "The current SMS status.",
                                   PK_SMS_NOTREADY,
                                   PK_SMS_READY,
                                   PK_SMS_NOTREADY,
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
  signals[STATUS_CHANGED] =
    g_signal_new ("status_changed", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoSmsClass, status_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__INT,
                  G_TYPE_NONE, 
                  1, G_TYPE_INT);

  signals[SIM_FULL] =
    g_signal_new ("sim_memory_state", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoSmsClass, memory_full),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 
                  1, G_TYPE_BOOLEAN);

  signals[MEMORY_FULL] =
    g_signal_new ("phone_memory_state", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoSmsClass, memory_full),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 
                  1, G_TYPE_BOOLEAN);

  g_type_class_add_private (obj_class, sizeof (MokoSmsPrivate));
  dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass), 
                                   &dbus_glib_moko_sms_object_info);
}

static void
on_incoming_sms (MokoListener *listener, struct lgsm_handle *handle,
                 const struct gsmd_sms_list *sms)
{
  gchar *message;

  MokoSms *moko_sms = MOKO_SMS (listener);
  MokoSmsPrivate *priv = moko_sms->priv;

  /* Return if we're not ready - this shouldn't happen as we don't listen to 
   * events when we're not ready...
   */
  if (!priv->handling_sms) return;

  /* Ignore voicemail notifications */
  if (sms->payload.is_voicemail) return;

  /* TODO: Verify type of message for journal (sent/received) - 
   *       Assuming received for now, as messages sent with phone-kit 
   *       will be marked already.
   */
  message = NULL;
  switch (sms->payload.coding_scheme) {
  case ALPHABET_DEFAULT :
    g_debug ("Decoding 7-bit ASCII message:");
    message = g_malloc0 (GSMD_SMS_DATA_MAXLEN);
    unpacking_7bit_character (&sms->payload, message);
    break;
  case ALPHABET_8BIT :
    /* TODO: Verify: Is this encoding just UTF-8? (it is on my Samsung phone) */
    g_debug ("Decoding UTF-8 message:");
    message = g_strdup (sms->payload.data);
    break;
  case ALPHABET_UCS2 :
    {
      gint i;
      gunichar2 *ucs2 = (gunichar2 *)sms->payload.data;

      g_debug ("Decoding UCS-2 message:");

      for (i = 0; i < sms->payload.length / 2; i++)
        ucs2[i] = GUINT16_FROM_BE(ucs2[i]);

      message = g_utf16_to_utf8 (ucs2,
                                 sms->payload.length, NULL, NULL, NULL);
      
      break;
    }
  }
  
  /* Store message in the journal */
  if (message) {
    struct lgsm_sms_delete sms_del;
    gchar *author, *own_number;
    JanaNote *note = jana_ecal_note_new ();
    
    g_debug ("Moving message to journal:\n\"%s\"", message);
    
    author = g_strconcat (((sms->addr.type & __GSMD_TOA_TON_MASK) ==
                          GSMD_TOA_TON_INTERNATIONAL) ? "+" : "",
                          sms->addr.number, NULL);
    jana_note_set_author (note, author);
    g_free (author);
    
    own_number = NULL;
    moko_network_get_subscriber_number (priv->network, &own_number, NULL);
    jana_note_set_recipient (note, own_number);
    g_free (own_number);
    
    jana_note_set_body (note, message);
    
    /* TODO: Set creation time from SMS timestamp */
    
    /* Add SMS to store */
    jana_store_add_component (priv->sms_store, JANA_COMPONENT (note));
    
    /* Delete SMS from internal storage */
    sms_del.index = sms->index;
    sms_del.delflg = LGSM_SMS_DELFLG_INDEX;
    lgsm_sms_delete (handle, &sms_del);
    
    g_free (message);
  }
}

typedef struct {
  MokoSms *moko_sms;
  gchar *ref;
} MokoSmsStatusReport;

static void
status_report_added_cb (JanaStoreView *view, GList *components,
                        MokoSmsStatusReport *sr)
{
  MokoSmsPrivate *priv = sr->moko_sms->priv;

  for (; components; components = components->next) {
    gchar *compref;
    JanaComponent *comp = JANA_COMPONENT (components->data);
    
    compref = jana_component_get_custom_prop (
      comp, "X-PHONEKIT-SMS-REF");
    if (compref && (strcmp (compref, sr->ref) == 0)) {
      jana_utils_component_remove_category (comp, "Sending");
      jana_utils_component_insert_category (comp, "Sent", 0);
      jana_store_modify_component (priv->sms_store, comp);
      g_debug ("Setting message status to confirmed sent");
    }
    g_free (compref);
  }
}

static void
status_report_progress_cb (JanaStoreView *view, gint percent,
                           MokoSmsStatusReport *sr)
{
  if (percent != 100) return;
  
  g_object_unref (view);
  
  g_object_unref (sr->moko_sms);
  g_free (sr->ref);
  g_slice_free (MokoSmsStatusReport, sr);
}

static void
on_incoming_ds (MokoListener *listener, struct lgsm_handle *handle,
                const struct gsmd_sms_list *sms)
{
  MokoSms *moko_sms = MOKO_SMS (listener);
  MokoSmsPrivate *priv = moko_sms->priv;

  g_debug ("Received sent SMS status report");
  if ((sms->payload.coding_scheme == TP_STATUS_RECEIVED_OK)/* &&
      (sms->stat == LGSM_SMS_STO_SENT)*/) {
    gchar *ref = g_strdup_printf ("%d", sms->index);
    JanaStoreView *view = jana_store_get_view (priv->sms_store);
    MokoSmsStatusReport *sr = g_slice_new (MokoSmsStatusReport);

    sr->moko_sms = g_object_ref (moko_sms);
    sr->ref = ref;
    
    g_debug ("Report signals success");
    jana_store_view_add_match (view, JANA_STORE_VIEW_CATEGORY, "Sending");
    g_signal_connect (view, "added",
                      G_CALLBACK (status_report_added_cb), sr);
    g_signal_connect (view, "progress",
                      G_CALLBACK (status_report_progress_cb), sr);
	jana_store_view_start (view);

  }
}

static void
on_send_sms (MokoListener *listener, struct lgsm_handle *handle,
             int result)
{
  MokoSms *sms = MOKO_SMS (listener);
  MokoSmsPrivate *priv = sms->priv;
  
  if (priv->last_msg) {
    gchar *uid = jana_component_get_uid (
      JANA_COMPONENT (priv->last_msg));
    
    if (result >= 0) {
      gchar *ref = g_strdup_printf ("%d", result);
      jana_component_set_custom_prop (JANA_COMPONENT (priv->last_msg),
                                      "X-PHONEKIT-SMS-REF", ref);
      g_free (ref);
      g_debug ("Sent message accepted");
    } else {
      g_debug ("Sent message rejected");
      jana_utils_component_remove_category (JANA_COMPONENT(priv->last_msg),
                                            "Sent");
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
  }
}

static void
on_error (MokoListener *listener, struct lgsm_handle *handle,
          int cme, int cms)
{
  MokoSmsPrivate *priv = ((MokoSms *)listener)->priv;

  if (cms == 322) {
    if (!priv->sim_full) {
      priv->sim_full = TRUE;
      g_signal_emit (listener, signals[SIM_FULL], 0, TRUE);
    }
  }
}

static void
listener_interface_init (gpointer g_iface, gpointer iface_data)
{
  MokoListenerInterface *iface = (MokoListenerInterface *)g_iface;
  
  iface->on_incoming_sms = on_incoming_sms;
  iface->on_incoming_ds = on_incoming_ds;
  iface->on_send_sms = on_send_sms;
  iface->on_error = on_error;
}

static gboolean
stop_notify_timeout (MokoSms *sms)
{
  MokoSmsPrivate *priv = sms->priv;
  moko_notify_stop (priv->notify);
  return FALSE;
}

static void
update_notification (MokoSms *sms, gboolean show)
{
  gchar *body;
  MokoSmsPrivate *priv = sms->priv;
  
  if (!priv->unread_uids) {
    notify_notification_close (priv->notification, NULL);
    return;
  }
  
  body = g_strdup_printf ("%d unread message(s)",
                          g_list_length (priv->unread_uids));
  g_object_set (G_OBJECT (priv->notification), "body", body, NULL);
  g_free (body);
  
  /* Show notification */
  if (show) {
    notify_notification_show (priv->notification, NULL);
    moko_notify_start (priv->notify);
    g_timeout_add (1000, (GSourceFunc)stop_notify_timeout, sms);
  }
}

static void
note_added_cb (JanaStoreView *store_view, GList *components, MokoSms *sms)
{
  MokoSmsPrivate *priv = sms->priv;
  gboolean update = FALSE;
  
  for (; components; components = components->next) {
    JanaComponent *comp = JANA_COMPONENT (components->data);
    
    if (!comp) continue;
    
    if ((!jana_utils_component_has_category (comp, "Read")) &&
	(!(jana_utils_component_has_category (comp, "Sending") ||
	 jana_utils_component_has_category (comp, "Sent")))) {
      gchar *uid = jana_component_get_uid (comp);
      priv->unread_uids = g_list_prepend (priv->unread_uids, uid);
      update = TRUE;
    }
  }
  
  /* TODO: Put this in an idle? */
  if (update) update_notification (sms, TRUE);
}

static void
note_modified_cb (JanaStoreView *store_view, GList *components, MokoSms *sms)
{
  MokoSmsPrivate *priv = sms->priv;
  gboolean update = FALSE;
  
  for (; components; components = components->next) {
    gchar *uid;
    GList *found;
    JanaComponent *comp = JANA_COMPONENT (components->data);
    
    if (!comp) continue;
    
    uid = jana_component_get_uid (comp);
    if ((found = g_list_find_custom (
         priv->unread_uids, uid, (GCompareFunc)strcmp))) {
      g_free (uid);
      if (jana_utils_component_has_category (comp, "Read")) {
        g_free (found->data);
        priv->unread_uids = g_list_delete_link (priv->unread_uids, found);
        update = TRUE;
      }
    } else if ((!jana_utils_component_has_category (comp, "Read")) &&
	(!(jana_utils_component_has_category (comp, "Sending") ||
	 jana_utils_component_has_category (comp, "Sent")))) {
      priv->unread_uids = g_list_prepend (priv->unread_uids, uid);
      update = TRUE;
    } else {
      g_free (uid);
    }
  }
  
  if (update) update_notification (sms, FALSE);
}

static void
note_removed_cb (JanaStoreView *store_view, GList *uids, MokoSms *sms)
{
  MokoSmsPrivate *priv = sms->priv;
  gboolean update = FALSE;
  
  for (; uids; uids = uids->next) {
    GList *found = g_list_find_custom (priv->unread_uids, uids->data,
                                       (GCompareFunc)strcmp);
    if (found) {
      g_free (found->data);
      priv->unread_uids = g_list_delete_link (priv->unread_uids, found);
      update = TRUE;
    }
  }
  
  if (update) update_notification (sms, FALSE);
}

static void
start_handling_sms (MokoSms *sms)
{
  MokoSmsPrivate *priv = sms->priv;
  struct lgsm_handle *handle;
  
  if (priv->handling_sms || (!priv->sms_store_open)/* ||
      (!priv->got_subscriber_number)*/) return;
  if (!moko_network_get_lgsm_handle (priv->network, &handle, NULL)) return;
  
  /* Add listener to MokoNetwork object */
  moko_network_add_listener (priv->network, MOKO_LISTENER (sms));

  /* List all messages to move to journal */
  lgsm_sms_list (handle, GSMD_SMS_ALL);
  
  priv->handling_sms = TRUE;
}

static void
sms_store_opened_cb (JanaStore *store, MokoSms *self)
{
  JanaStoreView *view;
  MokoSmsPrivate *priv = self->priv;
  priv->sms_store_open = TRUE;

  g_debug ("Sms store opened");

  /* Hook onto added/modified/removed signals for SMS notification */
  view = jana_store_get_view (store);
  g_signal_connect (view, "added", G_CALLBACK (note_added_cb), self);
  g_signal_connect (view, "modified", G_CALLBACK (note_modified_cb), self);
  g_signal_connect (view, "removed", G_CALLBACK (note_removed_cb), self);
  jana_store_view_start (view);

  if (!priv->handling_sms) start_handling_sms (self);
  /*if (priv->got_subscriber_number)*/
    g_signal_emit (self, signals[STATUS_CHANGED], 0, PK_SMS_READY);
}

static void
stop_handling_sms (MokoSms *sms)
{
  MokoSmsPrivate *priv = sms->priv;
  
  if (!priv->handling_sms) return;
  
  g_debug ("Closing sms store");
  
  moko_network_remove_listener (priv->network, MOKO_LISTENER (sms));
  
  g_object_unref (priv->sms_store);
  priv->sms_store = NULL;
  
  priv->sms_store_open = FALSE;
  priv->handling_sms = FALSE;
  
  g_signal_emit (sms, signals[STATUS_CHANGED], 0, PK_SMS_NOTREADY);
}

static void
open_sms_store (MokoSms *sms)
{
  MokoSmsPrivate *priv = sms->priv;
  
  if (priv->sms_store) return;
  
  g_debug ("Opening sms store");

  /* Get the SMS note store */
  priv->sms_store = jana_ecal_store_new (JANA_COMPONENT_NOTE);
  g_signal_connect (priv->sms_store, "opened",
                    G_CALLBACK (sms_store_opened_cb), sms);
  jana_store_open (priv->sms_store);
}

static gboolean
memory_check_idle (MokoSms *sms)
{
  struct statfs buf;

  MokoSmsPrivate *priv = sms->priv;
  
  statfs ("/", &buf);
  
  /* TODO: Is it reasonable to expect 4 megs/100 files free? */
  if (((buf.f_bfree * buf.f_bsize) < (1024*1024*4))/* ||
      (buf.f_ffree < 100)*/) {
    if (!priv->memory_full) {
      priv->memory_full = TRUE;
      g_signal_emit (sms, signals[MEMORY_FULL], 0, TRUE);
      if (priv->sms_store) {
        stop_handling_sms (sms);
      }
    }
  } else if (priv->memory_full) {
    priv->memory_full = FALSE;
    g_signal_emit (sms, signals[MEMORY_FULL], 0, FALSE);
    open_sms_store (sms);
  }
  
  return TRUE;
}

static void
moko_sms_init (MokoSms *sms)
{
  MokoSmsPrivate *priv;

  priv = sms->priv = MOKO_SMS_GET_PRIVATE (sms);
  priv->notification = notify_notification_new ("New SMS message",
                                                "",
                                                MOKO_STOCK_SMS_NEW,
                                                NULL);
  priv->notify = moko_notify_get_default ();

  memory_check_idle (sms);
  if (!priv->memory_full) open_sms_store (sms);
  
  priv->memory_idle = g_timeout_add_seconds (5, (GSourceFunc)
                                             memory_check_idle, sms);
}

MokoSms*
moko_sms_get_default (MokoNetwork *network)
{
  static MokoSms *sms = NULL;
  if (sms)
    return sms;

  sms = g_object_new (MOKO_TYPE_SMS, "network", network, NULL);

  return sms;
}

PhoneKitSmsStatus
moko_sms_get_status (MokoSms *sms)
{
  MokoSmsPrivate *priv = sms->priv;
  
  if (priv->sms_store_open/* && priv->got_subscriber_number*/)
    return PK_SMS_READY;
  else
    return PK_SMS_NOTREADY;
}

gboolean
moko_sms_send (MokoSms *self, const gchar *number,
               const gchar *message, gboolean report, gchar **uid,
               GError **error)
{
  PhoneKitNetworkStatus status;
  struct lgsm_handle *handle;
  MokoSmsPrivate *priv;
  struct lgsm_sms sms;
  gint msg_length, c;
  gboolean ascii;
  JanaNote *note;
  gchar *dialcode = NULL;
  gchar *sub_num = NULL;
  
  g_assert (self && number && message);
  priv = self->priv;

  if (!moko_network_get_lgsm_handle (priv->network, &handle, error))
    return FALSE;
  status = moko_network_get_status (priv->network);
  if (status < PK_NETWORK_REGISTERED_HOME) {
    if (error) *error = g_error_new (PHONE_KIT_NETWORK_ERROR,
                                     PK_NETWORK_ERROR_NOT_CONNECTED,
                                     "Not registered to a network");
    return FALSE;
  }
  
  if (!priv->sms_store_open) {
    *error = g_error_new (PHONE_KIT_SMS_ERROR, PK_SMS_ERROR_STORE_NOTOPEN,
                          "SMS store not opened");
    return FALSE;
  }
  
  moko_network_get_subscriber_number (priv->network, &sub_num, NULL);
  
  /* Ask for delivery report */
  sms.ask_ds = report ? 1 : 0;
  
  /* Set destination number */
  if (strlen (number) > GSMD_ADDR_MAXLEN + 1) {
    *error = g_error_new (PHONE_KIT_SMS_ERROR,
                          PK_SMS_ERROR_INVALID_NUMBER,
                          "Invalid number");
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
      *error = g_error_new (PHONE_KIT_SMS_ERROR, PK_SMS_ERROR_MSG_TOOLONG,
                            "Message too long");
      return FALSE;
  }
  if (ascii) {
    packing_7bit_character (message, &sms);
  } else {
    sms.alpha = ALPHABET_8BIT;
    strcpy ((gchar *)sms.data, message);
  }
  sms.length = msg_length;
  
  /* Send message */
  lgsm_sms_send (handle, &sms);
  
  /* Store sent message in journal */
  note = jana_ecal_note_new ();

  moko_network_get_country_code (priv->network, &dialcode, NULL);
  
  /*if ((number[0] == '0') && (number[1] != '0') && (dialcode)) {
    gchar *full_number = g_strconcat (dialcode, number + 1, NULL);
    jana_note_set_recipient (note, full_number);
    g_free (full_number);
  } else*/
    jana_note_set_recipient (note, number);
  if (sub_num) {
    jana_note_set_author (note, sub_num);
    g_free (sub_num);
  }
  g_free (dialcode);
  
  jana_note_set_body (note, message);
  if (report) {
    jana_component_set_categories (JANA_COMPONENT (note),
                                   (const gchar *[]){ "Sending", NULL});
  } else {
    jana_component_set_categories (JANA_COMPONENT (note),
                                   (const gchar *[]){ "Sent", NULL});
  }
  
  jana_store_add_component (priv->sms_store,
    JANA_COMPONENT (note));
  if (uid) *uid = jana_component_get_uid (JANA_COMPONENT (note));
  
  if (priv->last_msg) {
    g_warning ("Confirmation not received for last sent SMS, "
               "any delivery reports for this message will be lost.");
    g_object_unref (priv->last_msg);
  }
  priv->last_msg = note;
  
  return TRUE;
}

gboolean
moko_sms_get_memory_status (MokoSms *self, gboolean *sim_full,
                            gboolean *phone_full, GError **error)
{
  if (phone_full) *phone_full = FALSE;
  
  return TRUE;
}

