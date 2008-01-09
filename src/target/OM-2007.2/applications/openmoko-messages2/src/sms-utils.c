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
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

void
sms_clear_combo_box_text (GtkComboBox *combo)
{
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_combo_box_get_model (combo);

	while (gtk_tree_model_get_iter_first (model, &iter))
		gtk_combo_box_remove_text (combo, 0);
}

gboolean
sms_select_contact (SmsData *data, const gchar *uid)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	
	if (gtk_tree_model_get_iter_first (data->contacts_store, &iter)) do {
		gchar *iter_uid;
		gtk_tree_model_get (data->contacts_store,
			&iter, COL_UID, &iter_uid, -1);
		
		if (iter_uid && (strcmp (uid, iter_uid) == 0)) {
			g_free (iter_uid);
			selection = gtk_tree_view_get_selection (
				GTK_TREE_VIEW (data->contacts_treeview));
			gtk_tree_selection_select_iter (selection, &iter);
			return TRUE;
		}
		
		g_free (iter_uid);
		
	} while (gtk_tree_model_iter_next (data->contacts_store, &iter));
	
	return FALSE;
}

EContact *
sms_get_selected_contact (SmsData *data)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	EContact *contact;
	GtkTreeIter iter;
	
	GError *error = NULL;
	
	selection = gtk_tree_view_get_selection (
		GTK_TREE_VIEW (data->contacts_treeview));
	
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return NULL;
	gtk_tree_model_get (model, &iter, COL_UID, &data->author_uid, -1);
	if (!data->author_uid) return NULL;
	
	if (!e_book_get_contact (data->ebook,
	     data->author_uid, &contact, &error)) {
		g_warning ("Error retrieving contact: %s", error->message);
		g_error_free (error);
		contact = NULL;
	}
	
	return contact;
}

gboolean
sms_delete_selected_contact_messages (SmsData *data)
{
	EContact *contact;
	GtkWidget *dialog;
	
	contact = sms_get_selected_contact (data);
	dialog = gtk_message_dialog_new (GTK_WINDOW (data->window),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
		"Delete all messages from %s?", contact ? (const gchar *)
		e_contact_get_const (contact, E_CONTACT_FULL_NAME) :
		"unknown contacts");
	gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL, GTK_STOCK_DELETE, GTK_RESPONSE_YES, NULL);
	
	if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_YES) {
		gtk_widget_destroy (dialog);
		return FALSE;
	}
	
	gtk_widget_destroy (dialog);

	if (contact) {
		gint i, j;
		
		for (i = E_CONTACT_FIRST_PHONE_ID;
		     i <= E_CONTACT_LAST_PHONE_ID; i++) {
			SmsNoteCountData *ncdata;
			const gchar *number = e_contact_get_const (
				contact, (EContactField)i);
			if (!number) continue;
			
			ncdata = g_hash_table_lookup (data->note_count, number);
			if (!ncdata) continue;
			
			for (j = 0; j < 2; j++) {
				GList *uids = j ? ncdata->read : ncdata->unread;
				for (; uids; uids = uids->next) {
					/* TODO: Add
					 * jana_store_remove_component_from_uid
					 * to libjana?
					 */
					JanaComponent *comp =
						jana_store_get_component (
							data->notes,
							uids->data);
					jana_store_remove_component (
						data->notes, comp);
					g_object_unref (comp);
				}
			}
			
			g_hash_table_remove (data->note_count, number);
		}
		
		g_object_unref (contact);
	} else {
		while (data->unassigned_notes) {
			JanaComponent *comp = jana_store_get_component (
				data->notes, data->unassigned_notes->data);
			jana_store_remove_component (data->notes, comp);
			g_object_unref (comp);
			data->unassigned_notes = g_list_delete_link (
				data->unassigned_notes, data->unassigned_notes);
		}
	}
	
	return TRUE;
}

/* Following two functions taken from pimlico Contacts and modified slightly */
static void
contact_photo_size (GdkPixbufLoader * loader, gint width, gint height,
		    gpointer user_data)
{
	/* Max height of GTK_ICON_SIZE_DIALOG */
	gint iconwidth, iconheight;
	gtk_icon_size_lookup (GTK_ICON_SIZE_DIALOG, &iconwidth, &iconheight);
	
	gdk_pixbuf_loader_set_size (loader,
				    width / ((gdouble) height /
					     iconheight), iconheight);
}

