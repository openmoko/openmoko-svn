/*
 *  moko-network; a GObject wrapper for phone-kit that exports method and
 *  signals over dbus relating to GSM network status and properties
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

#include "moko-network.h"
#include "moko-pin.h"

#include "moko-mcc-dc.h"

static void
listener_interface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (MokoNetwork, moko_network, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (MOKO_TYPE_LISTENER,
                                                listener_interface_init))

#define MOKO_NETWORK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_NETWORK, MokoNetworkPrivate))

#define RETRY_MAX 5
#define RETRY_DELAY 60

enum {
  PROP_STATUS = 1,
};

enum
{
  STATUS_CHANGED,
  SUBSCRIBER_CHANGED,
  PROVIDER_CHANGED,
  
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0, };

struct _MokoNetworkPrivate
{
  gchar                     *own_number;
  gchar                     *network_name;
  gchar                     *network_number;
  gchar                     *imsi;
  gchar                     *imei;
  guint                     retry_register;
  gint                      retry_register_n;
  guint                     retry_oper;
  gint                      retry_oper_n;
  guint                     retry_opern;
  gint                      retry_opern_n;
  guint                     retry_imsi;
  gint                      retry_imsi_n;
  
  gint                      pin_attempts;
  enum gsmd_pin_type        pin_type;

  /* gsmd connection variables */
  GIOChannel                *channel;
  struct lgsm_handle        *handle;
  int                       lac;
  
  /* Registration variables */
  gboolean                  powered;
  enum lgsm_netreg_state    registered;
  gboolean                  pin_requested;
  
  /* List of listeners */
  GList                     *listeners;
};

static void network_init_gsmd (MokoNetwork *network);

static const gchar *moko_network_cc_from_mcc (gchar *mcc);

