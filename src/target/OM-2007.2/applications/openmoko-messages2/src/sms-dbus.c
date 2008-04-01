/*
 *  openmoko-messages -- OpenMoko SMS Application
 *
 *  Authored by Chris Lord <chris@openedhand.com>
 *
 *  Copyright (C) 2008 OpenMoko Inc.
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

#include "sms.h"
#include "sms-dbus.h"
#include "sms-utils.h"
#include "sms-compose.h"
#include "sms-notes.h"
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

G_DEFINE_TYPE (SmsDbus, sms_dbus, G_TYPE_OBJECT)

#include "sms-dbus-glue.h"

static void
sms_dbus_class_init (SmsDbusClass *klass)
{
	dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass),
		&dbus_glib_sms_dbus_object_info);
}

static void
sms_dbus_init (SmsDbus *sms_dbus)
{
}

SmsDbus *
sms_dbus_new (SmsData *data)
{
	SmsDbus *sms_dbus = g_object_new (SMS_TYPE_DBUS, NULL);
	sms_dbus->priv = data;
	return sms_dbus;
}

typedef struct {
	SmsData *data;
	gchar *uid;
	gchar *number;
	gchar *message;
} SmsDbusData;

static void
free_data (SmsDbusData *data)
{
	g_free (data->uid);
	g_free (data->number);
	g_free (data->message);
	g_slice_free (SmsDbusData, data);
}

static gboolean
view_messages_idle (SmsDbusData *data)
{
	if (data->data->book_seq_complete) {
		if (sms_select_contact (data->data, data->uid))
			sms_notes_refresh (data->data);
	
		free_data (data);
		return FALSE;
	} else
		return TRUE;
}

static gboolean
send_message_idle (SmsDbusData *data)
{
	if (data->data->book_seq_complete) {
		sms_select_contact (data->data, data->uid);
		sms_compose_refresh (data->data,data->number);

		free_data (data);
		return FALSE;
	} else
		return TRUE;
}

gboolean
sms_dbus_view_messages (SmsDbus *sms_dbus, const gchar *uid, GError **error)
{
	EContact *contact;
	
	if (e_book_get_contact (sms_dbus->priv->ebook, uid, &contact, error)) {
		SmsDbusData *data;
		
		g_object_unref (contact);
		
		data = g_slice_new0 (SmsDbusData);
		data->data = sms_dbus->priv;
		data->uid = g_strdup (uid);
		
		g_idle_add ((GSourceFunc)view_messages_idle, data);
		
		return TRUE;
	} else
		return FALSE;
}

gboolean
sms_dbus_send_message (SmsDbus *sms_dbus, const gchar *uid, const gchar *number,
		       const gchar *message, GError **error)
{
	SmsDbusData *data;
	
	data = g_slice_new0 (SmsDbusData);
	data->data = sms_dbus->priv;
	data->uid = g_strdup (uid);
	data->number = g_strdup (number);
	data->message = g_strdup (message);
	g_idle_add ((GSourceFunc)send_message_idle, data);

	return TRUE;

}

gboolean
sms_dbus_append_recipient (SmsDbus *sms_dbus, const gchar *uid,
			   const gchar *number, GError **error)
{
	EContact *contact;
	
	if (e_book_get_contact (sms_dbus->priv->ebook, uid, &contact, error)) {
		/* TODO: Add multiple recipient support and implement this */
		g_object_unref (contact);
		return TRUE;
	} else
		return FALSE;
}

