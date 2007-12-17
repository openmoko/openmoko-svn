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

#include "sms-contacts.h"
#include "sms-utils.h"
#include <libmokoui2/moko-finger-scroll.h>
#include <libmokoui2/moko-search-bar.h>
#include <string.h>
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

static const gchar *clear_numbers_uid;

static gboolean hidden = FALSE;

static void
page_shown (SmsData *data)
{
}

static void
page_hidden (SmsData *data)
{
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
	}
	
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
clear_numbers_cb (gchar *number, gchar *uid, GList **list)
{
	if (strcmp (uid, clear_numbers_uid) == 0)
		*list = g_list_prepend (*list, number);
}

static void
clear_numbers (SmsData *data, const gchar *uid)
{
	GList *n, *numbers = NULL;

	clear_numbers_uid = uid;
	g_hash_table_foreach (data->numbers,
		(GHFunc)clear_numbers_cb, &numbers);
	
	for (n = numbers; n; n = n->next)
		g_hash_table_remove (data->numbers, n->data);
	g_list_free (numbers);
}

static void
contacts_store (SmsData *data, GtkTreeIter *iter, EContact *contact)
{
	gint i;
	
	GdkPixbuf *photo = sms_contact_load_photo (contact);

	gtk_list_store_set ((GtkListStore *)data->contacts_store, iter,
		COL_UID, e_contact_get_const (contact, E_CONTACT_UID),
		COL_NAME, e_contact_get_const (contact, E_CONTACT_FULL_NAME),
		COL_ICON, photo, -1);
	if (photo) g_object_unref (photo);
	
	for (i = E_CONTACT_FIRST_PHONE_ID; i <= E_CONTACT_LAST_PHONE_ID; i++) {
		gchar *number = e_contact_get (contact, (EContactField)i);

		if (!number) continue;
		
		g_hash_table_insert (data->numbers, number,
			e_contact_get (contact, E_CONTACT_UID));
	}
}

static void
contacts_added_cb (EBookView *ebookview, GList *contacts, SmsData *data)
{
	for (; contacts; contacts = contacts->next) {
		GtkTreeIter *iter;
		EContact *contact = (EContact *)contacts->data;
		
		if (!contact) continue;
		
		iter = g_slice_new (GtkTreeIter);
		gtk_list_store_append ((GtkListStore *)data->contacts_store,
			iter);
		contacts_store (data, iter, contact);
		g_hash_table_insert (data->contacts,
			e_contact_get (contact, E_CONTACT_UID), iter);
	}

	if (!data->note_count_idle) data->note_count_idle =
		g_idle_add ((GSourceFunc)sms_contacts_note_count_update, data);
}

static void
contacts_changed_cb (EBookView *ebookview, GList *contacts, SmsData *data)
{
	for (; contacts; contacts = contacts->next) {
		GtkTreeIter *iter;
		const gchar *uid;
		
		EContact *contact = (EContact *)contacts->data;
		
		if (!contact) continue;
		
		uid = e_contact_get_const (contact, E_CONTACT_UID);
		iter = g_hash_table_lookup (data->contacts, uid);
		if (iter) {
			clear_numbers (data, uid);
			contacts_store (data, iter, contact);
		}
	}

	if (!data->note_count_idle) data->note_count_idle =
		g_idle_add ((GSourceFunc)sms_contacts_note_count_update, data);
}

static void
contacts_removed_cb (EBookView *ebookview, GList *uids, SmsData *data)
{
	for (; uids; uids = uids->next) {
		GtkTreeIter *iter = g_hash_table_lookup (
			data->contacts, uids->data);

		if (!iter) continue;
		
		clear_numbers (data, (const gchar *)uids->data);
		gtk_list_store_remove ((GtkListStore *)
			data->contacts_store, iter);
		g_hash_table_remove (data->contacts, uids->data);
	}

	if (!data->note_count_idle) data->note_count_idle =
		g_idle_add ((GSourceFunc)sms_contacts_note_count_update, data);
}

static void
free_iter_slice (GtkTreeIter *iter)
{
	g_slice_free (GtkTreeIter, iter);
}

static void
nophoto_filter_func (GtkTreeModel *model, GtkTreeIter *iter, GValue *value,
		     gint column, SmsData *data)
{
	GtkTreeIter real_iter;
	gpointer pointer;
	
	gtk_tree_model_filter_convert_iter_to_child_iter (
	     (GtkTreeModelFilter *)model, &real_iter, iter);
	
	gtk_tree_model_get (data->contacts_store, &real_iter,
		column, &pointer, -1);
	switch (column) {
	    case COL_UID :
	    case COL_NAME :
	    case COL_DETAIL :
		g_value_take_string (value, pointer);
		break;
	    case COL_ICON :
		if (pointer)
			g_value_take_object (value, pointer);
		else
			g_value_set_object (value, data->no_photo);
		break;
	    case COL_UNKNOWN :
		g_value_set_boolean (value, (gboolean)pointer);
		break;
	}
}

