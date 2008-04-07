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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "sms-notes.h"
#include "sms-contacts.h"
#include "sms-utils.h"
#include <libjana/jana.h>
#include <libjana-ecal/jana-ecal.h>
#include <libmokoui2/moko-finger-scroll.h>
#include <libmokoui2/moko-search-bar.h>
#include <libmokoui2/moko-stock.h>
#include <libebook/e-book.h>
#include <string.h>

#include "moko-save-number.h"

static GdkColor alt_color;
static gboolean hidden = TRUE;
static gboolean open = FALSE;

enum {
	ALL_NOTES,
	SENT_NOTES,
	RECV_NOTES,
};

static gboolean
notes_treeview_scroll_to_bottom (SmsData *data)
{
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeViewColumn *column;
	
	column = gtk_tree_view_get_column (
		GTK_TREE_VIEW (data->notes_treeview), 0);
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (data->notes_treeview));
	path = gtk_tree_path_new_from_indices (
		gtk_tree_model_iter_n_children (model, NULL) - 1, -1);

	gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (data->notes_treeview),
		path, column, TRUE, 1.0, 0.0);
	
	gtk_tree_path_free (path);
	
	return FALSE;
}

static void
note_changed_cb (JanaStoreView *store_view, GList *components, SmsData *data)
{
	if (data->recipient_number) goto note_changed_cb_end;
	
	/* Set recipient icon */
	for (; components; components = components->next) {
		EContact *contact;
		const gchar *uid;
		JanaNote *note;
		
		GError *error = NULL;

		note = JANA_NOTE (components->data);
		data->recipient_number = jana_note_get_recipient (note);
		
		if (!data->recipient_number) continue;
		
		uid = g_hash_table_lookup (data->numbers,
			data->recipient_number);
		if ((!uid) || (!data->author_uid) ||
		    strcmp (uid, data->author_uid) == 0) {
			g_free (data->recipient_number);
			data->recipient_number = NULL;
			continue;
		}

		if (!e_book_get_contact (data->ebook, uid, &contact, &error)) {
			/* TODO: Unknown contact, probably */
		} else {
			if (data->recipient_icon)
				g_object_unref (data->recipient_icon);
			data->recipient_icon =
				sms_contact_load_photo (contact);
			g_object_unref (contact);
			break;
		}
	}

note_changed_cb_end:
	/* Remove handlers */
	if (data->recipient_number) {
		g_signal_handlers_disconnect_by_func (
			store_view, note_changed_cb, data);
	}
}

static void
note_progress_cb (JanaStoreView *store_view, gint percent, SmsData *data)
{
	if (percent != 100) return;
	
	notes_treeview_scroll_to_bottom (data);
	/*g_idle_add ((GSourceFunc)notes_treeview_scroll_to_bottom, data);*/

	g_signal_handlers_disconnect_by_func (store_view,
		note_progress_cb, data);
}

static gboolean
mark_messages_read_idle (SmsData *data)
{
	GtkTreePath *start_path, *end_path;
	
	data->notes_scroll_idle = 0;
	if (gtk_tree_view_get_visible_range (
	    GTK_TREE_VIEW (data->notes_treeview), &start_path, &end_path)) {
		
		do {
			JanaComponent *comp;
			GtkTreeIter iter;
			gchar *uid;

			if (!gtk_tree_model_get_iter (data->note_filter,
			     &iter, start_path)) break;
			
			gtk_tree_model_get (data->note_filter, &iter,
				JANA_GTK_NOTE_STORE_COL_UID, &uid, -1);
			
			comp = jana_store_get_component (data->notes, uid);
			if (comp && (!jana_utils_component_has_category (comp,
			    "Read"))) {
				jana_utils_component_insert_category (comp,
					"Read", 0);
				jana_store_modify_component (data->notes, comp);
			}
			
			g_free (uid);
			
			gtk_tree_path_next (start_path);
		} while (gtk_tree_path_compare (start_path, end_path) <= 0);
		
		gtk_tree_path_free (start_path);
		gtk_tree_path_free (end_path);
	}
	return FALSE;
}

