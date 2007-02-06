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
#include "config.h"
#ifdef HAVE_GNOMEVFS
#include <libgnomevfs/gnome-vfs.h>
#endif

#include "contacts-defs.h"
#include "contacts-utils.h"
#include "contacts-ui.h"
#include "contacts-callbacks-ui.h"
#include "contacts-contact-pane.h"
#include "contacts-callbacks-ebook.h"
#include "contacts-main.h"

void
contacts_chooser_add_cb (GtkWidget *button, ContactsData *data)
{
	GtkWidget *treeview, *entry;
	GtkListStore *model;
	GtkTreeIter iter;
	const gchar *text;
	
	entry = data->ui->chooser_entry;
	text = gtk_entry_get_text (GTK_ENTRY (entry));
	
	if (g_utf8_strlen (text, -1) <= 0)
		return;
	
	treeview = data->ui->chooser_treeview;
	model = GTK_LIST_STORE (
		gtk_tree_view_get_model (GTK_TREE_VIEW (treeview)));
	
	gtk_list_store_append (model, &iter);
	gtk_list_store_set (model, &iter, CHOOSER_TICK_COL, TRUE,
			    CHOOSER_NAME_COL, text, -1);
	
	gtk_entry_set_text (GTK_ENTRY (entry), "");

	contacts_ui_update_groups_list (data);
}

void
contacts_disable_search_cb (GtkWidget *widget, ContactsData *data)
{
	/* save the old search string */
	g_free (data->search_string);
	data->search_string = g_strdup (gtk_entry_get_text (GTK_ENTRY (data->ui->search_entry)));
	data->search_enabled = FALSE;

	contacts_update_treeview (data);
}

void
contacts_enable_search_cb (GtkWidget *widget, ContactsData *data)
{
	if (data->search_string != NULL)
		gtk_entry_set_text (GTK_ENTRY (data->ui->search_entry), data->search_string);
	data->search_enabled = TRUE;

	contacts_update_treeview (data);
}

void
contacts_search_changed_cb (GtkWidget *widget, ContactsData *data)
{
	g_free (data->search_string);
	data->search_string =
		g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));

	gtk_widget_grab_focus (data->ui->search_entry);
	contacts_update_treeview (data);
}

void
contacts_chooser_toggle_cb (GtkCellRendererToggle * cell,
		   gchar * path_string, gpointer user_data)
{
	GtkTreeIter iter;
	GtkTreeModel *model = GTK_TREE_MODEL (user_data);

	gtk_tree_model_get_iter_from_string (model, &iter, path_string);
	if (gtk_cell_renderer_toggle_get_active (cell))
		gtk_list_store_set (GTK_LIST_STORE (model), &iter,
				    CHOOSER_TICK_COL, FALSE, -1);
	else
		gtk_list_store_set (GTK_LIST_STORE (model), &iter,
				    CHOOSER_TICK_COL, TRUE, -1);
}



void
contacts_selection_cb (GtkTreeSelection * selection, ContactsData *data)
{
	gint current_pane;

	/* if we have modified the previous contact, but not saved it, do so now */
	if (data->changed)
		e_book_async_commit_contact (data->book, data->contact, NULL, NULL);

	current_pane = gtk_notebook_get_current_page (GTK_NOTEBOOK (data->ui->main_notebook));

	/* Get the currently selected contact and update the contact summary */
	data->contact = contacts_contact_from_selection (selection, data->contacts_table);

	contacts_set_available_options (data, TRUE, GPOINTER_TO_INT (data->contact), GPOINTER_TO_INT (data->contact));

	if (current_pane == CONTACTS_CONTACT_PANE)
	{
		contacts_display_summary (data->contact, data);
	}

	if (current_pane == CONTACTS_GROUPS_PANE)
	{
		contacts_groups_pane_update_selection (
				selection,
				data);
	}
}

void
contacts_new_cb (GtkWidget *source, ContactsData *data)
{
  contacts_contact_pane_set_contact (CONTACTS_CONTACT_PANE (data->ui->contact_pane), NULL);
  contacts_contact_pane_set_editable (CONTACTS_CONTACT_PANE (data->ui->contact_pane),
                                      TRUE);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (data->ui->main_notebook), CONTACTS_CONTACT_PANE);
}

