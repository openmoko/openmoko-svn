/*
 *  openmoko-messages -- OpenMoko SMS Application
 *
 *  Authored by Chris Lord <chris@openedhand.com>
 *
 *  Copyright (C) 2007 OpenMoko Inc.
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
#include "sms-contacts.h"
#include "sms-notes.h"
#include "sms-compose.h"
#include <moko-stock.h>

static void
notebook_add_page_with_icon (GtkWidget *notebook, GtkWidget *child,
			     const gchar *icon_name, int padding)
{
	GtkWidget *icon = gtk_image_new_from_icon_name (icon_name,
		GTK_ICON_SIZE_LARGE_TOOLBAR);
	GtkWidget *align = gtk_alignment_new (0.5, 0.5, 1, 1);

	gtk_alignment_set_padding (GTK_ALIGNMENT (align), padding,
		padding, padding, padding);
	gtk_container_add (GTK_CONTAINER (align), icon);
	gtk_widget_show_all (align);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), child, align);
	gtk_container_child_set (GTK_CONTAINER (notebook), child,
		"tab-expand", TRUE, NULL);
}

static void
new_clicked_cb (GtkToolButton *button, SmsData *data)
{
	gtk_notebook_set_current_page (
		GTK_NOTEBOOK (data->notebook), SMS_PAGE_COMPOSE);
}

int
main (int argc, char **argv)
{
	SmsData data;
	DBusGConnection *connection;
	GtkWidget *vbox, *toolbar;
	GError *error = NULL;
	
	gtk_init (&argc, &argv);
	moko_stock_register ();
	
	/* Get SMS dbus proxy */
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (!connection) {
		g_warning ("Failed to get dbus connection: %s", error->message);
		g_error_free (error);
		data.sms_proxy = NULL;
	} else {
		data.sms_proxy = dbus_g_proxy_new_for_name (connection,
			"org.openmoko.PhoneKit", "/org/openmoko/PhoneKit/Sms",
			"org.openmoko.PhoneKit.Sms");
	}

	data.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (data.window), "Messages");
	g_signal_connect (data.window, "delete-event",
		G_CALLBACK (gtk_main_quit), NULL);
	
	/* Create toolbar */
	toolbar = gtk_toolbar_new ();
	
	/* New button */
	data.new_button = gtk_tool_button_new_from_stock (MOKO_STOCK_SMS_NEW);
	gtk_tool_item_set_expand (data.new_button, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data.new_button, 0);
	/*gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
		gtk_separator_tool_item_new (), 1);*/
	g_signal_connect (data.new_button, "clicked",
		G_CALLBACK (new_clicked_cb), &data);
	
	/* Delete all button */
	data.delete_all_button = gtk_tool_button_new_from_stock (
		GTK_STOCK_MISSING_IMAGE);
	gtk_tool_item_set_expand (data.delete_all_button, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data.delete_all_button, 1);
	/*gtk_toolbar_insert (GTK_TOOLBAR (toolbar),
		gtk_separator_tool_item_new (), 3);*/
	
	/* Delete button */
	data.delete_button = gtk_tool_button_new_from_stock (
		GTK_STOCK_DELETE);
	gtk_tool_item_set_expand (data.delete_button, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data.delete_button, 2);
	
	/* Create notebook */
	data.notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (data.notebook), GTK_POS_BOTTOM);
	
	/* Add contact page */
	notebook_add_page_with_icon (data.notebook,
		sms_contacts_page_new (&data), GTK_STOCK_INDEX, 6);

	/* Add message view page */
	notebook_add_page_with_icon (data.notebook,
		sms_notes_page_new (&data), GTK_STOCK_FILE, 6);

	/* Add message compose page */
	notebook_add_page_with_icon (data.notebook,
		sms_compose_page_new (&data), GTK_STOCK_EDIT, 6);
	
	/* Pack and show */
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), data.notebook, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (data.window), vbox);
	
#if 0
	/* Force theme settings */
	g_object_set (gtk_settings_get_default (),
		"gtk-theme-name", "openmoko-standard-2", /* Moko */
		"gtk-icon-theme-name", "openmoko-standard",
		"gtk-xft-dpi", 285 * 1024,
		"gtk-font-name", "Sans 6",
		NULL);
#endif
	gtk_window_set_default_size (GTK_WINDOW (data.window), 480, 600);

	gtk_widget_show_all (data.window);
	
	gtk_main ();
	
	return 0;
}

