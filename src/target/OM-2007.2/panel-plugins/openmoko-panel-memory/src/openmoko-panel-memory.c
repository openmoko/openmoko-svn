/*  openmoko-panel-memory.c
 *
 *  Copyright (C) 2008 OpenMoko Inc.
 *
 *  Authored by Chris Lord <chris@openedhand.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 */

#include <gtk/gtk.h>
#include <libmokopanelui2/moko-panel-applet.h>
#include <libnotify/notify.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

typedef struct {
	MokoPanelApplet *applet;
	GtkWidget *image;

	DBusGProxy *sms_proxy;
	NotifyNotification *notification;
	
	guint blink_idle;
} MemoryAppletData;

static gboolean
blink_idle (MemoryAppletData *data)
{
	g_object_set (G_OBJECT (data->applet), "visible",
		!GTK_WIDGET_VISIBLE (data->applet), NULL);
	
	return TRUE;
}

static void
memory_full_cb (DBusGProxy *proxy, gboolean sim_full, gboolean phone_full,
		MemoryAppletData *data)
{
	if (sim_full || phone_full) {
		const gchar *message;
		
		gtk_widget_show (GTK_WIDGET (data->applet));
		data->blink_idle = g_timeout_add_seconds (1,
			(GSourceFunc)blink_idle, data);
		
		if (sim_full && phone_full) {
			message = "Phone and SIM memory full";
		} else if (sim_full) {
			message = "SIM memory full";
		} else {
			message = "Phone memory full";
		}
		g_object_set (G_OBJECT (data->notification),
			"body", message, NULL);
		
		notify_notification_show (data->notification, NULL);
	} else {
		g_source_remove (data->blink_idle);
		gtk_widget_hide (GTK_WIDGET (data->applet));
		notify_notification_close (data->notification, NULL);
	}
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char* id, GtkOrientation orientation)
{
	DBusGConnection *connection;
	MemoryAppletData *data;
	
	GError *error = NULL;
	
	if (!(connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error))) {
		g_warning ("Failed to get dbus connection: %s", error->message);
		g_error_free (error);
		return NULL;
	}
	
	data = g_slice_new0 (MemoryAppletData);
	
	data->sms_proxy = dbus_g_proxy_new_for_name (connection,
		"org.openmoko.PhoneKit", "/org/openmoko/PhoneKit/Sms",
		"org.openmoko.PhoneKit.Sms");
	
	dbus_g_proxy_add_signal (data->sms_proxy, "MemoryFull",
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal (data->sms_proxy, "MemoryFull",
		G_CALLBACK (memory_full_cb), data, NULL);
	
	notify_init ("openmoko-panel-memory");
	data->notification = notify_notification_new ("Memory full", "",
		GTK_STOCK_DIALOG_WARNING, NULL);
	
	data->applet = MOKO_PANEL_APPLET(moko_panel_applet_new());
	
	data->image = gtk_image_new_from_stock (
		GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
	gtk_widget_show (data->image);
	moko_panel_applet_set_widget (data->applet, data->image);
	
	return GTK_WIDGET (data->applet);
}