static gint
contacts_iter_compare_func (GtkTreeModel *model, GtkTreeIter *a,
			    GtkTreeIter *b, SmsData *data)
{
	gint result, prio1, prio2;
	gchar *name1, *name2, *name1c, *name2c;
	
	gtk_tree_model_get (model, a, COL_PRIORITY, &prio1, -1);
	gtk_tree_model_get (model, b, COL_PRIORITY, &prio2, -1);
	if (prio1 != prio2) return prio2 - prio1;
	
	gtk_tree_model_get (model, a, COL_NAME, &name1, -1);
	gtk_tree_model_get (model, b, COL_NAME, &name2, -1);
	
	name1c = g_utf8_casefold (name1, -1);
	name2c = g_utf8_casefold (name2, -1);
	
	if (name1c && name2c) result = strcmp (name1c, name2c);
	else if (name1c) result = 1;
	else if (name2c) result = -1;
	else result = 0;
	
	g_free (name1c);
	g_free (name1);
	g_free (name2c);
	g_free (name2);
	
	return result;
}

static void
delete_clicked_cb (GtkToolButton *button, SmsData *data)
{
	if (hidden) return;
	
	sms_delete_selected_contact_messages (data);
}

static void
delete_all_added_cb (JanaStoreView *store_view, GList *components,
		     SmsData *data)
{
	for (; components; components = components->next) {
		JanaComponent *comp = JANA_COMPONENT (components->data);
		jana_store_remove_component (
			jana_store_view_get_store (store_view), comp);
	}
}

static void
delete_all_progress_cb (JanaStoreView *store_view, gint percent,
			SmsData *data)
{
	if (percent == 100) g_object_unref (store_view);
}

static void
delete_all_clicked_cb (GtkToolButton *button, SmsData *data)
{
	JanaStoreView *notes_view;
	GtkWidget *dialog;
	gint response;

	if (hidden) return;
	
	dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (data->window),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
		"Delete <b>all</b> messages?");
	gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL, GTK_STOCK_DELETE, GTK_RESPONSE_YES, NULL);
	
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (response != GTK_RESPONSE_YES) return;
	
	/* Delete all messages */
	notes_view = jana_store_get_view (data->notes);
	g_signal_connect (notes_view, "added",
		G_CALLBACK (delete_all_added_cb), data);
	g_signal_connect (notes_view, "progress",
		G_CALLBACK (delete_all_progress_cb), data);
	jana_store_view_start (notes_view);
}

