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

#ifndef SMS_DBUS_H
#define SMS_DBUS_H

#include "sms.h"
#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define SMS_TYPE_DBUS (sms_dbus_get_type ())

#define SMS_DBUS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        SMS_TYPE_DBUS, SmsDbus))

#define SMS_DBUS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        SMS_TYPE_DBUS, SmsDbusClass))

#define SMS_IS_DBUS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
        SMS_TYPE_DBUS))

#define SMS_IS_DBUS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
        SMS_TYPE_DBUS))

#define SMS_DBUS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
        SMS_TYPE_DBUS, SmsDbusClass))

typedef struct _SmsDbus SmsDbus;
typedef struct _SmsDbusClass SmsDbusClass;

struct _SmsDbus
{
	GObject		parent;
	SmsData		*priv;
};

struct _SmsDbusClass
{
	GObjectClass	parent_class;
};

SmsDbus * sms_dbus_new (SmsData *data);

/* dbus methods */
gboolean sms_dbus_view_messages (SmsDbus *sms_dbus, const gchar *uid,
				 GError **error);
gboolean sms_dbus_send_message (SmsDbus *sms_dbus, const gchar *uid,
				const gchar *number, const gchar *message,
				GError **error);
gboolean sms_dbus_append_recipient (SmsDbus *sms_dbus, const gchar *uid,
				    const gchar *number, GError **error);

G_END_DECLS

#endif /* SMS_DBUS_H */

