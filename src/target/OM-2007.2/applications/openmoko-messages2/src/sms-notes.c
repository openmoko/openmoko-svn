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

#include "sms-notes.h"
#include "sms-contacts.h"
#include <libjana-ecal/jana-ecal.h>
#include <libmokoui2/moko-finger-scroll.h>
#include <libmokoui2/moko-search-bar.h>
#include <libebook/e-book.h>
#include <string.h>

static GdkColor alt_color;
static gboolean hidden = TRUE;
static gboolean open = FALSE;

static EContact *
get_selected_contact (SmsData *data)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	EContact *contact;
	GtkTreeIter iter;
	gchar *uid;
	
	GError *error = NULL;
	
	selection = gtk_tree_view_get_selection (
		GTK_TREE_VIEW (data->contacts_treeview));
	
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return NULL;
	gtk_tree_model_get (model, &iter, COL_UID, &uid,
		COL_ICON, &data->author_icon, -1);
	
	if (!e_book_get_contact (data->ebook, uid, &contact, &error)) {
		g_warning ("Error retrieving contact: %s", error->message);
		g_error_free (error);
		contact = NULL;
	}
	
	g_free (uid);
	
	return contact;
}

static void
page_shown (SmsData *data)
{
	JanaStoreView *store_view;
	gint i;

	gboolean found_match = FALSE;
	EContact *contact = NULL;
	
	if (!open) return;
	
	if (!(contact = get_selected_contact (data))) return;
	
	store_view = jana_store_get_view (data->notes);
	for (i = E_CONTACT_FIRST_PHONE_ID; i <= E_CONTACT_LAST_PHONE_ID; i++) {
		const gchar *number = e_contact_get_const (
			contact, (EContactField)i);
		if (!number) continue;
		
		jana_store_view_add_match (store_view,
			JANA_STORE_VIEW_AUTHOR, number);
		jana_store_view_add_match (store_view,
			JANA_STORE_VIEW_RECIPIENT, number);
		found_match = TRUE;
	}
	
	if (found_match) {
		jana_gtk_note_store_set_view (JANA_GTK_NOTE_STORE (
			data->note_store), store_view);
		jana_store_view_start (store_view);
	}
	g_object_unref (store_view);

	g_object_unref (contact);
}

static void
page_hidden (SmsData *data)
{
	jana_gtk_note_store_set_view (JANA_GTK_NOTE_STORE (
		data->note_store), NULL);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (
		data->new_button), FALSE);
	if (data->recipient_icon) {
		g_object_unref (data->recipient_icon);
		data->recipient_icon = NULL;
	}
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
new_toggled_cb (GtkToggleToolButton *button, SmsData *data)
{
	gboolean active = gtk_toggle_tool_button_get_active (button);

	g_object_set (data->sms_hbox, "visible", active, NULL);
	if (active) {
		gtk_notebook_set_current_page (GTK_NOTEBOOK (
			data->notebook), SMS_PAGE_NOTES);
		gtk_widget_grab_focus (data->sms_textview);
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
	
	/* TODO: Spawn an error dialog or something */
	if (!(contact = get_selected_contact (data))) return;
	
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (
		data->new_button), FALSE);

	/* Try getting mobile and primary number first, before looping through 
	 * all numbers.
	 */
	number = NULL;
	if (!(number = e_contact_get_const (contact, E_CONTACT_PHONE_MOBILE)) &&
	    !(number = e_contact_get_const (contact, E_CONTACT_PHONE_PRIMARY))){
		gint i;
		for (i = E_CONTACT_FIRST_PHONE_ID;
		     i <= E_CONTACT_LAST_PHONE_ID; i++) {
			number = e_contact_get_const (
				contact, (EContactField)i);
			if (number) break;
		}
	}
	
	if (number) {
		GError *error = NULL;
		buffer = gtk_text_view_get_buffer (
			GTK_TEXT_VIEW (data->sms_textview));
		gtk_text_buffer_get_start_iter (buffer, &start);
		gtk_text_buffer_get_end_iter (buffer, &end);
		message = gtk_text_buffer_get_text (
			buffer, &start, &end, FALSE);
		
		g_debug ("Sending message '%s' to %s", message, number);
		if (!dbus_g_proxy_call (data->sms_proxy, "Send", &error,
			G_TYPE_STRING, number, G_TYPE_STRING, message,
			G_TYPE_INVALID, G_TYPE_STRING, NULL, G_TYPE_INVALID)) {
			g_debug ("Error sending message: %s", error->message);
			g_error_free (error);
		}
		
		g_free (message);
	}
	
	g_object_unref (contact);
}