static void
moko_network_get_property (GObject *object, guint property_id,
                           GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
    case PROP_STATUS :
      g_value_set_int (value, moko_network_get_status (MOKO_NETWORK (object)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

/* Callbacks to retry retrieving of operator/sim details after registration */
static gboolean
retry_oper_get (MokoNetwork *network)
{
  g_debug ("Retrying operator name retrieval (%d)", network->priv->retry_oper_n);
  lgsm_oper_get (network->priv->handle);
  return (--network->priv->retry_oper_n) ? TRUE : FALSE;
}

static gboolean
retry_opern_get (MokoNetwork *network)
{
  g_debug ("Retrying operator number retrieval (%d)",
           network->priv->retry_opern_n);
  lgsm_oper_n_get (network->priv->handle);
  return (--network->priv->retry_opern_n) ? TRUE : FALSE;
}

static gboolean
retry_get_imsi (MokoNetwork *network)
{
  g_debug ("Retrying imsi retrieval (%d)", network->priv->retry_imsi_n);
  lgsm_get_imsi (network->priv->handle);
  return (--network->priv->retry_imsi_n) ? TRUE : FALSE;
}

static gboolean
retry_register (MokoNetwork *network)
{
  g_debug ("Retrying network registration");
  lgsm_netreg_register (network->priv->handle, "\0     ");
  return (--network->priv->retry_register_n) ? TRUE : FALSE;
}

static void
stop_retrying (MokoNetwork *network)
{
  /* Stop trying to get details */
  if (network->priv->retry_oper) {
    g_source_remove (network->priv->retry_oper);
    network->priv->retry_oper = 0;
  }
  if (network->priv->retry_opern) {
    g_source_remove (network->priv->retry_opern);
    network->priv->retry_opern = 0;
  }
  if (network->priv->retry_imsi) {
    g_source_remove (network->priv->retry_imsi);
    network->priv->retry_imsi = 0;
  }
}

static void
stop_retrying_registration (MokoNetwork *network)
{
  if (network->priv->retry_register) {
    g_source_remove (network->priv->retry_register);
    network->priv->retry_register = 0;
    network->priv->retry_register_n = RETRY_MAX;
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
  MokoNetworkPrivate *priv;
  
  g_return_if_fail (MOKO_IS_NETWORK (listener));
  priv = MOKO_NETWORK (listener)->priv;

  switch (type)
  {
    case GSMD_NETREG_UNREG:
      /* Start searching for network */
      /* Note: Operator name is a 6 character string, thus: */
      if (priv->registered != type) {
        if (!priv->retry_register) {
          g_debug ("Registering to network");
          lgsm_netreg_register (handle, "\0     ");
          priv->retry_register = g_timeout_add_seconds (RETRY_DELAY,
                                                    (GSourceFunc)retry_register,
                                                    listener);
        }
      }
    case GSMD_NETREG_UNREG_BUSY:
      g_debug ("Searching for network");
      
      /* Clear operator location */
      priv->lac = 0;
      
      /* Stop trying to get details */
      stop_retrying (MOKO_NETWORK (listener));
      
      break;
    case GSMD_NETREG_DENIED:
      /* This may be a pin issue*/

      /* Stop trying to get details */
      stop_retrying (MOKO_NETWORK (listener));
      break;
    case GSMD_NETREG_REG_HOME:
    case GSMD_NETREG_REG_ROAMING:
      g_debug ("Network registered: LocationAreaCode: %x. CellID: %x.", lac, cell);
      
      stop_retrying_registration ((MokoNetwork *)listener);
      
      /* Retrieve details when we switch location/type */
      if ((priv->registered != type) || (priv->lac != lac)) {
        priv->lac = lac;
        
        /* Retrieve operator name */
        lgsm_oper_get (handle);
        
        /* Retrieve IMSI to get home country code */
        lgsm_get_imsi (handle);
        
        /* Add a time-out in case retrieval fails,
         * retry every RETRY_DELAY seconds */
        stop_retrying (MOKO_NETWORK (listener));
        if (priv->retry_oper_n)
          priv->retry_oper = g_timeout_add_seconds (RETRY_DELAY,
                                                    (GSourceFunc)retry_oper_get,
                                                    listener);
        if (priv->retry_imsi_n)
          priv->retry_imsi = g_timeout_add_seconds (RETRY_DELAY,
                                                    (GSourceFunc)retry_get_imsi,
                                                    listener);

	if (type == GSMD_NETREG_REG_ROAMING) {
        /* Retrieve operator number to get current country code */
        lgsm_oper_n_get (handle);
        
        if (priv->retry_opern_n)
          priv->retry_opern = g_timeout_add_seconds (RETRY_DELAY,
                                                     (GSourceFunc)
                                                     retry_opern_get,
                                                     listener);
        }
      }
      
      break;
    default:
      g_warning ("Unhandled register event type = %d\n", type);
   };

  if (priv->registered != type) {
    priv->registered = type;
    g_signal_emit (listener, signals[STATUS_CHANGED], 0,
                   moko_network_get_status (MOKO_NETWORK (listener)));
    
    if ((priv->registered != GSMD_NETREG_REG_HOME) &&
        (priv->registered != GSMD_NETREG_REG_ROAMING) &&
        (priv->registered != GSMD_NETREG_UNREG) ) {
      /* Unset operator name on disconnect */
      moko_listener_on_network_name (listener, handle, NULL);
    }
  }
}

static void
on_pin_requested (MokoListener *listener, struct lgsm_handle *handle,
                  enum gsmd_pin_type type, int error)
{
  MokoNetworkPrivate *priv;
  gchar *pin;

  g_return_if_fail (MOKO_IS_NETWORK (listener));
  priv = MOKO_NETWORK (listener)->priv;

  g_debug ("Pin Requested");
  
  /* Stop the registering time-out if we receive a pin request */
  stop_retrying_registration ((MokoNetwork *)listener);
  
  if (priv->pin_attempts < 3) {
    const char *message;
    
    switch (type) {
      case GSMD_PIN_READY :
        g_debug ("Received GSMD_PIN_READY, ignoring");
        return;
      
      default :
      case GSMD_PIN_SIM_PIN :         /* SIM PIN */
        message = "Enter PIN";
        break;
      case GSMD_PIN_SIM_PUK :         /* SIM PUK */
        message = "Enter PUK";
        break;
      case GSMD_PIN_PH_SIM_PIN :      /* phone-to-SIM password */
        message = "Enter SIM PIN";
        break;
      case GSMD_PIN_PH_FSIM_PIN :     /* phone-to-very-first SIM password */
        message = "Enter new SIM PIN";
        break;
      case GSMD_PIN_PH_FSIM_PUK :     /* phone-to-very-first SIM PUK password */
        message = "Enter new SIM PUK";
        break;
      case GSMD_PIN_SIM_PIN2 :        /* SIM PIN2 */
        message = "Enter PIN2";
        break;
      case GSMD_PIN_SIM_PUK2 :        /* SIM PUK2 */
        message = "Enter PUK2";
        break;
      case GSMD_PIN_PH_NET_PIN :      /* network personalisation password */
        message = "Enter network PIN";
        break;
      case GSMD_PIN_PH_NET_PUK :      /* network personalisation PUK */
        message = "Enter network PUK";
        break;
      case GSMD_PIN_PH_NETSUB_PIN :   /* network subset personalisation PIN */
        message = "Enter network subset PIN";
        break;
      case GSMD_PIN_PH_NETSUB_PUK :   /* network subset personalisation PUK */
        message = "Enter network subset PUK";
        break;
      case GSMD_PIN_PH_SP_PIN :       /* service provider personalisation PIN */
        message = "Enter service provider PIN";
        break;
      case GSMD_PIN_PH_SP_PUK :       /* service provider personalisation PUK */
        message = "Enter service provider PUK";
        break;
      case GSMD_PIN_PH_CORP_PIN :     /* corporate personalisation PIN */
        message = "Enter corporate PIN";
        break;
      case GSMD_PIN_PH_CORP_PUK :     /* corporate personalisation PUK */
        message = "Enter corporate PUK";
        break;
    }
    
    pin = get_pin_from_user (message);
    if (!pin) {
      g_debug ("No PIN entered, not requestiong");
      return;
    }
  
    lgsm_pin (handle, 1, pin, NULL);
    g_free (pin);
    
    priv->pin_attempts ++;
  } else {
    display_pin_error ("Maximum number of PIN attempts reached.");
  }
}

static void
on_subscriber_number (MokoListener *listener, struct lgsm_handle *handle,
                      const gchar *number)
{
  MokoNetwork *network = MOKO_NETWORK (listener);
  MokoNetworkPrivate *priv = network->priv;

  g_free (priv->own_number);
  
  if ((number) && (number[0] == '0') && (priv->imsi))
    priv->own_number = g_strconcat (moko_network_cc_from_mcc (priv->imsi),
                                    number + 1, NULL);
  else
    priv->own_number = g_strdup (number);

  g_signal_emit (listener, signals[SUBSCRIBER_CHANGED], 0, priv->own_number);
}

static void
on_network_name (MokoListener *listener, struct lgsm_handle *handle,
                 const gchar *name)
{
  MokoNetwork *network = MOKO_NETWORK (listener);
  MokoNetworkPrivate *priv = network->priv;

  if ((!name) && (!priv->network_name))
    return;
  
  g_free (priv->network_name);
  if (name && name[0]) priv->network_name = g_strdup (name);
  else priv->network_name = NULL;
  
  g_signal_emit (listener, signals[PROVIDER_CHANGED], 0, priv->network_name);
}

static void
on_network_number (MokoListener *listener, struct lgsm_handle *handle,
                   const gchar *number)
{
  MokoNetwork *network = MOKO_NETWORK (listener);
  MokoNetworkPrivate *priv = network->priv;

  g_free (priv->network_number);
  priv->network_number = g_strdup (number);
}

static void
on_imsi (MokoListener *listener, struct lgsm_handle *handle,
         const gchar *imsi)
{
  MokoNetwork *network = MOKO_NETWORK (listener);
  MokoNetworkPrivate *priv = network->priv;

  g_free (priv->imsi);
  priv->imsi = g_strdup (imsi);

  /* Get phone number */
  lgsm_get_subscriber_num (handle);
}

/* XXX we don't want GTK+ here */
#include <gtk/gtk.h>
static void
on_imei (MokoListener *listener, struct lgsm_handle *handle,
         const gchar *imei)
{
  MokoNetwork *network = MOKO_NETWORK (listener);
  MokoNetworkPrivate *priv = network->priv;
  GtkWidget *dlg;
  
  g_free (priv->imei);
  priv->imei = g_strdup (imei);

  dlg = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                imei);
  gtk_window_set_title (GTK_WINDOW (dlg), "IMEI");
  gtk_widget_show_all (dlg);
  
  g_signal_connect (GTK_DIALOG (dlg), "response",
  			G_CALLBACK (gtk_widget_destroy), NULL);
}


/* GObject functions */
static void
moko_network_dispose (GObject *object)
{
  MokoNetwork *network;
  MokoNetworkPrivate *priv;

  network = MOKO_NETWORK (object);
  priv = network->priv;

  stop_retrying (MOKO_NETWORK (object));

  if (priv->channel) {
    g_io_channel_shutdown (priv->channel, FALSE, NULL);
    g_io_channel_unref (priv->channel);
    priv->channel = NULL;
  }

  if (priv->handle) {
    lgsm_exit (priv->handle);
    priv->handle = NULL;
  }

  G_OBJECT_CLASS (moko_network_parent_class)->dispose (object);
}

static void
moko_network_finalize (GObject *object)
{
  MokoNetwork *network;
  MokoNetworkPrivate *priv;
  
  network = MOKO_NETWORK (object);
  priv = network->priv;

  g_free (priv->own_number);
  g_free (priv->network_name);
  g_free (priv->network_number);
  g_free (priv->imsi);
  
  G_OBJECT_CLASS (moko_network_parent_class)->finalize (object);
}

#include "moko-network-glue.h"

static void
moko_network_class_init (MokoNetworkClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->get_property = moko_network_get_property;
  obj_class->finalize = moko_network_finalize;
  obj_class->dispose = moko_network_dispose;
  
  /* add class properties */
  g_object_class_install_property (obj_class,
                                   PROP_STATUS,
                                   g_param_spec_int (
                                   "status",
                                   "PhoneKitNetworkStatus",
                                   "The current network status.",
                                   PK_NETWORK_UNREGISTERED,
                                   PK_NETWORK_REGISTERED_ROAMING,
                                   PK_NETWORK_UNREGISTERED,
                                   G_PARAM_READABLE));

  /* add class signals */
  signals[STATUS_CHANGED] =
    g_signal_new ("status_changed", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoNetworkClass, status_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__INT,
                  G_TYPE_NONE, 
                  1, G_TYPE_INT);

  signals[SUBSCRIBER_CHANGED] =
    g_signal_new ("subscriber_number_changed", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoNetworkClass, subscriber_number_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 
                  1, G_TYPE_STRING);

  signals[PROVIDER_CHANGED] =
    g_signal_new ("provider_changed", 
                  G_TYPE_FROM_CLASS (obj_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (MokoNetworkClass, provider_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 
                  1, G_TYPE_STRING);

  g_type_class_add_private (obj_class, sizeof (MokoNetworkPrivate)); 
  dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass), 
                                   &dbus_glib_moko_network_object_info);
}

static void
listener_interface_init (gpointer g_iface, gpointer iface_data)
{
  MokoListenerInterface *iface = (MokoListenerInterface *)g_iface;
  
  iface->on_network_registered = on_network_registered;
  iface->on_pin_requested = on_pin_requested;
  iface->on_network_name = on_network_name;
  iface->on_network_number = on_network_number;
  iface->on_imsi = on_imsi;
  iface->on_imei = on_imei;
  iface->on_subscriber_number = on_subscriber_number;
}

static int 
gsmd_eventhandler (struct lgsm_handle *lh, int evt_type,
                   struct gsmd_evt_auxdata *aux)
{
  GList *l;
  MokoNetwork *network = moko_network_get_default ();
  MokoNetworkPrivate *priv = network->priv;
  
  switch (evt_type) {
  case GSMD_EVT_IN_CALL :
    for (l = priv->listeners; l; l = l->next) {
      moko_listener_on_incoming_call (MOKO_LISTENER (l->data), priv->handle,
                                      aux->u.call.type);
    }
    break;
  case GSMD_EVT_IN_SMS : /* Incoming SMS */
    g_debug ("Received incoming SMS");
    if (aux->u.sms.inlined) {
      struct gsmd_sms_list * sms = (struct gsmd_sms_list *)aux->data;
      g_debug ("Message inline");
      for (l = priv->listeners; l; l = l->next) {
        moko_listener_on_incoming_sms (MOKO_LISTENER (l->data), priv->handle,
                                       sms);
      }
    } else {
      g_debug ("Message stored on SIM, reading...");
      lgsm_sms_read (lh, aux->u.sms.index);
    }
    break;
  case GSMD_EVT_IN_DS : /* SMS status report */
    if (aux->u.ds.inlined) {
      struct gsmd_sms_list *sms = (struct gsmd_sms_list *) aux->data;
      
      for (l = priv->listeners; l; l = l->next) {
        moko_listener_on_incoming_ds (MOKO_LISTENER (l->data), priv->handle,
                                      sms);
      }
    } else {
      g_debug ("Delivery status report stored on SIM, reading...");
      lgsm_sms_read (lh, aux->u.sms.index);
    }
    break;
  case GSMD_EVT_IN_CLIP :
    for (l = priv->listeners; l; l = l->next) {
      moko_listener_on_incoming_clip (MOKO_LISTENER (l->data), priv->handle,
                                      aux->u.clip.addr.number);
    }
    break;
  case GSMD_EVT_NETREG :
    for (l = priv->listeners; l; l = l->next) {
      moko_listener_on_network_registered (MOKO_LISTENER (l->data),
                                           priv->handle, aux->u.netreg.state,
                                           aux->u.netreg.lac, aux->u.netreg.ci);
    }
    break;
  case GSMD_EVT_PIN :
    priv->pin_type = aux->u.pin.type;
    for (l = priv->listeners; l; l = l->next) {
      moko_listener_on_pin_requested (MOKO_LISTENER (l->data), priv->handle,
                                      aux->u.pin.type, 0);
    }
    break;
  case GSMD_EVT_OUT_STATUS :
    for (l = priv->listeners; l; l = l->next) {
      moko_listener_on_call_progress (MOKO_LISTENER (l->data), priv->handle,
                                      aux->u.call_status.prog);
    }
    break;
  case GSMD_EVT_IN_ERROR :
    for (l = priv->listeners; l; l = l->next) {
      moko_listener_on_error (MOKO_LISTENER (l->data), priv->handle,
                              aux->u.cme_err.number,
                              aux->u.cms_err.number);
    }
    break;
  default :
    g_warning ("Unhandled gsmd event (%d)", evt_type);
  }
  
  return 0;
}

static int
sms_msghandler (struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
  GList *l;
  MokoNetwork *network = moko_network_get_default ();
  MokoNetworkPrivate *priv = network->priv;

  if (gmh->msg_subtype == GSMD_SMS_SEND) {
    int *result = (int *) ((void *) gmh + sizeof(*gmh));
    for (l = priv->listeners; l; l = l->next) {
      moko_listener_on_send_sms (MOKO_LISTENER (l->data), priv->handle,
                                 *result);
    }
  } else if ((gmh->msg_subtype == GSMD_SMS_LIST) ||
             (gmh->msg_subtype == GSMD_SMS_READ)) {
    struct gsmd_sms_list *sms_list = (struct gsmd_sms_list *)
                                     ((void *) gmh + sizeof(*gmh));
    for (l = priv->listeners; l; l = l->next) {
      switch (sms_list->payload.tp_mti) {
        case GSMD_SMS_TP_MTI_DELIVER :
        case GSMD_SMS_TP_MTI_SUBMIT :
          moko_listener_on_incoming_sms (MOKO_LISTENER (l->data), priv->handle,
                                         sms_list);
          break;
        case GSMD_SMS_TP_MTI_STATUS_REPORT :
          moko_listener_on_incoming_ds (MOKO_LISTENER (l->data), priv->handle,
                                        sms_list);
          break;
        default :
          break;
      }
    }
  } else {
    return -EINVAL;
  }
  
  return 0;
}

static int
net_msghandler (struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
  GList *l;
  MokoNetwork *network = moko_network_get_default ();
  MokoNetworkPrivate *priv = network->priv;

  const char *oper = (char *) gmh + sizeof(*gmh);
  const struct gsmd_own_number *num = (struct gsmd_own_number *)
                                      ((void *) gmh + sizeof(*gmh));
  const struct gsmd_msg_oper *opers = (struct gsmd_msg_oper *)
                                      ((void *) gmh + sizeof(*gmh));

  switch (gmh->msg_subtype) {
    case GSMD_NETWORK_GET_NUMBER :
      for (l = priv->listeners; l; l = l->next) {
        moko_listener_on_subscriber_number (MOKO_LISTENER (l->data),
                                            priv->handle, num->addr.number);
      }
      break;
    case GSMD_NETWORK_OPER_GET :
      if (priv->retry_oper) {
        g_source_remove (priv->retry_oper);
        priv->retry_oper = 0;
        priv->retry_oper_n = RETRY_MAX;
      }
      for (l = priv->listeners; l; l = l->next) {
        moko_listener_on_network_name (MOKO_LISTENER (l->data),
                                       priv->handle, oper);
      }
      break;
    case GSMD_NETWORK_OPER_N_GET :
      if (priv->retry_opern) {
        g_source_remove (priv->retry_opern);
        priv->retry_opern = 0;
        priv->retry_opern_n = RETRY_MAX;
      }
      for (l = priv->listeners; l; l = l->next) {
        moko_listener_on_network_number (MOKO_LISTENER (l->data),
                                         priv->handle, oper);
      }
      break;
    case GSMD_NETWORK_OPER_LIST :
      for (l = priv->listeners; l; l = l->next) {
        moko_listener_on_network_list (MOKO_LISTENER (l->data),
                                       priv->handle, opers);
      }
      break;
    default :
      return -EINVAL;
  }
  
  return 0;
}

static int
phone_msghandler (struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
  GList *l;
  MokoNetwork *network = moko_network_get_default ();
  MokoNetworkPrivate *priv = network->priv;
  int *intresult = (void *)gmh + sizeof(*gmh);

  switch (gmh->msg_subtype)
  {
    case GSMD_PHONE_GET_IMSI:
      if (priv->retry_imsi) {
        g_source_remove (priv->retry_imsi);
        priv->retry_imsi = 0;
        priv->retry_imsi_n = RETRY_MAX;
      }
      for (l = priv->listeners; l; l = l->next) {
        moko_listener_on_imsi (MOKO_LISTENER (l->data), priv->handle,
                               (const gchar *)gmh + sizeof (*gmh));
      }
      break;
    case GSMD_PHONE_POWERUP:
      if (*intresult == 0)
      {
        /* phone has been powered on successfully */
        g_debug ("Phone powered on");

        priv->powered = TRUE;
        g_signal_emit (network, signals[STATUS_CHANGED], 0,
                       moko_network_get_status (network));

        /* Register with network */
        priv->registered = GSMD_NETREG_UNREG;
        lgsm_netreg_register (priv->handle, "");
        
        /* Get phone number */
        lgsm_get_subscriber_num (priv->handle);
      }
      break;
    
    case GSMD_PHONE_GET_SERIAL:
      for (l = priv->listeners; l; l = l->next)
        moko_listener_on_imei (MOKO_LISTENER (l->data), priv->handle,
                               (const gchar *)gmh + sizeof (*gmh));
      break;
    default :
      return -EINVAL;
  }
  
  return 0;
}

static int
pin_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
  GList *l;

  MokoNetwork *network = moko_network_get_default ();
  MokoNetworkPrivate *priv = network->priv;
  
  int result = *(int *) gmh->data;
 
  if (result) {
    for (l = priv->listeners; l; l = l->next) {
      moko_listener_on_pin_requested (MOKO_LISTENER (l->data), priv->handle,
                                      priv->pin_type, result);
    }
  } else {
    /* PIN accepted, so let's make sure the phone is now powered on */
    g_debug ("PIN accepted!");
    lgsm_phone_power (lh, 1);
  }

  return 0;
}

static int
pb_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
  GList *l;
  MokoNetwork *network = moko_network_get_default ();
  MokoNetworkPrivate *priv = network->priv;
  struct gsmd_phonebooks *gps;

  switch (gmh->msg_subtype) {
    case GSMD_PHONEBOOK_READRG:
      gps = (struct gsmd_phonebooks *) ((char *)gmh + sizeof(*gmh));
      for (l = priv->listeners; l; l = l->next) {
	moko_listener_on_read_phonebook (MOKO_LISTENER (l->data),
	    priv->handle, gps);
      }
      break;
    default:
      g_warning ("Unhandled phonebook event type = %d\n", gmh->msg_subtype);
      break;
   };

  return 0;
}

static gboolean
io_func (GIOChannel *source, GIOCondition condition, MokoNetwork *self)
{
  gchar buf[1025];
  gsize length;

  MokoNetworkPrivate *priv = self->priv;
  GError *error = NULL;
  
  switch (condition) {
    case G_IO_PRI :
    case G_IO_IN :
      if (g_io_channel_read_chars (source, buf, sizeof (buf), &length, &error)
          == G_IO_STATUS_NORMAL) {
        lgsm_handle_packet (priv->handle, buf, length);
      } else {
        g_warning ("Error reading from source: %s", error->message);
        g_error_free (error);
      }
      break;

    case G_IO_ERR :
    case G_IO_NVAL :
      g_warning ("Encountered an error, stopping IO watch");
      return FALSE;
    
    case G_IO_HUP :
      g_warning ("Gsmd hung up - TODO: Reconnect/restart gsmd?");
      return FALSE;
    
    case G_IO_OUT :
      break;

    default :
      g_warning ("Unhandled IO condition, bailing");
      return FALSE;
  }

  return TRUE;
}

static void
network_init_gsmd (MokoNetwork *network)
{
  MokoNetworkPrivate *priv;
  
  priv = network->priv;
  
  /* Add ourselves as an event listener */
  /* TODO: Split this file into two classes, MokoBase, MokoNetwork? */
  moko_network_add_listener (network, MOKO_LISTENER (network));

  /* Get a gsmd handle */
  if (!(priv->handle = lgsm_init (LGSMD_DEVICE_GSMD))) {
    g_warning ("Error connecting to gsmd");
    return;
  }

  /* Add event handlers */
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_CALL, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_CLIP, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_SMS, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_DS, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_NETREG, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_OUT_STATUS, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_OUT_STATUS, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_PIN, gsmd_eventhandler);
  lgsm_evt_handler_register (priv->handle, GSMD_EVT_IN_ERROR, gsmd_eventhandler);
  lgsm_register_handler (priv->handle, GSMD_MSG_NETWORK, net_msghandler);
  lgsm_register_handler (priv->handle, GSMD_MSG_PHONE, phone_msghandler);
  lgsm_register_handler (priv->handle, GSMD_MSG_SMS, sms_msghandler);
  lgsm_register_handler (priv->handle, GSMD_MSG_PIN, pin_msghandler);
  lgsm_register_handler (priv->handle, GSMD_MSG_PHONEBOOK, pb_msghandler);

  /* Power the gsm modem up - this should trigger a PIN requiest if needed */
  lgsm_phone_power (priv->handle, 1);

  /* Start polling for events */
  priv->channel = g_io_channel_unix_new (lgsm_fd (priv->handle));
  g_io_channel_set_encoding (priv->channel, NULL, NULL);
  g_io_channel_set_buffered (priv->channel, FALSE);
  g_io_add_watch (priv->channel, G_IO_IN | G_IO_ERR | G_IO_NVAL | G_IO_HUP,
                  (GIOFunc)io_func, network);
}