static void
scroll_changed_cb (GtkAdjustment *adjust, SmsData *data)
{
	if (data->notes_scroll_idle) g_source_remove (data->notes_scroll_idle);
	data->notes_scroll_idle = g_timeout_add (500,
		(GSourceFunc)mark_messages_read_idle, data);
}

static void
page_shown (SmsData *data)
{
	JanaStoreView *store_view;
	GtkAdjustment *hadjust, *vadjust;
	GList *numbers, *n;

	gboolean found_match = FALSE;
	EContact *contact = NULL;

	if (!open) return;
	
	/* Attach to scrolling signals so we can mark messages as read */
	g_object_get (G_OBJECT (data->notes_treeview),
		"hadjustment", &hadjust, "vadjustment", &vadjust, NULL);
	g_signal_connect (hadjust, "changed",
		G_CALLBACK (scroll_changed_cb), data);
	g_signal_connect (hadjust, "value-changed",
		G_CALLBACK (scroll_changed_cb), data);
	g_signal_connect (vadjust, "changed",
		G_CALLBACK (scroll_changed_cb), data);
	g_signal_connect (vadjust, "value-changed",
		G_CALLBACK (scroll_changed_cb), data);
	
	/* Assign the recipient photo to the generic avatar icon, in case we 
	 * can't find it later.
	 */
	if (data->no_photo)
		data->recipient_icon = g_object_ref (data->no_photo);
	
	if (!(contact = sms_get_selected_contact (data))) {
		GList *u, *components = NULL;

		/* show the "add contact" button, as this contact is unknown */
		gtk_widget_show (GTK_WIDGET (data->save_contact_button));

		/* Assume the 'unknown' contact was selected */
		if (data->no_photo) {
			data->author_icon = g_object_ref (data->no_photo);
		}
		
		/* Manually feed the notes in - this is a bit naughty as if 
		 * they change, we won't be notified...
		 */
		for (u = data->unassigned_notes; u; u = u->next) {
			JanaComponent *component;
			const gchar *uid = (const gchar *)u->data;
			
			component = jana_store_get_component (data->notes, uid);
			if (component) components = g_list_prepend (
				components, component);
		}
		
		store_view = jana_store_get_view (data->notes);
		jana_gtk_note_store_set_view (JANA_GTK_NOTE_STORE (
			data->note_store), store_view);
		note_changed_cb (store_view, components, data);
		g_signal_emit_by_name (store_view, "added", components);
		
		for (u = components; u; u = u->next) {
			g_object_unref (G_OBJECT (u->data));
		}
		g_list_free (components);
		
		note_progress_cb (store_view, 100, data);
		
		return;
	}

	/* hide the "add contact" button, as this contact is already known */
	gtk_widget_hide (GTK_WIDGET (data->save_contact_button));

	data->author_icon = sms_contact_load_photo (contact);
	if ((!data->author_icon) && (data->no_photo))
		data->author_icon = g_object_ref (data->no_photo);
	
	store_view = jana_store_get_view (data->notes);
	numbers = hito_vcard_get_named_attributes (E_VCARD (contact), EVC_TEL);
	for (n = numbers; n; n = n->next) {
		gchar *number = hito_vcard_attribute_get_value_string (
			(EVCardAttribute *)n->data);
		
		if (!number) continue;
		
		jana_store_view_add_match (store_view,
			JANA_STORE_VIEW_AUTHOR, number);
		jana_store_view_add_match (store_view,
			JANA_STORE_VIEW_RECIPIENT, number);
		g_free (number);
		found_match = TRUE;
	}
	g_list_free (numbers);
	
	if (found_match) {
		jana_gtk_note_store_set_view (JANA_GTK_NOTE_STORE (
			data->note_store), store_view);
		g_signal_connect (store_view, "added",
			G_CALLBACK (note_changed_cb), data);
		g_signal_connect (store_view, "modified",
			G_CALLBACK (note_changed_cb), data);
		g_signal_connect (store_view, "progress",
			G_CALLBACK (note_progress_cb), data);
		jana_store_view_start (store_view);
	}
	g_object_unref (store_view);

	g_object_unref (contact);
}