static void sms_notes_data_func (GtkTreeViewColumn *tree_column,
				 GtkCellRenderer *cell,
				 GtkTreeModel *model,
				 GtkTreeIter *iter,
				 SmsData *data)
{
	gchar *author, *recipient, *body, **categories;
	JanaTime *created, *modified;
	gboolean outgoing = FALSE;
	gint i;
	
	gtk_tree_model_get (model, iter,
		JANA_GTK_NOTE_STORE_COL_AUTHOR, &author,
		JANA_GTK_NOTE_STORE_COL_RECIPIENT, &recipient,
		JANA_GTK_NOTE_STORE_COL_BODY, &body,
		JANA_GTK_NOTE_STORE_COL_CREATED, &created,
		JANA_GTK_NOTE_STORE_COL_MODIFIED, &modified,
		JANA_GTK_NOTE_STORE_COL_CATEGORIES, &categories, -1);

	if (categories) for (i = 0; categories[i]; i++) {
		/* Note that this category is not meant for display and 
		 * shouldn't be translated... (see phone-kit)
		 */
		if ((strcmp (categories[i], "Sent") == 0) ||
		    (strcmp (categories[i], "Sending") == 0) ||
		    (strcmp (categories[i], "Rejected") == 0)) {
			outgoing = TRUE;
			break;
		}
	}
	
	g_object_set (cell,
		"author", author,
		"recipient", recipient,
		"body", body,
		"created", created,
		"modified", modified,
		"justify", outgoing ?
		      GTK_JUSTIFY_LEFT : GTK_JUSTIFY_RIGHT,
		"icon", outgoing ?
		      data->author_icon : data->recipient_icon,
		NULL);
	
	g_free (author);
	g_free (recipient);
	g_free (body);
	g_strfreev (categories);
	if (created) g_object_unref (created);
	if (modified) g_object_unref (modified);
}

static void
store_opened_cb (JanaStore *store, SmsData *data)
{
	open = TRUE;
	if (!hidden) page_shown (data);
}

GtkWidget *
sms_notes_page_new (SmsData *data)
{
	GtkWidget *treeview, *scroll, *vbox, *searchbar,
		*sms_vbox, *frame, *label, *button;
	GtkCellRenderer *renderer;
	GHashTable *colours_hash;
	
	/* FIXME: Set recipient pixbuf */
	data->recipient_icon = g_object_ref (data->no_photo);
	
	/* Create note store */
	data->notes = jana_ecal_store_new (JANA_COMPONENT_NOTE);
	g_signal_connect (data->notes, "opened",
		G_CALLBACK (store_opened_cb), data);
	
	/* Create model and filter */
	data->note_store = jana_gtk_note_store_new ();
	data->note_filter = gtk_tree_model_filter_new (data->note_store, NULL);
	
	/* Create a category-colour hash for the cell renderer */
	colours_hash = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_insert (colours_hash, "Sent", &alt_color);
	g_hash_table_insert (colours_hash, "Sending", &alt_color);
	
	/* Create treeview and cell renderer */
	treeview = gtk_tree_view_new_with_model (data->note_filter);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
	renderer = jana_gtk_cell_renderer_note_new ();
	g_object_set (renderer, "draw_box", TRUE, "show_recipient", TRUE, NULL);
	gtk_tree_view_insert_column_with_data_func (GTK_TREE_VIEW (treeview), 0,
		"Messages", renderer, (GtkTreeCellDataFunc)sms_notes_data_func,
		data, NULL);
	
	/* Create search bar */
	data->notes_combo = gtk_combo_box_new_text ();
	searchbar = moko_search_bar_new_with_combo (
		GTK_COMBO_BOX (data->notes_combo));
	
	/* Create text entry bits */
	data->sms_textview = gtk_text_view_new ();
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (data->sms_textview),
		GTK_WRAP_WORD_CHAR);
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), data->sms_textview);
	
	label = gtk_label_new (NULL);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_label_set_markup (GTK_LABEL (label),
		"<small>0\n  /\n     160</small>");
	
	button = gtk_button_new_with_label ("Send");
	g_signal_connect (button, "clicked",
		G_CALLBACK (send_clicked_cb), data);
	if (!data->sms_proxy) gtk_widget_set_sensitive (button, FALSE);
	
	sms_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (sms_vbox), label, FALSE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (sms_vbox), button, TRUE, TRUE, 0);

	data->sms_hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (data->sms_hbox), frame, TRUE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (data->sms_hbox), sms_vbox, FALSE, TRUE, 0);

	gtk_widget_show_all (data->sms_hbox);
	gtk_widget_set_no_show_all (data->sms_hbox, TRUE);
	gtk_widget_hide (data->sms_hbox);
	
	/* Pack widgets */
	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), treeview);
	
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), searchbar, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (vbox), data->sms_hbox, FALSE, TRUE, 0);
	gtk_widget_show_all (vbox);
	
	/* Add events for detecting whether the page has been hidden/shown */
	gtk_widget_add_events (treeview, GDK_VISIBILITY_NOTIFY_MASK);
	g_signal_connect (treeview, "visibility-notify-event",
		G_CALLBACK (visibility_notify_event_cb), data);
	g_signal_connect (treeview, "notify::visible",
		G_CALLBACK (notify_visible_cb), data);
	g_signal_connect (vbox, "unmap",
		G_CALLBACK (unmap_cb), data);
	
	/* Connect to new button toggle */
	g_signal_connect (data->new_button, "toggled",
		G_CALLBACK (new_toggled_cb), data);
	
	jana_store_open (data->notes);

	return vbox;
}
