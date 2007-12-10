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
	
	if (!gtk_tree_model_get_iter_first (data->contacts_store, &iter)) {
		data->note_count_idle = 0;
		return FALSE;
	}
	
	do {
		gint i;
		EContact *contact;
		gchar *uid;
		
		gtk_tree_model_get (data->contacts_store, &iter, COL_UID,
			&uid, -1);
		if (!uid) {
			unknown_iter = iter;
			continue;
		}
		
		if (!e_book_get_contact (data->ebook, uid, &contact, NULL)) {
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
	
	return FALSE;
}