static void
page_hidden (SmsData *data)
{
	GtkAdjustment *hadjust, *vadjust;

	if (data->notes_scroll_idle) g_source_remove (data->notes_scroll_idle);
	g_object_get (G_OBJECT (data->notes_treeview),
		"hadjustment", &hadjust, "vadjustment", &vadjust, NULL);
	g_signal_handlers_disconnect_by_func (hadjust, scroll_changed_cb, data);
	g_signal_handlers_disconnect_by_func (vadjust, scroll_changed_cb, data);
	
	jana_gtk_note_store_set_view (JANA_GTK_NOTE_STORE (
		data->note_store), NULL);
	if (data->author_uid) {
		g_free (data->author_uid);
		data->author_uid = NULL;
	}
	if (data->author_icon) {
		g_object_unref (data->author_icon);
		data->author_icon = NULL;
	}
	if (data->recipient_number) {
		g_free (data->recipient_number);
		data->recipient_number = NULL;
	}
	if (data->recipient_icon) {
		g_object_unref (data->recipient_icon);
		data->recipient_icon = NULL;
	}
}

void
sms_notes_refresh (SmsData *data)
{
	if (gtk_notebook_get_current_page (GTK_NOTEBOOK (data->notebook)) ==
	    SMS_PAGE_NOTES) {
		page_hidden (data);
		page_shown (data);
	} else {
		gtk_notebook_set_current_page (GTK_NOTEBOOK (data->notebook),
			SMS_PAGE_NOTES);
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

static void sms_notes_data_func (GtkTreeViewColumn *tree_column,
				 GtkCellRenderer *cell,
				 GtkTreeModel *model,
				 GtkTreeIter *iter,
				 SmsData *data)
{
	gchar *author, /**recipient,*/ *body, **categories;
	JanaTime *created, *modified;
	gboolean outgoing;
	gint i;
	
	gtk_tree_model_get (model, iter,
		JANA_GTK_NOTE_STORE_COL_AUTHOR, &author,
		/*JANA_GTK_NOTE_STORE_COL_RECIPIENT, &recipient,*/
		JANA_GTK_NOTE_STORE_COL_BODY, &body,
		JANA_GTK_NOTE_STORE_COL_CREATED, &created,
		JANA_GTK_NOTE_STORE_COL_MODIFIED, &modified,
		JANA_GTK_NOTE_STORE_COL_CATEGORIES, &categories,
		-1);

	outgoing = FALSE;
	if (categories) {
		for (i = 0; categories[i]; i++) {
			if ((strcmp (categories[i], "Sent") == 0) ||
			    (strcmp (categories[i], "Sending") == 0)) {
				outgoing = TRUE;
				break;
			}
		}
	}
	
	/* Replace numbers with contact names */
	for (i = /*0*/ 1; i < 2; i++) {
		const gchar *uid;
		GtkTreeIter *iter;
		gchar *number = i ? author : /*recipient*/ NULL;
		gchar *name;
		
		if (!number) continue;
		
		uid = g_hash_table_lookup (data->numbers, number);
		if (!uid) continue;
		
		iter = g_hash_table_lookup (data->contacts, uid);
		if (!iter) continue;
		
		gtk_tree_model_get (data->contacts_store, iter,
			COL_NAME, &name, -1);
		if (name) {
			g_free (number);
			if (i) author = name;
			/*else recipient = name;*/
		}
	}
	
	g_object_set (cell,
		"author", author,
		"recipient", /*recipient*/ NULL,
		"body", body,
		"created", created,
		"modified", modified,
		"justify", outgoing ?
		      GTK_JUSTIFY_LEFT : GTK_JUSTIFY_RIGHT,
		"icon", outgoing ?
		      data->author_icon : data->recipient_icon,
		"categories", categories,
		NULL);
	
	g_strfreev (categories);
	g_free (author);
	/*g_free (recipient);*/
	g_free (body);
	if (created) g_object_unref (created);
	if (modified) g_object_unref (modified);
}

static void
global_note_added_cb (JanaStoreView *store_view, GList *components,
		      SmsData *data)
{
	for (; components; components = components->next) {
		SmsNoteCountData *ncdata;
		JanaNote *note;
		gchar *uid;
		gchar *key;
		
		if (!JANA_IS_NOTE (components->data)) continue;
		
		note = JANA_NOTE (components->data);
		
		if (jana_utils_component_has_category (
		    (JanaComponent *)note, "Sent") ||
		    jana_utils_component_has_category (
		    (JanaComponent *)note, "Sending"))
			key = jana_note_get_recipient (note);
		else
			key = jana_note_get_author (note);

		if (!key) continue;

		ncdata = g_hash_table_lookup (data->note_count, key);
		
		if (!ncdata) {
			ncdata = g_slice_new0 (SmsNoteCountData);
			g_hash_table_insert (
				data->note_count, key, ncdata);
		} else {
			g_free (key);
		}
		
		uid = jana_component_get_uid (
			JANA_COMPONENT (note));
		if (jana_utils_component_has_category (
		     JANA_COMPONENT (note), "Read")) {
			ncdata->read = g_list_prepend (
				ncdata->read, uid);
		} else {
			ncdata->unread = g_list_prepend (
				ncdata->unread, uid);
		}
	}
	
	if (!data->note_count_idle) data->note_count_idle =
		g_idle_add ((GSourceFunc)sms_contacts_note_count_update, data);
}

static void
global_note_modified_cb (JanaStoreView *store_view, GList *components,
			 SmsData *data)
{
	for (; components; components = components->next) {
		SmsNoteCountData *ncdata;
		JanaNote *note;
		gchar *uid;
		gint i;
		
		if (!JANA_IS_NOTE (components->data)) continue;
		
		note = JANA_NOTE (components->data);
		uid = jana_component_get_uid (JANA_COMPONENT (note));
		
		for (i = 0; i < 2; i++) {
			GList *u, *r;
			gchar *key = i ? jana_note_get_author (note) :
				jana_note_get_recipient (note);
			
			if (!key) continue;
			
			ncdata = g_hash_table_lookup (data->note_count, key);
			g_free (key);
			
			if (!ncdata) continue;
			
			/* Swap from read/unread lists if necessary */
			u = g_list_find_custom (ncdata->unread, uid,
				(GCompareFunc)strcmp);
			r = g_list_find_custom (ncdata->read, uid,
				(GCompareFunc)strcmp);
			if (jana_utils_component_has_category (
			    JANA_COMPONENT (note), "Read")) {
				if (u) {
					ncdata->read = g_list_prepend (
						ncdata->read, u->data);
					ncdata->unread = g_list_delete_link (
						ncdata->unread, u);
				}
			} else if (r) {
				ncdata->unread = g_list_prepend (
					ncdata->unread, r->data);
				ncdata->read = g_list_delete_link (
					ncdata->read, r);
			}
		}
		g_free (uid);
	}

	if (!data->note_count_idle) data->note_count_idle =
		g_idle_add ((GSourceFunc)sms_contacts_note_count_update, data);
}

static gboolean
global_note_removed_ghrfunc (gchar *key, SmsNoteCountData *value,
			     gchar *uid)
{
	GList *u, *r;
	
	u = g_list_find_custom (value->unread, uid, (GCompareFunc)strcmp);
	r = g_list_find_custom (value->read, uid, (GCompareFunc)strcmp);
	
	if (u) {
		g_free (u->data);
		value->unread = g_list_delete_link (value->unread, u);
	}
	
	if (r) {
		g_free (r->data);
		value->read = g_list_delete_link (value->read, r);
	}
	
	if ((!value->read) && (!value->unread))
		return TRUE;
	else
		return FALSE;
}

static void
global_note_removed_cb (JanaStoreView *store_view, GList *uids,
			SmsData *data)
{
	for (; uids; uids = uids->next) {
		g_hash_table_foreach_remove (data->note_count,
			(GHRFunc)global_note_removed_ghrfunc, uids->data);
	}

	if (!data->note_count_idle) data->note_count_idle =
		g_idle_add ((GSourceFunc)sms_contacts_note_count_update, data);
}

static void
store_opened_cb (JanaStore *store, SmsData *data)
{
	JanaStoreView *store_view;
	
	open = TRUE;
	if (!hidden) page_shown (data);

	/* Create and start global view to bind notes to contacts */
	store_view = jana_store_get_view (store);
	g_signal_connect (store_view, "added",
		G_CALLBACK (global_note_added_cb), data);
	g_signal_connect (store_view, "modified",
		G_CALLBACK (global_note_modified_cb), data);
	g_signal_connect (store_view, "removed",
		G_CALLBACK (global_note_removed_cb), data);
	jana_store_view_start (store_view);
}

static void
free_count_data (SmsNoteCountData *data)
{
	while (data->read) {
		g_free (data->read->data);
		data->read = g_list_delete_link (data->read, data->read);
	}
	while (data->unread) {
		g_free (data->unread->data);
		data->unread = g_list_delete_link (data->unread, data->unread);
	}
	
	g_slice_free (SmsNoteCountData, data);
}


static void
save_contact_clicked_cb (GtkToolButton *button, SmsData *data)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *author, *recipient, **categories;
	gboolean sent;
	gint i;
  
	selection = gtk_tree_view_get_selection (
		GTK_TREE_VIEW (data->notes_treeview));
	if ((!selection) ||
	    (!gtk_tree_selection_get_selected (selection, &model, &iter)))
		return;
	
	gtk_tree_model_get (model, &iter,
		JANA_GTK_NOTE_STORE_COL_AUTHOR, &author,
		JANA_GTK_NOTE_STORE_COL_RECIPIENT, &recipient,
		JANA_GTK_NOTE_STORE_COL_CATEGORIES, &categories, -1);

	/* find out if the SMS was sent or received, so we can save the
	 * appropriate number */
	sent = FALSE;
	for (i = 0; categories[i]; i++)
	{
		if (!strcmp (categories[i], "Sent"))
		{
			sent = TRUE;
			break;
		}
	}

	if (sent)
		moko_save_number (recipient);
	else
		moko_save_number (author);
  
	g_free (author);
	g_free (recipient);
	g_strfreev (categories);
}

static void
new_clicked_cb (GtkToolButton *button, SmsData *data)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *author;
  
	selection = gtk_tree_view_get_selection (
		GTK_TREE_VIEW (data->notes_treeview));
	if ((!selection) ||
	    (!gtk_tree_selection_get_selected (selection, &model, &iter)))
        {
		/* TODO: find the current selected contact preferred number? */
		author = g_strdup ("");
	}
	else
	{
		gtk_tree_model_get (model, &iter,
			JANA_GTK_NOTE_STORE_COL_AUTHOR, &author, -1);
	}

	gtk_text_buffer_set_text (gtk_text_view_get_buffer (
		GTK_TEXT_VIEW (data->sms_textview)), author, -1);

	gtk_notebook_set_current_page (
			GTK_NOTEBOOK (data->notebook), SMS_PAGE_COMPOSE);
	g_free (author);
}

