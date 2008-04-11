/*
 *  moko-listener; An interface for listening to libgsmd events
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

#include "moko-listener.h"

static void
moko_listener_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
moko_listener_get_type (void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (MokoListenerInterface),
			moko_listener_base_init,   /* base_init */
			NULL,

		};
		type = g_type_register_static (G_TYPE_INTERFACE,
			"MokoListener", &info, 0);
	}
	return type;
}

void
moko_listener_on_network_registered (MokoListener *listener,
                                     struct lgsm_handle *handle,
                                     int type,
                                     int lac,
                                     int cell)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_network_registered)
    interface->on_network_registered (listener, handle, type, lac, cell);
}

void
moko_listener_on_pin_requested (MokoListener *listener,
                                 struct lgsm_handle *handle,
                                 enum gsmd_pin_type type,
                                 int error)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_pin_requested)
    interface->on_pin_requested (listener, handle, type, error);
}

void
moko_listener_on_network_name (MokoListener *listener,
                               struct lgsm_handle *handle,
                               const gchar *name)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_network_name)
    interface->on_network_name (listener, handle, name);
}

void
moko_listener_on_network_number (MokoListener *listener,
                               struct lgsm_handle *handle,
                               const gchar *number)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_network_number)
    interface->on_network_number (listener, handle, number);
}

void
moko_listener_on_network_list (MokoListener *listener,
                               struct lgsm_handle *handle,
                               const struct gsmd_msg_oper *opers)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_network_list)
    interface->on_network_list (listener, handle, opers);
}

void
moko_listener_on_imsi (MokoListener *listener,
                       struct lgsm_handle *handle,
                       const gchar *imsi)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_imsi)
    interface->on_imsi (listener, handle, imsi);
}

void
moko_listener_on_imei (MokoListener *listener,
                       struct lgsm_handle *handle,
                       const gchar *imei)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_imei)
    interface->on_imei (listener, handle, imei);
}

void
moko_listener_on_subscriber_number (MokoListener *listener,
                                    struct lgsm_handle *handle,
                                    const gchar *number)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_subscriber_number)
    interface->on_subscriber_number (listener, handle, number);
}

void
moko_listener_on_incoming_call (MokoListener *listener,
                                struct lgsm_handle *handle,
                                int type)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_incoming_call)
    interface->on_incoming_call (listener, handle, type);
}

void
moko_listener_on_incoming_clip (MokoListener *listener,
                                struct lgsm_handle *handle,
                                const gchar *number)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_incoming_clip)
    interface->on_incoming_clip (listener, handle, number);
}

void
moko_listener_on_call_progress (MokoListener *listener,
                                struct lgsm_handle *handle, int type)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_call_progress)
    interface->on_call_progress (listener, handle, type);
}

void
moko_listener_on_incoming_sms (MokoListener *listener,
                               struct lgsm_handle *handle,
                               const struct gsmd_sms_list *sms)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_incoming_sms)
    interface->on_incoming_sms (listener, handle, sms);
}

void
moko_listener_on_incoming_ds (MokoListener *listener,
                              struct lgsm_handle *handle,
                              const struct gsmd_sms_list *sms)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_incoming_ds)
    interface->on_incoming_ds (listener, handle, sms);
}

void
moko_listener_on_send_sms (MokoListener *listener,
                           struct lgsm_handle *handle,
                           int result)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_send_sms)
    interface->on_send_sms (listener, handle, result);
}

void
moko_listener_on_read_phonebook (MokoListener *listener,
                                 struct lgsm_handle *handle,
                                 struct gsmd_phonebooks *gps)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_read_phonebook)
    interface->on_read_phonebook (listener, handle, gps);
}

void
moko_listener_on_error (MokoListener *listener,
                        struct lgsm_handle *handle,
                        int cme, int cms)
{
  MokoListenerInterface *interface = MOKO_LISTENER_GET_INTERFACE (listener);
  if (interface->on_error)
    interface->on_error (listener, handle, cme, cms);
}