static void
moko_network_init (MokoNetwork *network)
{
  MokoNetworkPrivate *priv;

  priv = network->priv = MOKO_NETWORK_GET_PRIVATE (network);
  
  priv->retry_register_n = RETRY_MAX;
  priv->retry_oper_n = RETRY_MAX;
  priv->retry_opern_n = RETRY_MAX;
  priv->retry_imsi_n = RETRY_MAX;

  network_init_gsmd (network);
}

MokoNetwork*
moko_network_get_default (void)
{
  static MokoNetwork *network = NULL;
  if (network)
    return network;

  network = g_object_new (MOKO_TYPE_NETWORK, NULL);

  return network;
}

static gboolean
moko_network_check_registration (MokoNetwork *self, GError **error)
{
  MokoNetworkPrivate *priv = self->priv;
  
  if ((priv->registered != GSMD_NETREG_REG_HOME) &&
      (priv->registered != GSMD_NETREG_REG_ROAMING)) {
    if (error) *error = g_error_new (PHONE_KIT_NETWORK_ERROR,
                                     PK_NETWORK_ERROR_NOT_CONNECTED,
                                     "Not registered to a network");
    return FALSE;
  }
  
  return TRUE;
}

static const gchar *
moko_network_cc_from_mcc (gchar *mcc)
{
  gint i;
  for (i = 0; mcc_to_dc[i][0]; i++) {
    if (strncmp (mcc, mcc_to_dc[i][0], 3) == 0) {
      return mcc_to_dc[i][1];
    }
  }
  
  return NULL;
}

