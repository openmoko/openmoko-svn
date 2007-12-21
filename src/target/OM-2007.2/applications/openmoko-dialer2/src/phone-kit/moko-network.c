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

typedef struct {
  GSource source;
  GPollFD pollfd;
  struct lgsm_handle *handle;
} MokoNetworkSource;

struct _MokoNetworkPrivate
{
  gchar                     *own_number;
  gchar                     *network_name;
  gchar                     *network_number;
  gchar                     *imsi;

  /* gsmd connection variables */
  struct lgsm_handle        *handle;
  MokoNetworkSource         *source;
  int                       lac;
  
  /* Registration variables */
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
      
      /* Retrieve details when we switch location/type */
      if ((priv->registered != type) || (priv->lac != lac)) {
        priv->lac = lac;
        
        /* Retrieve operator name */
        lgsm_oper_get (handle);
        
        /* Retrieve operator list to get current country code */
        lgsm_opers_get (handle);
        
        /* Retrieve IMSI to get home country code */
        lgsm_get_imsi (handle);
      }
      
      break;
    default:
      g_warning ("Unhandled register event type = %d\n", type);
   };

  if (priv->registered != type) {
    priv->registered = type;
    g_signal_emit (listener, signals[STATUS_CHANGED], 0,
                   moko_network_get_status (MOKO_NETWORK (listener)));
  }
}

static void
on_pin_requested (MokoListener *listener, struct lgsm_handle *handle,
                  int type)
{
  MokoNetworkPrivate *priv;
  gchar *pin;

  g_return_if_fail (MOKO_IS_NETWORK (listener));
  priv = MOKO_NETWORK (listener)->priv;

  g_debug ("Pin Requested");
  
  pin = get_pin_from_user ();
  if (!pin)
    return;
  
  lgsm_pin (handle, 1, pin, NULL);
  g_free (pin);
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
on_network_list (MokoListener *listener, struct lgsm_handle *handle,
                 const struct gsmd_msg_oper *opers)
{
  MokoNetwork *network = MOKO_NETWORK (listener);
  MokoNetworkPrivate *priv = network->priv;

  for (; !opers->is_last; opers++) {
    if (opers->stat == GSMD_OPER_CURRENT) {
      g_free (priv->network_number);
      priv->network_number = g_strndup (opers->opname_num,
                                        sizeof(opers->opname_num));
      break;
    }
  }
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

/* GObject functions */
static void
moko_network_dispose (GObject *object)
{
  MokoNetwork *network;
  MokoNetworkPrivate *priv;

  network = MOKO_NETWORK (object);
  priv = network->priv;

  if (priv->handle) {
    lgsm_exit (priv->handle);
    priv->handle = NULL;
  }

  if (priv->source) {
    g_source_destroy ((GSource *)priv->source);
    priv->source = NULL;
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
  iface->on_network_list = on_network_list;
  iface->on_imsi = on_imsi;
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
      g_warning ("Delivery status report not in-line, left unhandled");
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
    for (l = priv->listeners; l; l = l->next) {
      moko_listener_on_pin_requested (MOKO_LISTENER (l->data), priv->handle,
                                      aux->u.pin.type);
    }
    break;
  case GSMD_EVT_OUT_STATUS :
    for (l = priv->listeners; l; l = l->next) {
      moko_listener_on_call_progress (MOKO_LISTENER (l->data), priv->handle,
                                      aux->u.call_status.prog);
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
      moko_listener_on_incoming_sms (MOKO_LISTENER (l->data), priv->handle,
                                     sms_list);
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
      for (l = priv->listeners; l; l = l->next) {
        moko_listener_on_network_name (MOKO_LISTENER (l->data),
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
      for (l = priv->listeners; l; l = l->next)
      {
        moko_listener_on_imsi (MOKO_LISTENER (l->data), priv->handle,
                               (const gchar *)gmh + sizeof (*gmh));
      }
      break;
    case GSMD_PHONE_POWERUP:
      if (*intresult == 0)
      {
        /* phone has been powered on successfully */
        g_debug ("Phone powered on");
        /* Register with network */
        priv->registered = GSMD_NETREG_UNREG;
        lgsm_netreg_register (priv->handle, "");
        
        /* Get phone number */
        lgsm_get_subscriber_num (priv->handle);
      }
      break;
    
    default :
      return -EINVAL;
  }
  
  return 0;
}

static int
pin_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
  int result = *(int *) gmh->data;
  static int attempt = 0;
  
  /* store the number of pin attempts */
  attempt++;
  
  /* must not do more than three attempts */
  if (attempt > 3)
  {
    char *msg = "Maximum number of PIN attempts reached";
    g_debug (msg);
    display_pin_error (msg);
  }
 
  if (result)
  {
    gchar *pin = NULL;
    char *msg = g_strdup_printf ("PIN error: %i", result);
    
    g_debug (msg);
    display_pin_error (msg);
    g_free (msg);
    
    pin = get_pin_from_user ();
    if (!pin) return 0;
    lgsm_pin (lh, 1, pin, NULL);
    g_free (pin);
  }
  else
  {
    /* PIN accepted, so let's make sure the phone is now powered on */
    g_debug ("PIN accepted!");
    lgsm_phone_power (lh, 1);
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
  MokoNetworkSource *self = (MokoNetworkSource *)source;
  return self->pollfd.revents & G_IO_IN;
}

static gboolean 
connection_source_dispatch (GSource *source, GSourceFunc callback,
                            gpointer data)
{
  char buf[1025];
  int size;

  MokoNetworkSource *self = (MokoNetworkSource *)source;

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
network_init_gsmd (MokoNetwork *network)
{
  static GSourceFuncs funcs = {
    connection_source_prepare,
    connection_source_check,
    connection_source_dispatch,
    NULL,
  };

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
  lgsm_register_handler (priv->handle, GSMD_MSG_NETWORK, net_msghandler);
  lgsm_register_handler (priv->handle, GSMD_MSG_PHONE, phone_msghandler);
  lgsm_register_handler (priv->handle, GSMD_MSG_SMS, sms_msghandler);
  lgsm_register_handler (priv->handle, GSMD_MSG_PIN, pin_msghandler);

  /* Power the gsm modem up - this should trigger a PIN requiest if needed */
  lgsm_phone_power (priv->handle, 1);

  /* Start polling for events */
  priv->source = (MokoNetworkSource *)
    g_source_new (&funcs, sizeof (MokoNetworkSource));
  priv->source->handle = priv->handle;
  priv->source->pollfd.fd = lgsm_fd (priv->handle);
  priv->source->pollfd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
  priv->source->pollfd.revents = 0;
  g_source_add_poll ((GSource*)priv->source, &priv->source->pollfd);
  g_source_attach ((GSource*)priv->source, NULL);
}

static void
moko_network_init (MokoNetwork *network)
{
  MokoNetworkPrivate *priv;

  priv = network->priv = MOKO_NETWORK_GET_PRIVATE (network);

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