static void
forward_clicked_cb (GtkToolButton *button, SmsData *data)
{
	gchar *body;
	GtkTreeIter iter, citer;
	GtkTreeModel *model;
	GtkTreeSelection *selection, *cselection;

	/* Fill in compose box with message text and call new */
	selection = gtk_tree_view_get_selection (
		GTK_TREE_VIEW (data->notes_treeview));
	if ((!selection) ||
	    (!gtk_tree_selection_get_selected (selection, &model, &iter)))
		return;
	
	gtk_tree_model_get (model, &iter,
		JANA_GTK_NOTE_STORE_COL_BODY, &body, -1);
	gtk_text_buffer_set_text (gtk_text_view_get_buffer (
		GTK_TEXT_VIEW (data->sms_textview)), body, -1);
	g_free (body);
	
	cselection = gtk_tree_view_get_selection (
		GTK_TREE_VIEW (data->contacts_treeview));
	if (cselection)
		gtk_tree_selection_get_selected (cselection, NULL, &citer);
	
	if (sms_contact_picker_dialog (
	    data, "Choose a contact to forward to:")) {
		gtk_notebook_set_current_page (
			GTK_NOTEBOOK (data->notebook), SMS_PAGE_COMPOSE);
	} else {
		gtk_tree_selection_select_iter (cselection, &citer);
	}
}