PhoneKitNetworkStatus
moko_network_get_status (MokoNetwork *network)
{
  MokoNetworkPrivate *priv = network->priv;
  
  if (!priv->powered)
    return PK_NETWORK_POWERDOWN;

  switch (priv->registered) {
    default:
    case GSMD_NETREG_UNREG:
    case GSMD_NETREG_UNREG_BUSY:
      return PK_NETWORK_UNREGISTERED;
    case GSMD_NETREG_DENIED:
      return PK_NETWORK_DENIED;
    case GSMD_NETREG_REG_HOME:
      return PK_NETWORK_REGISTERED_HOME;
    case GSMD_NETREG_REG_ROAMING:
      return PK_NETWORK_REGISTERED_ROAMING;
  }
}

gboolean
moko_network_get_lgsm_handle (MokoNetwork *network, struct lgsm_handle **handle,
                              GError **error)
{
  MokoNetworkPrivate *priv = network->priv;
  
  if (!priv->handle) network_init_gsmd (network);

  if (priv->handle) {
    if (handle) *handle = priv->handle;
    return TRUE;
  } else {
    if (error) *error = g_error_new (PHONE_KIT_NETWORK_ERROR,
                                     PK_NETWORK_ERROR_GSMD,
                                     "Failed to connect to gsmd");
    return FALSE;
  }
}