static void
contacts_show_contact_pane (ContactsData *data, gboolean editable)
{
	data->contact = contacts_get_selected_contact (data,
						       data->contacts_table);
	if (data->contact) {
		data->changed = FALSE;
		contacts_contact_pane_set_contact (CONTACTS_CONTACT_PANE (data->ui->contact_pane),
                                                   data->contact);
		contacts_contact_pane_set_editable (CONTACTS_CONTACT_PANE (data->ui->contact_pane),
                                                   editable);
		gtk_notebook_set_current_page (GTK_NOTEBOOK (data->ui->main_notebook), CONTACTS_CONTACT_PANE);
	}
}

void
contacts_view_cb (GtkWidget *source, ContactsData *data)
{
	contacts_show_contact_pane (data, FALSE);
}

void
contacts_edit_cb (GtkWidget *source, ContactsData *data)
{
	contacts_show_contact_pane (data, TRUE);
}

void
contacts_treeview_edit_cb (GtkTreeView *treeview, GtkTreePath *arg1,
	GtkTreeViewColumn *arg2, ContactsData *data)
{
	contacts_edit_cb (GTK_WIDGET (treeview), data);
}

void
contacts_delete_cb (GtkWidget *source, ContactsData *data)
{
	GtkWidget *dialog, *main_window;
	gint result, count_selected;
	EContact *contact;
	GList *widgets, *selected_paths, *current_path;
	GList *contact_list = NULL;
	const gchar *name;
	gchar *message = NULL;
	GtkTreeModel *model;

	GtkWidget *widget;
	GtkTreeSelection *selection;

	widget = data->ui->contacts_treeview;
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
	count_selected = gtk_tree_selection_count_selected_rows (selection);

	if (count_selected < 2) {
		contact = contacts_get_selected_contact (data,
						data->contacts_table);
		if (!contact) return;

		name = e_contact_get_const (contact, E_CONTACT_FULL_NAME);
		if ((!name) || (g_utf8_strlen (name, 4) <= 0))
			name = _("Unknown");

		contact_list = g_list_prepend (contact_list, e_contact_get (contact, E_CONTACT_UID));

		message = g_strdup_printf (_("Are you sure you want to delete "\
				 "'%s'?"), name);
	}
	else {
		message = g_strdup_printf (_("Are you sure you want to delete "\
					"%d contacts?"), count_selected);

		selected_paths = gtk_tree_selection_get_selected_rows (selection, &model);
		
		current_path = g_list_first (selected_paths);
		while (current_path) {
			contact = contacts_contact_from_tree_path (model, current_path->data, data->contacts_table);
			contact_list = g_list_prepend (contact_list, e_contact_get(contact, E_CONTACT_UID));
			current_path = g_list_next (current_path);
		}

		g_list_foreach (selected_paths, (GFunc)gtk_tree_path_free, NULL);
		g_list_free (selected_paths);
	}


	main_window = data->ui->main_window;
	dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
					 0, GTK_MESSAGE_QUESTION,
					 GTK_BUTTONS_CANCEL,
					 message);
	gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_DELETE,
		GTK_RESPONSE_YES, NULL);

	widgets = contacts_set_widgets_desensitive (main_window);
	result = gtk_dialog_run (GTK_DIALOG (dialog));
	switch (result) {
		case GTK_RESPONSE_YES:
			/* TODO: add progress indicator and callback here */
			e_book_async_remove_contacts (data->book, contact_list, NULL, NULL);
			break;
		default:
			break;
	}
	g_list_free (contact_list);
	gtk_widget_destroy (dialog);
	contacts_set_widgets_sensitive (widgets);
}