GtkWidget *
sms_contacts_page_new (SmsData *data)
{
	EBookQuery *qrys[(E_CONTACT_LAST_PHONE_ID-E_CONTACT_FIRST_PHONE_ID)+1];
	GtkWidget *searchbar, *scroll, *vbox;
	GtkCellRenderer *renderer;
	EBookQuery *tel_query;
	EBookView *view;
	gint i, width;

	GError *error = NULL;
	
	/* Create query for all contacts with telephone numbers */
	/* FIXME: This query doesn't seem to work? */
	/*for (i = E_CONTACT_FIRST_PHONE_ID; i <= E_CONTACT_LAST_PHONE_ID; i++) {
		qrys[i - E_CONTACT_FIRST_PHONE_ID] =
			e_book_query_field_exists ((EContactField)i);
	}
	i -= E_CONTACT_FIRST_PHONE_ID;
	if (!(tel_query = e_book_query_or (i, qrys, TRUE))) {
		g_warning ("Error creating query");
		return gtk_label_new ("Error creating query");
	}*/
	tel_query = e_book_query_any_field_contains (NULL);
	
	/* Create/retrieve and open system address book */
	/* TODO: async opening? */
	if (!(data->ebook = e_book_new_system_addressbook (&error))) {
		g_warning ("Error retrieving system addressbook: %s",
			error->message);
		g_error_free (error);
		return gtk_label_new ("Error, see console output");
	}
	
	if (!e_book_open (data->ebook, FALSE, &error)) {
		g_warning ("Error opening system addressbook: %s",
			error->message);
		g_error_free (error);
		return gtk_label_new ("Error, see console output");
	}
	
	/* Get view on telephone number query */
	if (!(e_book_get_book_view (data->ebook, tel_query,
	      NULL, 0, &view, &error))) {
		g_warning ("Error retrieving addressbook view: %s",
			error->message);
		g_error_free (error);
		return gtk_label_new ("Error, see console output");
	}
	
	/* Get icon to use when no contact photo exists */
	gtk_icon_size_lookup (GTK_ICON_SIZE_DIALOG, &width, NULL);
	data->no_photo = gtk_icon_theme_load_icon (
		gtk_icon_theme_get_default (), "stock_person", width, 0, NULL);

	/* Create contacts model */
	data->contacts_store = (GtkTreeModel *)gtk_list_store_new (COL_LAST,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, GDK_TYPE_PIXBUF,
		G_TYPE_INT, G_TYPE_BOOLEAN);
	gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (
		data->contacts_store), COL_NAME,
		(GtkTreeIterCompareFunc)contacts_iter_compare_func,
		data, NULL);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (
		data->contacts_store), COL_NAME, GTK_SORT_ASCENDING);
	data->contacts = g_hash_table_new_full (g_str_hash, g_str_equal,
		(GDestroyNotify)g_free, (GDestroyNotify)free_iter_slice);
	data->numbers = g_hash_table_new_full (g_str_hash, g_str_equal,
		(GDestroyNotify)g_free, (GDestroyNotify)g_free);
	
	/* Insert contact for 'unknown' messages */
	gtk_list_store_insert_with_values (
		GTK_LIST_STORE (data->contacts_store), NULL, 0,
		COL_UID, NULL, COL_NAME, "Unknown sender", COL_PRIORITY, 5,
		COL_UNKNOWN, TRUE, -1);
	
	/* Create filter */
	data->contacts_filter = gtk_tree_model_filter_new (
		data->contacts_store, NULL);
	gtk_tree_model_filter_set_modify_func ((GtkTreeModelFilter *)
		data->contacts_filter, COL_LAST,
		(GType []){G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			GDK_TYPE_PIXBUF, G_TYPE_INT, G_TYPE_BOOLEAN },
		(GtkTreeModelFilterModifyFunc)nophoto_filter_func, data, NULL);
	
	/* Create groups model */
	data->contacts_combo = gtk_combo_box_new_text ();
	
	/* Create search box */
	searchbar = moko_search_bar_new_with_combo (
		GTK_COMBO_BOX (data->contacts_combo));
	
	/* Create tree view */
	data->contacts_treeview = gtk_tree_view_new_with_model (
		data->contacts_filter);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (
		data->contacts_treeview), TRUE);
	gtk_tree_view_set_headers_visible (
		GTK_TREE_VIEW (data->contacts_treeview), FALSE);
	
	/* Create renderer and column */
	/* Slight abuse of the note cell renderer I suppose... */
	renderer = jana_gtk_cell_renderer_note_new ();
	g_object_set (G_OBJECT (renderer), "show_created", FALSE,
		"show_recipient", FALSE, NULL);
	
	gtk_tree_view_insert_column_with_attributes (
		GTK_TREE_VIEW (data->contacts_treeview),
		0, NULL, renderer, "author", COL_NAME,
		"body", COL_DETAIL, "icon", COL_ICON, NULL);
	
	g_signal_connect (data->contacts_treeview, "size-allocate",
		G_CALLBACK (jana_gtk_utils_treeview_resize), renderer);

	/* Pack treeview into a finger-scroll */
	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), data->contacts_treeview);
	
	/* Pack widgets into vbox and return */
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), searchbar, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
	gtk_widget_show_all (vbox);
	
	/* Start book view */
	g_signal_connect (view, "contacts-added",
		G_CALLBACK (contacts_added_cb), data);
	g_signal_connect (view, "contacts-changed",
		G_CALLBACK (contacts_changed_cb), data);
	g_signal_connect (view, "contacts-removed",
		G_CALLBACK (contacts_removed_cb), data);
	e_book_view_start (view);
	
	/* Connect to toolbar delete buttons */
	g_signal_connect (data->delete_button, "clicked",
		G_CALLBACK (delete_clicked_cb), data);
	g_signal_connect (data->delete_all_button, "clicked",
		G_CALLBACK (delete_all_clicked_cb), data);

	/* Add events for detecting whether the page has been hidden/shown */
	gtk_widget_add_events (data->contacts_treeview,
		GDK_VISIBILITY_NOTIFY_MASK);
	g_signal_connect (data->contacts_treeview, "visibility-notify-event",
		G_CALLBACK (visibility_notify_event_cb), data);
	g_signal_connect (data->contacts_treeview, "notify::visible",
		G_CALLBACK (notify_visible_cb), data);
	g_signal_connect (vbox, "unmap",
		G_CALLBACK (unmap_cb), data);
	
	return vbox;
}

