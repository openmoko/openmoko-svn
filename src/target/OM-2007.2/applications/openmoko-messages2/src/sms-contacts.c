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
#include <libmokoui2/moko-finger-scroll.h>
#include <libmokoui2/moko-search-bar.h>
#include <libhito/hito-contact-store.h>
#include <libhito/hito-group-store.h>
#include <libhito/hito-all-group.h>
#include <libhito/hito-group-combo.h>
#include <libebook/e-book.h>

static void sms_contacts_data_func (GtkTreeViewColumn *tree_column,
				    GtkCellRenderer *cell,
				    GtkTreeModel *model,
				    GtkTreeIter *iter,
				    SmsData *data)
{
	EContact *contact = NULL;
	
	gtk_tree_model_get (model, iter, COLUMN_CONTACT, &contact, -1);
	if (!contact) return;
	
	g_object_set (cell,
		"author", e_contact_get_const (contact, E_CONTACT_FULL_NAME),
		"body", "0 sent, 0 received", NULL);
	
	g_object_unref (contact);
}

GtkWidget *
sms_contacts_page_new (SmsData *data)
{
	EBookQuery *qrys[(E_CONTACT_LAST_PHONE_ID-E_CONTACT_FIRST_PHONE_ID)+1];
	GtkWidget *searchbar, *scroll, *vbox;
	GtkCellRenderer *renderer;
	GtkTreeModel *group_store;
	EBookQuery *tel_query;
	EBookView *view;
	EBook *book;
	gint i;

	/* Temporary, just for testing */
	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon (
		gtk_icon_theme_get_default (), "stock_person", 48, 0, NULL);

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
	if (!(book = e_book_new_system_addressbook (&error))) {
		g_warning ("Error retrieving system addressbook: %s",
			error->message);
		g_error_free (error);
		return gtk_label_new ("Error, see console output");
	}
	
	if (!e_book_open (book, FALSE, &error)) {
		g_warning ("Error opening system addressbook: %s",
			error->message);
		g_error_free (error);
		return gtk_label_new ("Error, see console output");
	}
	
	/* Get view on telephone number query */
	if (!(e_book_get_book_view (book, tel_query, NULL, 0, &view, &error))) {
		g_warning ("Error retrieving addressbook view: %s",
			error->message);
		g_error_free (error);
		return gtk_label_new ("Error, see console output");
	}
	
	/* Create contacts model */
	data->contacts_store = hito_contact_store_new (view);
	
	/* Create filter */
	data->contacts_filter = hito_contact_model_filter_new (
		HITO_CONTACT_STORE (data->contacts_store));
	
	/* Create groups model */
	group_store = hito_group_store_new ();
	hito_group_store_add_group (HITO_GROUP_STORE (group_store),
		hito_all_group_new ());
	hito_group_store_set_view (HITO_GROUP_STORE (group_store), view);
	
	/* Create search box */
	searchbar = moko_search_bar_new_with_combo (GTK_COMBO_BOX (
		hito_group_combo_new (HITO_GROUP_STORE (group_store))));
	
	/* Create cell renderer and tree view */
	data->contacts_treeview = gtk_tree_view_new_with_model (
		data->contacts_filter);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (
		data->contacts_treeview), TRUE);
	gtk_tree_view_set_headers_visible (
		GTK_TREE_VIEW (data->contacts_treeview), FALSE);
	renderer = jana_gtk_cell_renderer_note_new ();
	g_signal_connect (data->contacts_treeview, "size-allocate",
		G_CALLBACK (jana_gtk_utils_treeview_resize), renderer);
	g_object_set (G_OBJECT (renderer), "show_created", FALSE,
		"show_recipient", FALSE, "icon", pixbuf, NULL);
	gtk_tree_view_insert_column_with_data_func (
		GTK_TREE_VIEW (data->contacts_treeview),
		0, NULL, renderer, (GtkTreeCellDataFunc)sms_contacts_data_func,
		data, NULL);
	
	/* Pack treeview into a finger-scroll */
	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), data->contacts_treeview);
	
	/* Pack widgets into vbox and return */
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), searchbar, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
	gtk_widget_show_all (vbox);
	
	/* Start book view */
	e_book_view_start (view);
	
	return vbox;
}