static void
delete_clicked_cb (GtkToolButton *button, SmsData *data)
{
	gchar *uid;
	gint response;
	GtkTreeIter iter;
	GtkWidget *dialog;
	GtkTreeModel *model;
	JanaComponent *comp;
	GtkTreeSelection *selection;
	
	if (hidden) return;
	
	dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (data->window),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
		"Delete selected message?");
	gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL, GTK_STOCK_DELETE, GTK_RESPONSE_YES, NULL);
	
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (response != GTK_RESPONSE_YES) return;

	selection = gtk_tree_view_get_selection (
		GTK_TREE_VIEW (data->notes_treeview));
	if (!selection ||
	    !gtk_tree_selection_get_selected (selection, &model, &iter))
		return;
	
	gtk_tree_model_get (model, &iter,
		JANA_GTK_NOTE_STORE_COL_UID, &uid, -1);
	if (!uid) return;
	
	comp = jana_store_get_component (data->notes, uid);
	if (!comp) return;
	
	jana_store_remove_component (data->notes, comp);
	g_object_unref (comp);
	g_free (uid);
}

static void
search_toggled_cb (MokoSearchBar *bar, gboolean search_visible, SmsData *data)
{
	gtk_tree_model_filter_refilter (
		GTK_TREE_MODEL_FILTER (data->note_filter));
}

