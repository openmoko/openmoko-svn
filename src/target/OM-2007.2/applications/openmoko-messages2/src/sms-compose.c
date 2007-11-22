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

#include "sms-compose.h"
#include "sms-utils.h"
#include <moko-stock.h>

static gboolean hidden = TRUE;

static void
page_shown (SmsData *data)
{
	GtkTreeModel *model;
	EContact *contact;
	GdkPixbuf *photo;
	gchar *string;
	gint i;
	
	gboolean set = FALSE;
	
	if (!data->sms_proxy)
		gtk_widget_set_sensitive (GTK_WIDGET (data->new_button), FALSE);
	gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (data->new_button),
		MOKO_STOCK_MAIL_SEND);
	gtk_widget_grab_focus (data->sms_textview);

	/* Empty numbers combo */
	model = gtk_combo_box_get_model (GTK_COMBO_BOX (data->number_combo));
	while (gtk_tree_model_iter_n_children (model, NULL))
		gtk_combo_box_remove_text (GTK_COMBO_BOX (
			data->number_combo), 0);

	if (!(contact = sms_get_selected_contact (data))) {
		gtk_image_set_from_icon_name (GTK_IMAGE (data->contact_image),
			"stock_person", GTK_ICON_SIZE_DIALOG);
		gtk_label_set_markup (GTK_LABEL (data->contact_label),
			"<big>Unknown</big>");
		return;
	}
	
	/* Fill contact photo */
	photo = sms_contact_load_photo (contact);
	if (photo) {
		gtk_image_set_from_pixbuf (GTK_IMAGE (
			data->contact_image), photo);
		g_object_unref (photo);
	} else {
		gtk_image_set_from_icon_name (GTK_IMAGE (data->contact_image),
			"stock_person", GTK_ICON_SIZE_DIALOG);
	}
	
	/* Fill contact label */
	string = g_strconcat ("<big>", e_contact_get_const (
		contact, E_CONTACT_FULL_NAME), "</big>", NULL);
	gtk_label_set_markup (GTK_LABEL (data->contact_label), string);
	g_free (string);
	
	/* Fill number combo */
	for (i = E_CONTACT_FIRST_PHONE_ID; i <= E_CONTACT_LAST_PHONE_ID; i++) {
		const gchar *number = e_contact_get_const (
			contact, (EContactField)i);

		if (!number) continue;
		
		if (((i == E_CONTACT_PHONE_MOBILE) ||
		     (i == E_CONTACT_PHONE_PRIMARY)) && (!set)) {
			gtk_entry_set_text (GTK_ENTRY (GTK_BIN (
				data->number_combo)->child), number);
			set = TRUE;
		}
		gtk_combo_box_append_text (GTK_COMBO_BOX (data->number_combo),
			number);
	}
	if (!set) {
		gtk_entry_set_text (GTK_ENTRY (GTK_BIN (
			data->number_combo)->child),
			gtk_combo_box_get_active_text (
				GTK_COMBO_BOX (data->number_combo)));
	}
	
	g_object_unref (contact);
}

static void
page_hidden (SmsData *data)
{
	gtk_widget_set_sensitive (GTK_WIDGET (data->new_button), TRUE);
	gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (data->new_button),
		MOKO_STOCK_SMS_NEW);
}

static void
notify_visible_cb (GObject *gobject, GParamSpec *arg1, SmsData *data)
{
	if ((!hidden) && (!GTK_WIDGET_VISIBLE (gobject))) {
		hidden = TRUE;
		page_hidden (data);
	}
}

static gboolean
visibility_notify_event_cb (GtkWidget *widget, GdkEventVisibility *event,
			    SmsData *data)
{
	if (((event->state == GDK_VISIBILITY_PARTIAL) ||
	     (event->state == GDK_VISIBILITY_UNOBSCURED)) && (hidden)) {
		hidden = FALSE;
		page_shown (data);
	}/* else if ((event->state == GDK_VISIBILITY_FULLY_OBSCURED) &&
		   (!hidden)) {
		hidden = TRUE;
	}*/
	
	return FALSE;
}

