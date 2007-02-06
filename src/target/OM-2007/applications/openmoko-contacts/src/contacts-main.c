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
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <libebook/e-book.h>
#include "config.h"
#ifdef HAVE_GNOMEVFS
#include <libgnomevfs/gnome-vfs.h>
#endif

#include "bacon-message-connection.h"
#include "contacts-defs.h"
#include "contacts-utils.h"
#include "contacts-main.h"
#include "contacts-contact-pane.h"
#include "contacts-callbacks-ui.h"
#include "contacts-callbacks-ebook.h"
#include "contacts-ui.h"

void
contacts_update_treeview (ContactsData *data)
{
	GtkTreeModelFilter *model;
	gint visible_rows;

	model = GTK_TREE_MODEL_FILTER (data->contacts_filter);
	gtk_tree_model_filter_refilter (model);
	
	visible_rows = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (model),
						       NULL);
	/* If there's only one visible contact, select it */
	if (visible_rows == 1) {
		GtkTreeSelection *selection =
					gtk_tree_view_get_selection (GTK_TREE_VIEW (data->ui->contacts_treeview));
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model),
						   &iter)) {
			gtk_tree_selection_select_iter (selection, &iter);
		}
	}
}

static void
start_query (EBook *book, EBookStatus status, EBookView *book_view,
	gpointer closure)
{
	ContactsData *contacts_data = closure;
	if (status == E_BOOK_ERROR_OK) {
		g_object_ref (book_view);
		contacts_data->book_view = book_view;
		/* Yuck. */
		contacts_contact_pane_set_book_view
		  (CONTACTS_CONTACT_PANE (contacts_data->ui->contact_pane), book_view);

		/* Connect signals on EBookView */
		g_signal_connect (G_OBJECT (book_view), "contacts_added",
			G_CALLBACK (contacts_added_cb), contacts_data);
		g_signal_connect (G_OBJECT (book_view), "contacts_changed",
			G_CALLBACK (contacts_changed_cb), contacts_data);
		g_signal_connect (G_OBJECT (book_view), "contacts_removed",
			G_CALLBACK (contacts_removed_cb), contacts_data);
		g_signal_connect (G_OBJECT (book_view), "sequence_complete",
			G_CALLBACK (contacts_sequence_complete_cb), contacts_data);
		
		e_book_view_start (book_view);
	} else {
		g_warning("Got error %d when getting book view", status);
	}
}

static void
opened_book (EBook *book, EBookStatus status, gpointer closure)
{
	ContactsData *contacts_data = closure;
	EBookQuery *query;
	
	if (status == E_BOOK_ERROR_OK) {
		query = e_book_query_any_field_contains ("");
		e_book_async_get_book_view (contacts_data->book, query, NULL,
			-1, start_query, contacts_data);
		e_book_query_unref (query);
	} else {
		g_warning("Got error %d when opening book", status);
	}
}

static gboolean
open_book (gpointer data)
{
	ContactsData *contacts_data = data;
	e_book_async_open (
		contacts_data->book, FALSE, opened_book, contacts_data);
	return FALSE;
}

static gboolean
contacts_import_from_param (gpointer data)
{
	ContactsData *contacts_data = data;
	
	g_printf ("Opening '%s'\n", contacts_data->file);
	contacts_import (contacts_data, contacts_data->file, TRUE);
	
	return FALSE;
}

static void
contacts_bacon_cb (const char *message, gpointer user_data)
{
	ContactsData *data = user_data;
	
	if (!message)
		return;
	
	gtk_window_present (GTK_WINDOW (data->ui->main_window));
	if (message[0] != ':') {
		g_printf ("Opening '%s'\n", message);
		contacts_import (data, message, TRUE);
	}
}