void
contacts_import (ContactsData *data, const gchar *filename, gboolean do_confirm)
{
	gchar *vcard_string, *card;
	gchar **vcard_array = NULL;
	gint i = 0;
#ifdef HAVE_GNOMEVFS
	int size;
	GnomeVFSResult vfs_result;

	vfs_result = gnome_vfs_read_entire_file (
		filename, &size, &vcard_string);
	if (vfs_result == GNOME_VFS_OK) {
#else
	if (g_file_get_contents (
		filename, &vcard_string, NULL, NULL)) {
#endif
		vcard_array = g_strsplit (vcard_string, "END:VCARD\n", 0);
		while ((card = vcard_array[i++]))
		{
			EContact *contact = NULL;
			gchar *str1;
			/* make sure we haven't reached the end */
			if (strlen (card) < 1)
				continue;
			/* END:VCARD is stripped by strsplit, so add it again here */
			str1 = g_strconcat (card, "END:VCARD", NULL);
			contact = e_contact_new_from_vcard (str1);
			g_free (str1);

			if (contact) {
				gint result = GTK_RESPONSE_YES;
				if (do_confirm) {
					GtkWidget *dialog, *main_window;
					GList *widgets;
					
					main_window = data->ui->main_window;
					dialog = gtk_message_dialog_new (
						GTK_WINDOW (main_window),
						0, GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_NONE,
						_("Would you like to import contact "\
						"'%s'?"),
						(const char *)e_contact_get_const (
							contact, E_CONTACT_FULL_NAME));
					gtk_dialog_add_buttons (GTK_DIALOG (dialog),
						_("_Show contact"), GTK_RESPONSE_NO,
						_("_Import contact"), GTK_RESPONSE_YES,
						NULL);
					widgets = contacts_set_widgets_desensitive (
						main_window);
					result = gtk_dialog_run (GTK_DIALOG (dialog));
					gtk_widget_destroy (dialog);
					contacts_set_widgets_sensitive (widgets);
					g_list_free (widgets);
				}
				if (result == GTK_RESPONSE_YES) {
					GList *lcontact =
						g_list_prepend (NULL, contact);
					/* Add contact to db and select it */
					/* TODO: add progress indicator and callback here */
					e_book_async_add_contact (data->book, contact,
								   NULL, NULL);
					/* Maually trigger the added callback so that
					 * the contact can be selected.
					 */
					#if 0
					contacts_added_cb (data->book_view, lcontact,
						data);
					contacts_set_selected_contact (data->xml,
						(const gchar *)e_contact_get_const (
							contact, E_CONTACT_UID));
					#endif
					g_list_free (lcontact);
				} else {
					contacts_display_summary (contact, data);
					contacts_set_available_options (
						data, TRUE, FALSE, FALSE);
				}
				g_object_unref (contact);
			}
		}
		g_free (vcard_string);
		g_strfreev (vcard_array);
	}
#ifdef HAVE_GNOMEVFS
	else {
		g_warning ("Error loading '%s': %s",
			filename, gnome_vfs_result_to_string (vfs_result));
	}
#endif
}

void
contacts_import_cb (GtkWidget *source, ContactsData *data)
{
	GList *widgets;
	GtkFileFilter *filter;
	GtkWidget *main_window =
		data->ui->main_window;
	GtkWidget *dialog = gtk_file_chooser_dialog_new (
		_("Import Contact"),
		GTK_WINDOW (main_window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN,
		GTK_RESPONSE_ACCEPT,
		NULL);

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_mime_type (filter, "text/x-vcard");
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), filter);
#ifdef HAVE_GNOMEVFS
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), FALSE);
#endif
	
	widgets = contacts_set_widgets_desensitive (main_window);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar *filename = gtk_file_chooser_get_filename 
					(GTK_FILE_CHOOSER (dialog));
		if (filename) {
			contacts_import (data, filename, FALSE);
			g_free (filename);
		}
	}
	
	contacts_set_widgets_sensitive (widgets);
	gtk_widget_destroy (dialog);
	g_list_free (widgets);
}

void
contacts_export (ContactsData *data, const gchar *filename)
{
	char *vcard = e_vcard_to_string (
		E_VCARD (data->contact), EVC_FORMAT_VCARD_30);
		
	if (vcard) {
#ifdef HAVE_GNOMEVFS
		GnomeVFSHandle *file;
		GnomeVFSFileSize bytes_written;
		if (gnome_vfs_open (&file, filename, GNOME_VFS_OPEN_WRITE) ==
		    GNOME_VFS_OK) {
			if (gnome_vfs_write (file, vcard, strlen (vcard),
			    &bytes_written) != GNOME_VFS_OK)
				g_warning ("Writing to '%s' failed, %lld bytes "
					"written", filename,
					bytes_written);
			gnome_vfs_close (file);
		}
#else
		FILE *file = fopen (filename, "w");
		if (file) {
			fputs (vcard, file);
			fclose (file);
		}
#endif
		g_free (vcard);
	}
}

