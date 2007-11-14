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
#include <libjana-ecal/jana-ecal.h>
#include <libmokoui2/moko-finger-scroll.h>
#include <libhito/hito-contact-store.h>
#include <libebook/e-book.h>

static GdkColor alt_color;
static gboolean hidden = TRUE;
static gboolean open = FALSE;

static void
page_shown (SmsData *data)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint i;

	EContact *contact = NULL;
	
	if (!open) return;
	
	selection = gtk_tree_view_get_selection (
		GTK_TREE_VIEW (data->contacts_treeview));
	
	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) return;

	gtk_tree_model_get (model, &iter, COLUMN_CONTACT, &contact, -1);
	if (!contact) return;
	
	data->notes_view = jana_store_get_view (data->notes);
	for (i = E_CONTACT_FIRST_PHONE_ID; i <= E_CONTACT_LAST_PHONE_ID; i++) {
		const gchar *number = e_contact_get_const (
			contact, (EContactField)i);
		if (!number) continue;
		
		jana_store_view_add_match (data->notes_view,
			JANA_STORE_VIEW_AUTHOR, number);
		jana_store_view_add_match (data->notes_view,
			JANA_STORE_VIEW_RECIPIENT, number);
	}
	jana_gtk_note_store_set_view (JANA_GTK_NOTE_STORE (data->note_store),
		data->notes_view);
	jana_store_view_start (data->notes_view);
	g_object_unref (data->notes_view);

	g_object_unref (contact);
}

static void
page_hidden (SmsData *data)
{
	jana_gtk_note_store_set_view (JANA_GTK_NOTE_STORE (
		data->note_store), NULL);
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

static void sms_notes_data_func (GtkTreeViewColumn *tree_column,
				 GtkCellRenderer *cell,
				 GtkTreeModel *model,
				 GtkTreeIter *iter,
				 SmsData *data)
{
	gchar *author, *recipient, *body;
	JanaTime *created, *modified;
	
	gtk_tree_model_get (model, iter,
		JANA_GTK_NOTE_STORE_COL_AUTHOR, &author,
		JANA_GTK_NOTE_STORE_COL_RECIPIENT, &recipient,
		JANA_GTK_NOTE_STORE_COL_BODY, &body,
		JANA_GTK_NOTE_STORE_COL_CREATED, &created,
		JANA_GTK_NOTE_STORE_COL_MODIFIED, &modified, -1);

	g_object_set (cell,
		"author", author,
		"recipient", recipient,
		"body", body,
		"created", created,
		"modified", modified, NULL);
	
	g_free (author);
	g_free (recipient);
	g_free (body);
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
	GtkWidget *treeview, *scroll, *vbox;
	GtkCellRenderer *renderer;
	GHashTable *colours_hash;
	
	/* Create note store */
	data->notes = jana_ecal_store_new (JANA_COMPONENT_NOTE);
	g_signal_connect (data->notes, "opened",
		G_CALLBACK (store_opened_cb), data);
	data->notes_view = NULL;
	
	/* Create model and filter */
	data->note_store = jana_gtk_note_store_new ();
	data->note_filter = gtk_tree_model_filter_new (data->note_store, NULL);
	
	/* Create a category-colour hash for the cell renderer */
	colours_hash = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_insert (colours_hash, "Sent", &alt_color);
	
	/* Create treeview and cell renderer */
	treeview = gtk_tree_view_new_with_model (data->note_filter);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
	renderer = jana_gtk_cell_renderer_note_new ();
	g_object_set (renderer, "draw_box", TRUE, "show_recipient", TRUE, NULL);
	gtk_tree_view_insert_column_with_data_func (GTK_TREE_VIEW (treeview), 0,
		"Messages", renderer, (GtkTreeCellDataFunc)sms_notes_data_func,
		data, NULL);
	
	/* Pack widgets */
	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), treeview);
	
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
	gtk_widget_show_all (vbox);
	
	/* Add events for detecting whether the page has been hidden/shown */
	gtk_widget_add_events (treeview, GDK_VISIBILITY_NOTIFY_MASK);
	g_signal_connect (treeview, "visibility-notify-event",
		G_CALLBACK (visibility_notify_event_cb), data);
	g_signal_connect (treeview, "notify::visible",
		G_CALLBACK (notify_visible_cb), data);
	g_signal_connect (vbox, "unmap",
		G_CALLBACK (unmap_cb), data);
	
	jana_store_open (data->notes);

	return vbox;
}