int
main (int argc, char **argv)
{
	BaconMessageConnection *mc;
#ifdef HAVE_GCONF
	const char *search;
	GConfClient *client;
#endif
	ContactsData *contacts_data;	/* Variable for passing around data -
					 * see contacts-defs.h.
					 */
	GOptionContext *context;
	static gint plug = 0;
	GtkWidget *widget;
	
	static GOptionEntry entries[] = {
		{ "plug", 'p', 0, G_OPTION_ARG_INT, &plug,
			"Socket ID of an XEmbed socket to plug into", NULL },
		{ NULL }
	};

        /* Initialise the i18n support code */
        bindtextdomain (GETTEXT_PACKAGE, CONTACTS_LOCALE_DIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);

	context = g_option_context_new (" - A light-weight address-book");
	g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	g_option_context_parse (context, &argc, &argv, NULL);

	mc = bacon_message_connection_new ("contacts");
	if (!bacon_message_connection_get_is_server (mc)) {
		bacon_message_connection_send (mc, argv[1] ? argv[1] : ":");
		bacon_message_connection_free (mc);
		gdk_notify_startup_complete ();
		return 0;
	}

	contacts_data = g_new0 (ContactsData, 1);
	contacts_data->ui = g_new0 (ContactsUI, 1);
	contacts_data->initialising = TRUE; /* initialising until contacts have been loaded for the first time */
	bacon_message_connection_set_callback (
		mc, contacts_bacon_cb, contacts_data);

	/* Set critical errors to close application */
	//g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL);

	/* Load the system addressbook */
	contacts_data->book = e_book_new_system_addressbook (NULL);
	if (!contacts_data->book)
		g_critical ("Could not load system addressbook");

	contacts_data->contacts_table = g_hash_table_new_full (g_str_hash,
						g_str_equal, NULL, 
						(GDestroyNotify)
						 contacts_free_list_hash);

	contacts_data->groups_widgets_hash = g_hash_table_new (NULL, NULL);

	/* Setup the ui */
	contacts_setup_ui (contacts_data);

#ifdef HAVE_GNOMEVFS
	gnome_vfs_init ();
#endif

	/* Start */
	g_idle_add (open_book, contacts_data);
	if (argv[1] != NULL) {
		contacts_data->file = argv[1];
		g_idle_add (contacts_import_from_param, contacts_data);
	}
	
	widget = contacts_data->ui->main_window;
	if (plug > 0) {
		GtkWidget *plug_widget;
		GtkWidget *contents;
		
		g_debug ("Plugging into socket %d", plug);
		plug_widget = gtk_plug_new (plug);
		contents = g_object_ref (gtk_bin_get_child (GTK_BIN (widget)));
		gtk_container_remove (GTK_CONTAINER (widget), contents);
		gtk_container_add (GTK_CONTAINER (plug_widget), contents);
		g_object_unref (contents);
		g_signal_connect (G_OBJECT (plug_widget), "destroy",
				  G_CALLBACK (gtk_main_quit), NULL);
		widget = contacts_data->ui->main_menubar;
		gtk_widget_hide (widget);
		gtk_widget_show (plug_widget);
	} else {
		g_signal_connect (G_OBJECT (widget), "destroy",
				  G_CALLBACK (gtk_main_quit), NULL);
		gtk_widget_show (widget);
	}

	/* fix icon sizes to 16x16 for the moment... */
	gtk_rc_parse_string ("gtk_icon_sizes=\"gtk-button=16,16:gtk-menu=16,16\"");

	gtk_main ();

	/* if we have modified the current contact, but not saved it, do so now */
	if (data->changed)
		e_book_async_commit_contact (data->book, data->contact, NULL, NULL);

	/* Unload the addressbook */
	e_book_view_stop (contacts_data->book_view);
	g_object_unref (contacts_data->book_view);
	g_object_unref (contacts_data->book);

	/* free various things */
	g_free (contacts_data->ui);
	g_free (contacts_data->selected_group);
	g_free (contacts_data->search_string);
	g_list_free (contacts_data->contacts_groups);
	//g_hash_table_destroy (contacts_data->contacts_table);
	g_hash_table_destroy (contacts_data->groups_widgets_hash);


	return 0;
}