void
moko_network_add_listener (MokoNetwork *network, MokoListener *listener)
{
  MokoNetworkPrivate *priv = network->priv;
  
  priv->listeners = g_list_prepend (priv->listeners, listener);
}

void
moko_network_remove_listener (MokoNetwork *network, MokoListener *listener)
{
  MokoNetworkPrivate *priv = network->priv;
  
  priv->listeners = g_list_remove (priv->listeners, listener);
}

/* DBus functions */
gboolean
moko_network_get_provider_name (MokoNetwork *self, gchar **name, GError **error)
{
  MokoNetworkPrivate *priv;
  
  if (!moko_network_get_lgsm_handle (self, NULL, error)) return FALSE;
  if (!moko_network_check_registration (self, error)) return FALSE;
  priv = self->priv;
  
  if (!priv->network_name) {
    if (error) *error = g_error_new (PHONE_KIT_NETWORK_ERROR,
                                     PK_NETWORK_ERROR_NO_PROVIDER_NAME,
                                     "Unable to retrieve provider name");
    return FALSE;
  }

  if (name) *name = g_strdup (priv->network_name);
  return TRUE;
}

gboolean
moko_network_get_subscriber_number (MokoNetwork *self, gchar **number,
                                    GError **error)
{
  MokoNetworkPrivate *priv;
  
  if (!moko_network_get_lgsm_handle (self, NULL, error)) return FALSE;
  priv = self->priv;
  
  if (!priv->own_number) {
    if (error) *error = g_error_new (PHONE_KIT_NETWORK_ERROR,
                                     PK_NETWORK_ERROR_NO_SUBSCRIBER_NUM,
                                     "Unable to retrieve subscriber number");
    return FALSE;
  }
  
  if (number) *number = g_strdup (priv->own_number);
  return TRUE;
}