GdkPixbuf *
sms_contact_load_photo (EContact *contact)
{
	EContactPhoto *photo;
	GdkPixbuf *pixbuf = NULL;
	
	/* Retrieve contact picture and resize */
	photo = e_contact_get (contact, E_CONTACT_PHOTO);
	if (photo) {
		GdkPixbufLoader *loader = gdk_pixbuf_loader_new ();
		if (loader) {
			g_signal_connect (G_OBJECT (loader),
					  "size-prepared",
					  G_CALLBACK (contact_photo_size),
					  NULL);
#if HAVE_PHOTO_TYPE
			switch (photo->type) {
			case E_CONTACT_PHOTO_TYPE_INLINED :
				gdk_pixbuf_loader_write (loader,
					photo->data.inlined.data,
					photo->data.inlined.length, NULL);
				break;
			case E_CONTACT_PHOTO_TYPE_URI :
			default :
				g_warning ("Cannot handle URI photos yet");
				g_object_unref (loader);
				loader = NULL;
				break;
			}
#else
			gdk_pixbuf_loader_write (loader, (const guchar *)
				photo->data, photo->length, NULL);
#endif
			if (loader) {
				gdk_pixbuf_loader_close (loader, NULL);
				pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
				if (pixbuf) g_object_ref (pixbuf);
				g_object_unref (loader);
			}
		}
		e_contact_photo_free (photo);
	}
	
	return pixbuf;
}

static void
set_message_count (SmsData *data, GtkTreeIter *iter, gint read, gint unread,
		   gboolean unknown)
{
	gchar *detail;
	gint priority;

	detail = g_strdup_printf ("%d unread\n%d read", unread, read);
	priority = unread ? (unknown ? 15 : 10) : (unknown ? 5 : 0);
	gtk_list_store_set (GTK_LIST_STORE (data->contacts_store),
		iter, COL_DETAIL, detail, COL_PRIORITY, priority, -1);
	g_free (detail);
}

gboolean
sms_contacts_note_count_update (SmsData *data)
{
	static guint assignment = 1;
	GHashTable *indexed_uids;
	GList *ncdatas, *n;
	gint unread, read;

	GtkTreeIter iter, unknown_iter;
	
	data->note_count_idle = 0;

	/* Update the delete-all button if we're on the contacts page */
	sms_contacts_update_delete_all (data);

	/* Change sort column so changing priorities doesn't break iterating 
	 * through the model.
	 */
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (
		data->contacts_store), COL_UID, GTK_SORT_ASCENDING);

	if (!gtk_tree_model_get_iter_first (data->contacts_store, &iter))
		return FALSE;
	
	do {
		gint i;
		EContact *contact;
		gchar *uid;
		gboolean unknown;
		
		GError *error = NULL;
		
		gtk_tree_model_get (data->contacts_store, &iter, COL_UID,
			&uid, COL_UNKNOWN, &unknown, -1);
		if (!uid) {
			if (unknown) unknown_iter = iter;
			continue;
		}
		
		if (!e_book_get_contact (data->ebook, uid, &contact, &error)) {
			g_warning ("Error retrieving contact: %s",
				error->message);
			g_error_free (error);
			g_free (uid);
			continue;
		}
		g_free (uid);
		
		unread = 0;
		read = 0;
		for (i = E_CONTACT_FIRST_PHONE_ID;
		     i <= E_CONTACT_LAST_PHONE_ID; i++) {
			SmsNoteCountData *ncdata;
			const gchar *number = e_contact_get_const (
				contact, (EContactField)i);
			if (!number) continue;
			
			ncdata = g_hash_table_lookup (data->note_count, number);
			if (!ncdata) continue;
			
			read += g_list_length (ncdata->read);
			unread += g_list_length (ncdata->unread);
			ncdata->assigned = assignment;
		}
		
		set_message_count (data, &iter, read, unread, FALSE);
	} while (gtk_tree_model_iter_next (data->contacts_store, &iter));
	
	data->note_count_idle = 0;
	
	/* Make a list of unassigned note UIDs */
	while (data->unassigned_notes) {
		g_free (data->unassigned_notes->data);
		data->unassigned_notes = g_list_delete_link (
			data->unassigned_notes, data->unassigned_notes);
	}
	indexed_uids = g_hash_table_new (g_str_hash, g_str_equal);
	ncdatas = g_hash_table_get_values (data->note_count);
	read = 0;
	unread = 0;
	for (n = ncdatas; n; n = n->next) {
		gint i;
		GList *u;
		SmsNoteCountData *ncdata = (SmsNoteCountData *)n->data;
		
		if (ncdata->assigned == assignment) continue;
		
		for (i = 0; i < 2; i++) {
			for (u = i ? ncdata->read : ncdata->unread;
			     u; u = u->next) {
				gchar *uid = (gchar *)u->data;
				if (!g_hash_table_lookup (indexed_uids, uid)) {
					g_hash_table_insert (indexed_uids, uid,
						GINT_TO_POINTER (1));
					data->unassigned_notes =
						g_list_prepend (
							data->unassigned_notes,
							g_strdup (uid));
					if (i) read ++;
					else unread ++;
				}
			}
		}
	}
	g_list_free (ncdatas);
	g_hash_table_destroy (indexed_uids);
	
	set_message_count (data, &unknown_iter, read, unread, TRUE);
	
	/* Add 2 to assignment so that when it eventually wraps (which will 
	 * almost certainly not happen, but still), it won't ever equal 0.
	 */
	assignment += 2;
	
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (
		data->contacts_store), COL_NAME, GTK_SORT_ASCENDING);

	return FALSE;
}