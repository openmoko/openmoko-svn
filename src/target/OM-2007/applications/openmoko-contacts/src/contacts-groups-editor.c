/*
 * Copyright (C) 2006-2007 by OpenMoko, Inc.
 * Written by OpenedHand Ltd <info@openedhand.com>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "contacts-defs.h"
#include "contacts-ui.h"
#include "contacts-main.h"
#include "contacts-utils.h"

void
contacts_groups_pane_show (GtkWidget *button, ContactsData *data)
{
	gtk_notebook_set_current_page (GTK_NOTEBOOK (data->ui->main_notebook), CONTACTS_GROUPS_PANE);
	contacts_groups_pane_update_selection (gtk_tree_view_get_selection (GTK_TREE_VIEW (data->ui->contacts_treeview)), data);
}

void
contacts_groups_pane_update_selection (GtkTreeSelection *selection, ContactsData *data)
{
	GtkWidget *widget;
	EContact *contact;
	GList *groups, *g;


	if (!selection)
	{
		gtk_widget_set_sensitive (GTK_WIDGET (data->ui->groups_vbox), FALSE);
		return;
	}

	/* Get the currently selected contact and update the contact summary */
	contact = contacts_contact_from_selection (selection,
						   data->contacts_table);
	if (!contact)
	{
		gtk_widget_set_sensitive (GTK_WIDGET (data->ui->groups_vbox), FALSE);
		return;
	}


	gtk_widget_set_sensitive (GTK_WIDGET (data->ui->groups_vbox), TRUE);

	groups = e_contact_get (contact, E_CONTACT_CATEGORY_LIST);
	for (g = data->contacts_groups; g; g = g_list_next (g))
	{
		widget = g_hash_table_lookup (data->groups_widgets_hash, g->data);
		gtk_widget_set_sensitive (GTK_WIDGET (widget), TRUE);

		if (g_list_find_custom (groups, g->data, (GCompareFunc) strcmp))
		{
			/* make sure we don't set updating flag unless it is actually going to change */
			if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
			{
				g_object_set_data (G_OBJECT (widget), "updating", GINT_TO_POINTER (TRUE));
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);
			}
		}
		else
		{
			/* make sure we don't set updating flag unless it is actually going to change */
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
			{
				g_object_set_data (G_OBJECT (widget), "updating", GINT_TO_POINTER (TRUE));
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), FALSE);
			}
		}
	}


}

static void
container_remove (GtkWidget *child, GtkWidget *container)
{
	gtk_container_remove (GTK_CONTAINER (container), child);
}

void
contacts_groups_new_cb (GtkWidget *button, ContactsData *data)
{
	GtkWidget *widget;
	gchar *text;
	GtkTreeSelection *selection;
	GtkWidget *input_dialog;

	input_dialog = gtk_dialog_new_with_buttons ("Add Group",
			GTK_WINDOW (data->ui->main_window), GTK_DIALOG_MODAL,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			NULL);
	gtk_dialog_set_has_separator (GTK_DIALOG (input_dialog), FALSE);

	widget = gtk_entry_new ();
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (input_dialog)->vbox), widget);
	gtk_widget_show (widget);

	if (gtk_dialog_run (GTK_DIALOG (input_dialog)) != GTK_RESPONSE_ACCEPT)
	{
		gtk_widget_destroy (input_dialog);
		return;
	}

	text = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));
	gtk_widget_destroy (input_dialog);

	if (!text || !strcmp (text, ""))
		return;

	/* if there were no previous groups, remove the "no groups" label */
	if (g_list_length (data->contacts_groups) == 0)
		gtk_container_foreach (GTK_CONTAINER (data->ui->groups_vbox), (GtkCallback) container_remove, data->ui->groups_vbox);

	/* add the group to the list of groups */
	if (!g_list_find_custom (data->contacts_groups, text, (GCompareFunc) strcmp))
		data->contacts_groups = g_list_append (data->contacts_groups, text);


	/* add a group checkbutton */
	if (!(widget = g_hash_table_lookup (data->groups_widgets_hash, text)))
	{
		widget = gtk_check_button_new_with_label (text);
		gtk_box_pack_start (GTK_BOX (data->ui->groups_vbox), widget, FALSE, FALSE, 0);
		g_signal_connect (G_OBJECT (widget), "toggled", G_CALLBACK (groups_checkbutton_cb), data);
		gtk_widget_show (widget);
		g_hash_table_insert (data->groups_widgets_hash, text, widget);
	}

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (data->ui->contacts_treeview));
	if (gtk_tree_selection_count_selected_rows (selection) == 0)
	{
		/* no contact selected, so just disable for now */
		gtk_widget_set_sensitive (widget, FALSE);
	}
	else
	{
		/* add the new group to the current contact */
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);
	}

	/* update groups */
	contacts_ui_update_groups_list (data);

}

void
groups_checkbutton_cb (GtkWidget *checkbutton, ContactsData *data)
{
	EContact *contact;
	GtkTreeSelection *selection;
	GList *current_groups, *g = NULL;
	gchar *new_group;

	if (GPOINTER_TO_INT (g_object_get_data (G_OBJECT (checkbutton), "updating")))
	{
		g_object_set_data (G_OBJECT(checkbutton), "updating", GINT_TO_POINTER (FALSE));
		return;
	}

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (data->ui->contacts_treeview));

	contact = contacts_contact_from_selection (selection,
						   data->contacts_table);
	current_groups = e_contact_get (contact, E_CONTACT_CATEGORY_LIST);

	/* TODO: probably ought to do something better here */
	new_group = g_strdup(gtk_button_get_label (GTK_BUTTON (checkbutton)));

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton)))
	{
		/* add this to the contact */
		current_groups = g_list_append (current_groups, new_group);
	}
	else
	{
		/* make sure this isn't in the list */
		g = g_list_find_custom (current_groups, new_group, (GCompareFunc) strcmp);
		if (g)
			current_groups = g_list_remove (current_groups, g->data);
	}

	e_contact_set (contact, E_CONTACT_CATEGORY_LIST, current_groups);
	data->changed = TRUE;
}