gboolean
moko_network_get_country_code (MokoNetwork *self, gchar **dial_code,
                               GError **error)
{
  MokoNetworkPrivate *priv;
  
  if (!moko_network_get_lgsm_handle (self, NULL, error)) return FALSE;
  if (!moko_network_check_registration (self, error)) return FALSE;
  priv = self->priv;
  
  if (priv->registered == GSMD_NETREG_REG_HOME) {
    return moko_network_get_home_country_code (self, dial_code, error);
  }
  
  if (!priv->network_number) {
    if (error) *error = g_error_new (PHONE_KIT_NETWORK_ERROR,
                                     PK_NETWORK_ERROR_NO_PROVIDER_NUM,
                                     "Unable to retrieve provider number");
    return FALSE;
  }
  
  if (dial_code)
    *dial_code = g_strdup (moko_network_cc_from_mcc (priv->network_number));
  
  return TRUE;
}

gboolean
moko_network_get_home_country_code (MokoNetwork *self, gchar **dial_code,
                                    GError **error)
{
  MokoNetworkPrivate *priv;
  
  if (!moko_network_get_lgsm_handle (self, NULL, error)) return FALSE;
  if (!moko_network_check_registration (self, error)) return FALSE;
  priv = self->priv;
  
  if (!priv->network_number) {
    if (error) *error = g_error_new (PHONE_KIT_NETWORK_ERROR,
                                     PK_NETWORK_ERROR_NO_IMSI,
                                     "Unable to retrieve IMSI");
    return FALSE;
  }
  
  if (dial_code)
    *dial_code = g_strdup (moko_network_cc_from_mcc (priv->imsi));
  
  return TRUE;
}

gboolean
moko_network_get_imsi (MokoNetwork *self, gchar **imsi, GError **error)
{
  MokoNetworkPrivate *priv;
  
  if (!moko_network_get_lgsm_handle (self, NULL, error)) return FALSE;
  if (!moko_network_check_registration (self, error)) return FALSE;
  priv = self->priv;
  
  if (!priv->imsi) {
    if (error) *error = g_error_new (PHONE_KIT_NETWORK_ERROR,
                                     PK_NETWORK_ERROR_NO_IMSI,
                                     "Unable to retrieve IMSI");
    return FALSE;
  }

  if (imsi)
    *imsi = g_strdup (priv->imsi);

  return TRUE;

}