void
contacts_export_cb (GtkWidget *source, ContactsData *data)
{
	GList *widgets;
	GtkWidget *main_window =
		data->ui->main_window;
	GtkWidget *dialog = gtk_file_chooser_dialog_new (
		_("Export Contact"),
		GTK_WINDOW (main_window),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE,
		GTK_RESPONSE_ACCEPT,
		NULL);
	
	/* Gtk 2.8 feature */
/*	gtk_file_chooser_set_do_overwrite_confirmation (
		GTK_FILE_CHOOSER (dialog), TRUE);*/
#ifdef HAVE_GNOMEVFS
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), FALSE);
#endif

	widgets = contacts_set_widgets_desensitive (main_window);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar *filename = gtk_file_chooser_get_filename 
					(GTK_FILE_CHOOSER (dialog));
		if (filename) {
#ifdef HAVE_GNOMEVFS
			GnomeVFSURI *uri = gnome_vfs_uri_new (filename);
			if (gnome_vfs_uri_exists (uri))
#else
			if (g_file_test (
			    filename, G_FILE_TEST_EXISTS))
#endif
			{
				GtkWidget *button, *image;
				GtkWidget *overwrite_dialog =
				gtk_message_dialog_new_with_markup (
					GTK_WINDOW (dialog),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_CANCEL,
                                        /* TODO: make it nicer for i18n */
					_("<big><b>The file \"%s\""
					" already exists.\n"
					"Do you want to replace it?</b></big>"),
					filename);
				gtk_message_dialog_format_secondary_markup (
					GTK_MESSAGE_DIALOG (overwrite_dialog),
					_("Replacing it will overwrite its "
					"contents."));
				button = gtk_dialog_add_button (
					GTK_DIALOG (overwrite_dialog),
					_("_Replace"),
					GTK_RESPONSE_OK);
				image = gtk_image_new_from_stock (
					GTK_STOCK_SAVE_AS,
					GTK_ICON_SIZE_BUTTON);
				gtk_button_set_image (
					GTK_BUTTON (button), image);
				
				if (gtk_dialog_run (
				     GTK_DIALOG (overwrite_dialog)) ==
				      GTK_RESPONSE_OK)
					contacts_export (data, filename);
				
				gtk_widget_destroy (overwrite_dialog);
			} else
				contacts_export (data, filename);
			
			g_free (filename);
#ifdef HAVE_GNOMEVFS
			gnome_vfs_uri_unref (uri);
#endif
		}
	}
	
	contacts_set_widgets_sensitive (widgets);
	gtk_widget_destroy (dialog);
	g_list_free (widgets);
}

void
contacts_edit_menu_activate_cb (GtkWidget *widget, ContactsData *data)
{
	gboolean can_cutcopy, can_paste;

	widget = data->ui->main_window;
	widget = gtk_window_get_focus (GTK_WINDOW (widget));
	if ((GTK_IS_EDITABLE (widget) || GTK_IS_TEXT_VIEW (widget) || GTK_IS_LABEL (widget)))
	{
		GtkClipboard *clip;
		clip = gtk_clipboard_get_for_display (gtk_widget_get_display (widget), GDK_SELECTION_CLIPBOARD);
		can_paste = gtk_clipboard_wait_is_text_available (clip);

		if (GTK_IS_EDITABLE (widget))
			can_cutcopy = gtk_editable_get_selection_bounds (GTK_EDITABLE (widget), NULL, NULL);
		else if (GTK_IS_TEXT_VIEW (widget))
		{
			GtkTextBuffer *buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
			can_cutcopy = gtk_text_buffer_get_selection_bounds (buf, NULL, NULL);
		}
		else if (GTK_IS_LABEL (widget))
			can_cutcopy = gtk_label_get_selection_bounds (GTK_LABEL (widget), NULL, NULL);
		else
			/* if we can't work out whether we can cut/copy, assume we can */
			can_cutcopy = TRUE;
	}
	else
		can_cutcopy = can_paste = FALSE;

	widget = data->ui->cut_menuitem;
	gtk_widget_set_sensitive (widget, can_cutcopy);
	widget = data->ui->copy_menuitem;
	gtk_widget_set_sensitive (widget, can_cutcopy);
	widget = data->ui->paste_menuitem;
	gtk_widget_set_sensitive (widget, can_paste);
}