static void
search_text_changed_cb (MokoSearchBar *bar, GtkEditable *editable,
			SmsData *data)
{
	gtk_tree_model_filter_refilter (
		GTK_TREE_MODEL_FILTER (data->note_filter));
}

static void
search_combo_changed_cb (MokoSearchBar *bar, GtkComboBox *combo_box,
			 SmsData *data)
{
	gtk_tree_model_filter_refilter (
		GTK_TREE_MODEL_FILTER (data->note_filter));
}

static
gboolean notes_visible_func (GtkTreeModel *model, GtkTreeIter *iter,
				SmsData *data)
{
	if (moko_search_bar_search_visible (
	    MOKO_SEARCH_BAR (data->notes_search))) {
		const gchar *search;
		gboolean result;
		gchar *body;

		search = gtk_entry_get_text (moko_search_bar_get_entry (
			MOKO_SEARCH_BAR (data->notes_search)));
		
		if ((!search) || (search[0] == '\0')) return TRUE;
		
		/* Filter on search query */
		gtk_tree_model_get (model, iter, JANA_GTK_NOTE_STORE_COL_BODY,
			&body, -1);
		if (!body) return FALSE;
		
		/* FIXME: Use a proper UTF-8 casefold comparison here */
		result = strcasestr (body, search) ? TRUE : FALSE;
		g_free (body);
		
		return result;
	} else {
		gchar **categories;
		gboolean result;
		gint i;
		
		/* Filter on selected category */
		gint type = gtk_combo_box_get_active (
			moko_search_bar_get_combo_box (MOKO_SEARCH_BAR (
				data->notes_search))); 
		
		if ((type <= ALL_NOTES)/* || (!data->author_uid)*/) return TRUE;
		
		gtk_tree_model_get (model, iter,
			JANA_GTK_NOTE_STORE_COL_CATEGORIES, &categories, -1);
		result = (type == SENT_NOTES) ? FALSE : TRUE;
		for (i = 0; categories && categories[i]; i++) {
			if ((strcmp (categories[i], "Sent") == 0) ||
			    (strcmp (categories[i], "Sending") == 0)) {
				result = !result;
				break;
			}
		}
		if (categories) g_strfreev (categories);
		
		return result;
	}
}

