/* 
 *  Contacts - A small libebook-based address book.
 *
 *  Authored By Chris Lord <chris@o-hand.com>
 *
 *  Copyright (c) 2005 OpenedHand Ltd - http://o-hand.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libebook/e-book.h>

#include "contacts-defs.h"
#include "contacts-ui.h"
#include "contacts-utils.h"
#include "contacts-main.h"

void
contacts_added_cb (EBookView *book_view, const GList *contacts,
		   ContactsData *data)
{
	GtkListStore *model;
	GtkTreeModelFilter *filter;
	GList *c;
	GtkWidget *widget;
	GHashTable *contacts_table = data->contacts_table;

	/* Get TreeView model and combo box to add contacts */
	widget = data->ui->contacts_treeview;
	filter = GTK_TREE_MODEL_FILTER (gtk_tree_view_get_model 
				 	(GTK_TREE_VIEW (widget)));
	model = GTK_LIST_STORE (gtk_tree_model_filter_get_model (filter));

	/* Iterate over new contacts and add them to the list */
	for (c = (GList *)contacts; c; c = c->next) {
		GList *contact_groups;
		const gchar *name, *uid;
		EContact *contact = E_CONTACT (c->data);
		EContactListHash *hash;
		
		if (!E_IS_CONTACT (contact))
			continue;

		/* Check if the contact has already been added (can happen if
		 * contact has been manually added before handling the signal)
		 */
		if (g_hash_table_lookup (contacts_table, e_contact_get_const (
		    contact, E_CONTACT_UID)) != NULL)
			continue;
			
		/* Add contact to list */
		hash = g_new (EContactListHash, 1);
		name = e_contact_get_const (contact, E_CONTACT_FULL_NAME);
		if ((!name) || (g_utf8_strlen (name, -1) <= 0))
			name = "Unnamed";


		const gchar * cell = e_contact_get_const (contact, E_CONTACT_PHONE_MOBILE);
		if ((!cell) || (g_utf8_strlen (cell, -1) <= 0))
			cell = "";

		uid = e_contact_get_const (contact, E_CONTACT_UID);
		hash->contact = g_object_ref (contact);
		hash->contacts_data = data;
		g_hash_table_insert (contacts_table, (gchar *)uid, hash);

		gtk_list_store_insert_with_values (model, &hash->iter, 0,
						   CONTACT_NAME_COL, name,
						   CONTACT_UID_COL, uid,
						   CONTACT_CELLPHONE_COL, cell,
						   -1);

		/* Check for groups and add them to group list */
		contact_groups =
		    e_contact_get (contact,
				   E_CONTACT_CATEGORY_LIST);
		if (contact_groups) {
			GList *group;
			for (group = contact_groups; group;
			     group = group->next) {
				if (!g_list_find_custom (data->contacts_groups,
							 group->data,
							 (GCompareFunc) strcmp))
				{
					data->contacts_groups = g_list_prepend
					   (data->contacts_groups, group->data);
				}
			}
			g_list_foreach (contact_groups, (GFunc) g_free, NULL);
			g_list_free (contact_groups);
		}
	}
}

void
contacts_changed_cb (EBookView *book_view, const GList *contacts,
		     ContactsData *data)
{
	GList *c;
	GtkWidget *widget;
	GtkListStore *model;

	EContact *current_contact = contacts_get_selected_contact (data,
							data->contacts_table);
	if (current_contact) g_object_ref (current_contact);

	widget = data->ui->contacts_treeview;
	model = GTK_LIST_STORE (gtk_tree_model_filter_get_model 
		 (GTK_TREE_MODEL_FILTER (gtk_tree_view_get_model 
		  (GTK_TREE_VIEW (widget)))));

	/* Loop through changed contacts */	
	for (c = (GList *)contacts; c; c = c->next) {
		EContact *contact = E_CONTACT (c->data);
		EContactListHash *hash;
		GList *contact_groups;
		const gchar *uid, *name, *cell;

		/* Lookup if contact exists in internal list (it should) */
		uid = e_contact_get_const (contact, E_CONTACT_UID);
		hash = g_hash_table_lookup (data->contacts_table, uid);
		if (!hash) continue;

		/* TODO: There's some funniness going on here... */
		/* Replace contact */
		/* g_object_unref (hash->contact); */
		hash->contact = g_object_ref (contact);
		hash->contacts_data = data;
		g_hash_table_steal (data->contacts_table, uid);
		g_hash_table_insert (data->contacts_table, (gchar *)uid, hash);

		/* Update list with possibly new name */
		name = e_contact_get_const (contact, E_CONTACT_FULL_NAME);
		if ((!name) || (g_utf8_strlen (name, -1) <= 0))
			name = "Unnamed";
		gtk_list_store_set (model, &hash->iter, CONTACT_NAME_COL, name, -1);

		cell = e_contact_get_const (contact, E_CONTACT_PHONE_MOBILE);
		if ((!cell) || (g_utf8_strlen (cell, -1) <= 0))
      cell = "";
			gtk_list_store_set (model, &hash->iter, CONTACT_CELLPHONE_COL, cell, -1);

		/* If contact is currently selected, update display */
		/*
		if (current_contact) {
			if (strcmp (e_contact_get_const
					(contact, E_CONTACT_UID),
				    e_contact_get_const
					(current_contact, E_CONTACT_UID)) == 0)
				contacts_display_summary (contact, data);
		}*/

		/* Check for groups and add them to group list */
		contact_groups =
		    e_contact_get (contact,
				   E_CONTACT_CATEGORY_LIST);
		if (contact_groups) {
			GList *group;
			for (group = contact_groups; group;
			     group = group->next) {
				if (!g_list_find_custom (data->contacts_groups,
							 group->data,
							 (GCompareFunc) strcmp))
				{
					data->contacts_groups = g_list_prepend 
					   (data->contacts_groups, group->data);
				}
			}
			g_list_free (contact_groups);
		}
	}
	
	if (current_contact) g_object_unref (current_contact);
}

/* TODO: Remove groups that no longer contain contacts */
void
contacts_removed_cb (EBookView *book_view, const GList *ids, ContactsData *data)
{
	GList *i;
	
	for (i = (GList *)ids; i; i = i->next) {
		const gchar *uid = (const gchar *)i->data;
		g_hash_table_remove (data->contacts_table, uid);
	}
}

void
contacts_sequence_complete_cb (EBookView *book_view, const GList *ids, ContactsData *data)
{
	GtkTreeModelFilter *model;

	model = GTK_TREE_MODEL_FILTER (data->contacts_filter);

	/* If there's only one visible contact, select it */
	if (data->initialising) {
		GtkTreeSelection *selection =
					gtk_tree_view_get_selection (GTK_TREE_VIEW (data->ui->contacts_treeview));
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model),
						   &iter)) {
			gtk_tree_selection_select_iter (selection, &iter);
		}
		data->initialising = FALSE;
		gtk_tree_view_scroll_to_point (GTK_TREE_VIEW (data->ui->contacts_treeview), 0, 0);
	}

	/* Update view */
	contacts_update_treeview (data);

	/* Update filter menu */
	contacts_ui_update_groups_list (data);

}