void
contacts_copy_cb (GtkWindow *main_window)
{
	GtkWidget *widget = gtk_window_get_focus (main_window);

	if (widget) {
		if (GTK_IS_EDITABLE (widget))
			gtk_editable_copy_clipboard (GTK_EDITABLE (widget));
		else if (GTK_IS_TEXT_VIEW (widget)) {
			GtkTextBuffer *buffer = gtk_text_view_get_buffer (
				GTK_TEXT_VIEW (widget));
			gtk_text_buffer_copy_clipboard (buffer,
				gtk_clipboard_get (GDK_SELECTION_CLIPBOARD));
		} else if (GTK_IS_LABEL (widget)) {
			gint start, end;
			if (gtk_label_get_selection_bounds (GTK_LABEL (widget),
							    &start, &end)) {
				const gchar *text =
					gtk_label_get_text (GTK_LABEL (widget));
				gchar *start_text =
					g_utf8_offset_to_pointer (text, start);
				gchar *copy_text =
					g_strndup (start_text, end-start);
				gtk_clipboard_set_text (
				    gtk_clipboard_get (GDK_SELECTION_CLIPBOARD),
				    copy_text, end-start);
				g_free (copy_text);
			}
		}
	}
}

void
contacts_cut_cb (GtkWindow *main_window)
{
	GtkWidget *widget = gtk_window_get_focus (main_window);
	
	if (widget) {
		if (GTK_IS_EDITABLE (widget))
			gtk_editable_cut_clipboard (GTK_EDITABLE (widget));
		else if (GTK_IS_TEXT_VIEW (widget)) {
			GtkTextBuffer *buffer = gtk_text_view_get_buffer (
				GTK_TEXT_VIEW (widget));
			gtk_text_buffer_cut_clipboard (buffer,
				gtk_clipboard_get (GDK_SELECTION_CLIPBOARD),
				TRUE);
		} else
			contacts_copy_cb (main_window);
	}
}

void
contacts_paste_cb (GtkWindow *main_window)
{
	GtkWidget *widget = gtk_window_get_focus (main_window);

	if (widget) {
		if (GTK_IS_EDITABLE (widget))
			gtk_editable_paste_clipboard (GTK_EDITABLE (widget));
		else if (GTK_IS_TEXT_VIEW (widget)) {
			GtkTextBuffer *buffer = gtk_text_view_get_buffer (
				GTK_TEXT_VIEW (widget));
			gtk_text_buffer_paste_clipboard (buffer,
				gtk_clipboard_get (GDK_SELECTION_CLIPBOARD),
				NULL, TRUE);
		}
	}
}

void
contacts_about_cb (GtkWidget *parent)
{
	gchar *authors[] = {"Chris Lord <chris@o-hand.com>", NULL};
	/* Translators: please translate this as your own name and optionally email
	   like so: "Your Name <your@email.com>" */
	const gchar *translator_credits = _("translator-credits");
	GdkPixbuf *icon;

	icon = gdk_pixbuf_new_from_file (DATADIR"/pixmaps/oh-contacts.png", NULL);
	gtk_show_about_dialog (GTK_WINDOW (parent),
		"name", GETTEXT_PACKAGE,
		"version", VERSION,
		"authors", authors,
		"logo", icon,
		"website", "http://projects.o-hand.com/contacts/",
		"copyright", "(c) 2006 OpenedHand Ltd",
		"translator-credits", translator_credits,
		NULL);

	if (icon != NULL)
		g_object_unref (icon);
}

gboolean
contacts_treeview_search_cb (GtkWidget *search_entry, GdkEventKey *event,
	GtkTreeView *treeview)
{
	gtk_widget_event (search_entry, (GdkEvent *)event);
	gtk_entry_set_position (GTK_ENTRY (search_entry), -1);
	
	return FALSE;
}