static void
selection_changed_cb (GtkTreeSelection *selection, SmsData *data)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	
	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) {
		gtk_widget_set_sensitive (GTK_WIDGET (
			data->delete_button), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (
			data->forward_button), FALSE);
	} else {
		gtk_widget_set_sensitive (GTK_WIDGET (
			data->delete_button), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (
			data->forward_button), TRUE);
	}
}

GtkWidget *
sms_notes_page_new (SmsData *data)
{
	GtkWidget *scroll, *vbox, *notes_combo, *toolbar;
	GtkCellRenderer *renderer;
	GHashTable *colours_hash;
	GtkIconTheme *icon_theme;
	gint size;
	GtkToolItem *new_button;
	
	data->author_uid = NULL;
	data->author_icon = NULL;
	data->recipient_number = NULL;
	data->recipient_icon = NULL;
	data->note_count = g_hash_table_new_full (g_str_hash, g_str_equal,
		(GDestroyNotify)g_free, (GDestroyNotify)free_count_data);
	data->note_count_idle = 0;
	data->unassigned_notes = NULL;
	data->notes_scroll_idle = 0;
	
	/* Create note emblem hash-table */
	data->note_emblems = g_hash_table_new_full (g_str_hash, g_str_equal,
		NULL, (GDestroyNotify)g_object_unref);
	icon_theme = gtk_icon_theme_get_default ();
	/* FIXME: These are temporary icons */
	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &size, NULL);
	g_hash_table_insert (data->note_emblems, "Sent",
		gtk_icon_theme_load_icon (icon_theme, MOKO_STOCK_MAIL_SEND,
			size, 0, NULL));
	g_hash_table_insert (data->note_emblems, "Sending",
		gtk_icon_theme_load_icon (icon_theme, MOKO_STOCK_SMS_NEW,
			size, 0, NULL));
	g_hash_table_insert (data->note_emblems, "Rejected",
		gtk_icon_theme_load_icon (icon_theme, MOKO_STOCK_CALL_REJECT,
			size, 0, NULL));
	
	/* Create note store */
	data->notes = jana_ecal_store_new (JANA_COMPONENT_NOTE);
	g_signal_connect (data->notes, "opened",
		G_CALLBACK (store_opened_cb), data);
	
	/* Create model and filter */
	data->note_store = jana_gtk_note_store_new ();
	data->note_filter = gtk_tree_model_filter_new (data->note_store, NULL);
	gtk_tree_model_filter_set_visible_func ((GtkTreeModelFilter *)
		data->note_filter, (GtkTreeModelFilterVisibleFunc)
		notes_visible_func, data, NULL);
	
	/* Create a category-colour hash for the cell renderer */
	colours_hash = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_insert (colours_hash, "Sent", &alt_color);
	g_hash_table_insert (colours_hash, "Sending", &alt_color);
	
	/* Create treeview and cell renderer */
	data->notes_treeview = gtk_tree_view_new_with_model (data->note_filter);
	gtk_tree_view_set_headers_visible (
		GTK_TREE_VIEW (data->notes_treeview), FALSE);
	renderer = jana_gtk_cell_renderer_note_new ();
	g_object_set (renderer, "draw_box", TRUE, "show_recipient", TRUE,
		"category_icon_hash", data->note_emblems, NULL);
	gtk_tree_view_insert_column_with_data_func (
		GTK_TREE_VIEW (data->notes_treeview), 0, "Messages", renderer,
		(GtkTreeCellDataFunc)sms_notes_data_func, data, NULL);
	g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (
		data->notes_treeview)), "changed",
		G_CALLBACK (selection_changed_cb), data);
	
	/* create toolbar */
	toolbar = gtk_toolbar_new ();

  	/* New button */
	new_button = gtk_tool_button_new_from_stock (MOKO_STOCK_SMS_NEW);
	gtk_tool_item_set_expand (new_button, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), new_button, -1);
	g_signal_connect (new_button, "clicked",
		G_CALLBACK (new_clicked_cb), data);
  
	/* Forward button */
	data->forward_button = gtk_tool_button_new_from_stock (
		MOKO_STOCK_MAIL_FORWARD);
	gtk_tool_item_set_expand (data->forward_button, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data->forward_button, -1);
	
	/* Delete button */
	data->delete_button = gtk_tool_button_new_from_stock (
		GTK_STOCK_DELETE);
	gtk_tool_item_set_expand (data->delete_button, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data->delete_button, -1);
	gtk_widget_set_sensitive (GTK_WIDGET (data->delete_button), FALSE);

	/* Save contact button */
	data->save_contact_button = gtk_tool_button_new_from_stock (
		MOKO_STOCK_CONTACT_NEW);
	gtk_tool_item_set_expand (data->save_contact_button, TRUE);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data->save_contact_button, -1);
	gtk_widget_hide (GTK_WIDGET (data->save_contact_button));
	
	/* Create search bar */
	notes_combo = gtk_combo_box_new_text ();
	gtk_combo_box_append_text (GTK_COMBO_BOX (notes_combo), "All");
	gtk_combo_box_append_text (GTK_COMBO_BOX (notes_combo), "Sent");
	gtk_combo_box_append_text (GTK_COMBO_BOX (notes_combo), "Received");
	gtk_combo_box_set_active (GTK_COMBO_BOX (notes_combo), 0);
	
	data->notes_search = moko_search_bar_new_with_combo (
		GTK_COMBO_BOX (notes_combo));
	g_signal_connect (data->notes_search, "toggled",
		G_CALLBACK (search_toggled_cb), data);
	g_signal_connect (data->notes_search, "text_changed",
		G_CALLBACK (search_text_changed_cb), data);
	g_signal_connect (data->notes_search, "combo_changed",
		G_CALLBACK (search_combo_changed_cb), data);
	
	/* Pack widgets */
	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), data->notes_treeview);
	
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), data->notes_search, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
	gtk_widget_show_all (vbox);
	
	/* Add events for detecting whether the page has been hidden/shown */
	gtk_widget_add_events (data->notes_treeview, GDK_VISIBILITY_NOTIFY_MASK);
	g_signal_connect (data->notes_treeview, "visibility-notify-event",
		G_CALLBACK (visibility_notify_event_cb), data);
	g_signal_connect (data->notes_treeview, "notify::visible",
		G_CALLBACK (notify_visible_cb), data);
	g_signal_connect (vbox, "unmap",
		G_CALLBACK (unmap_cb), data);
	
	jana_store_open (data->notes);

	/* Connect to toolbar button-click signals */
	g_signal_connect (data->forward_button, "clicked",
		G_CALLBACK (forward_clicked_cb), data);
	g_signal_connect (data->delete_button, "clicked",
		G_CALLBACK (delete_clicked_cb), data);
	g_signal_connect (data->save_contact_button, "clicked",
		G_CALLBACK (save_contact_clicked_cb), data);

	return vbox;
}