static void
unmap_cb (GtkWidget *widget, SmsData *data)
{
	if (!hidden) {
		hidden = TRUE;
		page_hidden (data);
	}
}

static void
send_clicked_cb (GtkButton *button, SmsData *data)
{
	GtkTextIter start, end;
	GtkTextBuffer *buffer;
	const gchar *number;
	EContact *contact;
	gchar *message;
	
	if (hidden) return;
	
	/* TODO: Spawn an error dialog or something */
	if (!(contact = sms_get_selected_contact (data))) return;
	
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (
		data->new_button), FALSE);

	number = gtk_entry_get_text (GTK_ENTRY (
		GTK_BIN (data->number_combo)->child));
	
	if (number && (number[0] != '\0')) {
		GError *error = NULL;
		buffer = gtk_text_view_get_buffer (
			GTK_TEXT_VIEW (data->sms_textview));
		gtk_text_buffer_get_start_iter (buffer, &start);
		gtk_text_buffer_get_end_iter (buffer, &end);
		message = gtk_text_buffer_get_text (
			buffer, &start, &end, FALSE);
		
		if (message[0] != '\0') {
			g_debug ("Sending message '%s' to %s", message, number);
			if (!dbus_g_proxy_call (data->sms_proxy, "Send", &error,
			     G_TYPE_STRING, number, G_TYPE_STRING, message,
			     G_TYPE_INVALID, G_TYPE_STRING, NULL,
			     G_TYPE_INVALID)) {
				g_warning ("Error sending message: %s",
					error->message);
				g_error_free (error);
			}
		} else {
			/* TODO: Error dialog for empty message */
		}
		
		g_free (message);
	} else {
		/* TODO: Error dialog for empty number */
	}
	
	g_object_unref (contact);

	gtk_notebook_set_current_page (
		GTK_NOTEBOOK (data->notebook), SMS_PAGE_NOTES);
}

GtkWidget *
sms_compose_page_new (SmsData *data)
{
	GtkWidget *vbox, *frame, *contact_table;
	
	/* Connect to new/send button clicked */
	g_signal_connect (data->new_button, "clicked",
		G_CALLBACK (send_clicked_cb), data);
	
	/* Create contact info display/number entry */
	contact_table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (contact_table), 6);
	gtk_table_set_row_spacings (GTK_TABLE (contact_table), 6);
	gtk_container_set_border_width (GTK_CONTAINER (contact_table), 6);
	data->contact_image = gtk_image_new ();
	data->contact_label = gtk_label_new (NULL);
	gtk_label_set_use_markup (GTK_LABEL (data->contact_label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (data->contact_label), 0, 0.5);
	data->number_combo = gtk_combo_box_entry_new_text ();
	gtk_table_attach (GTK_TABLE (contact_table), data->contact_image,
		0, 1, 0, 2, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach (GTK_TABLE (contact_table), data->contact_label,
		1, 2, 0, 1, GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach (GTK_TABLE (contact_table), data->number_combo,
		1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
	
	/* Create sms entry bits */
	data->sms_textview = gtk_text_view_new ();
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (data->sms_textview),
		GTK_WRAP_WORD_CHAR);
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), data->sms_textview);
	
	/* Pack widgets */
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), contact_table, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

	/* Add events for detecting whether the page has been hidden/shown */
	gtk_widget_add_events (data->sms_textview, GDK_VISIBILITY_NOTIFY_MASK);
	g_signal_connect (data->sms_textview, "visibility-notify-event",
		G_CALLBACK (visibility_notify_event_cb), data);
	g_signal_connect (data->sms_textview, "notify::visible",
		G_CALLBACK (notify_visible_cb), data);
	g_signal_connect (vbox, "unmap",
		G_CALLBACK (unmap_cb), data);

	gtk_widget_show_all (vbox);
	
	return vbox;
}