gboolean
contacts_is_row_visible_cb (GtkTreeModel * model, GtkTreeIter * iter,
			    GHashTable *contacts_table)
{
	gboolean result = FALSE;
	gchar *group = NULL;
	GList *groups, *g;
	gchar *uid;
	EContactListHash *hash;
	const gchar *search_string;
	ContactsData *data;

	/* Check if the contact is in the currently selected group. */
	gtk_tree_model_get (model, iter, CONTACT_UID_COL, &uid, -1);
	if (!uid) return FALSE;
	hash = g_hash_table_lookup (contacts_table, uid);
	g_free (uid);
	if (!hash || !hash->contact) return FALSE;
	data = hash->contacts_data;

	if (data->selected_group != NULL
		&& !str_equal (_("All"), data->selected_group)
		&& !str_equal(_("Search Results"), data->selected_group))
	{
		groups = e_contact_get (hash->contact, E_CONTACT_CATEGORY_LIST);
		if ((group = data->selected_group)) {
			for (g = groups; g; g = g->next) {
				if (str_equal (group, g->data))
					result = TRUE;
				g_free (g->data);
			}
			if (groups)
				g_list_free (groups);
		} else
			result = TRUE;
		if (!result)
			return FALSE;
	}
	else
		result = TRUE;

	/* Search for any occurrence of the string in the search box in the 
	 * contact file-as name; if none is found, row isn't visible. Ignores 
	 * empty searches.
	 */
	if (data->search_enabled
		|| str_equal(_("Search Results"), data->selected_group)) {
		search_string = data->search_string;
		if ((search_string) &&
		    (g_utf8_strlen (search_string, -1) > 0)) {
			gchar *name_string;
			gtk_tree_model_get (model, iter, CONTACT_NAME_COL,
				&name_string, -1);
			if (name_string) {
				gunichar *isearch =
				    kozo_utf8_strcasestrip (search_string);
				if (!kozo_utf8_strstrcasestrip
				    (name_string, isearch))
					result = FALSE;
				g_free (name_string);
				g_free (isearch);
			}
		}
	} else if ((data->ui->symbols_radiobutton)){
		gint i;
		gchar *name, *uname;
		gunichar c;
		GSList *b, *buttons = gtk_radio_button_get_group (
			GTK_RADIO_BUTTON (data->ui->symbols_radiobutton));
		
		/* Find the active radio button */
		for (b = buttons, i = 0; b; b = b->next, i++)
			if (gtk_toggle_button_get_active (
				GTK_TOGGLE_BUTTON (b->data))) break;
		
		gtk_tree_model_get (model, iter, CONTACT_NAME_COL,
			&name, -1);
		uname = g_utf8_strup (name, -1);
		c = g_utf8_get_char (uname);
		
		switch (i) {
			case 4 :
				if (c >= 'A' || c <= 'Z')
					return FALSE;
				break;
			case 3 :
				if (c < 'A' || c > 'G')
					return FALSE;
				break;
			case 2 :
				if (c < 'H' || c > 'N')
					return FALSE;
				break;
			case 1 :
				if (c < 'O' || c > 'U')
					return FALSE;
				break;
			case 0 :
				if (c < 'V' || c > 'Z')
					return FALSE;
				break;
			default :
				g_warning ("Unknown search tab state %d", i);
				break;
		}
		
		g_free (name);
	}
	return result;
}

gint
contacts_sort_treeview_cb (GtkTreeModel * model, GtkTreeIter * a, GtkTreeIter * b,
			   gpointer user_data)
{
	gchar *string1, *string2;
	gint returnval;

	gtk_tree_model_get (model, a, CONTACT_NAME_COL, &string1, -1);
	gtk_tree_model_get (model, b, CONTACT_NAME_COL, &string2, -1);
	if (!string1) string1 = g_new0 (gchar, 1);
	if (!string2) string2 = g_new0 (gchar, 1);
	returnval = strcasecmp ((const char *) string1,
				(const char *) string2);
	g_free (string1);
	g_free (string2);

	return returnval;
}

