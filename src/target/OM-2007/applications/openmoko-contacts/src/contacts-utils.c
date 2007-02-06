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

#include <config.h>

#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libebook/e-book.h>

#include "contacts-utils.h"
#include "contacts-defs.h"
#include "contacts-main.h"

/* The following functions taken from
 * http://svn.o-hand.com/repos/kozo/server/src/kozo-utf8.c
 */
 /*****************************************************************************/
G_INLINE_FUNC char *
e_util_unicode_get_utf8 (const char *text, gunichar * out)
{
	*out = g_utf8_get_char (text);

	return (G_UNLIKELY (*out == (gunichar) - 1)) ? NULL :
	    g_utf8_next_char (text);
}

static gunichar
stripped_char (gunichar ch)
{
	GUnicodeType utype;

	utype = g_unichar_type (ch);

	switch (utype) {
	case G_UNICODE_CONTROL:
	case G_UNICODE_FORMAT:
	case G_UNICODE_UNASSIGNED:
	case G_UNICODE_COMBINING_MARK:
		/* Ignore those */
		return 0;
	default:
		/* Convert to lowercase, fall through */
		ch = g_unichar_tolower (ch);
	case G_UNICODE_LOWERCASE_LETTER:
		return ch;
	}
}

const char *
kozo_utf8_strstrcasestrip (const char *haystack, const gunichar * needle)
{
	gunichar unival, sc;
	const char *o, *p, *q;
	int npos;

	/* FIXME this is unoptimal */
	/* TODO use Bayer-Moore algorithm, if faster */
	o = haystack;
	for (p = e_util_unicode_get_utf8 (o, &unival);
	     p && unival; p = e_util_unicode_get_utf8 (p, &unival)) {
		sc = stripped_char (unival);
		if (sc) {
			/* We have valid stripped char */
			if (sc == needle[0]) {
				q = p;
				npos = 1;

				while (needle[npos]) {
					q = e_util_unicode_get_utf8 (q,
								     &unival);
					if (!q || !unival)
						return NULL;

					sc = stripped_char (unival);
					if ((!sc) || (sc != needle[npos]))
						break;

					npos++;
				}

				if (!needle[npos])
					return o;
			}
		}

		o = p;
	}

	return NULL;
}

gunichar *
kozo_utf8_strcasestrip (const char *str)
{
	gunichar *needle;
	gunichar unival, sc;
	int nlen, normlen;
	const char *p;
	char *norm;

	/* FIXME unoptimal, too many iterations */
	norm = g_utf8_normalize (str, -1, G_NORMALIZE_DEFAULT);
	normlen = g_utf8_strlen (norm, -1);
	if (G_UNLIKELY (normlen == 0)) {
		g_free (norm);
		return NULL;
	}

	/* Over-estimates length */
	needle = g_new (gunichar, normlen + 1);

	nlen = 0;
	for (p = e_util_unicode_get_utf8 (norm, &unival);
	     p && unival; p = e_util_unicode_get_utf8 (p, &unival)) {
		sc = stripped_char (unival);
		if (sc)
			needle[nlen++] = sc;
	}

	g_free (norm);

	/* NULL means there was illegal utf-8 sequence */
	if (!p)
		return NULL;

	/* If everything is correct, we have decomposed, lowercase, stripped 
	 * needle */
	needle[nlen] = 0;
	if (normlen == nlen)
		return needle;
	else
		return g_renew (gunichar, needle, nlen + 1);
}

/******************************************************************************/

/* List of always-available fields */
/* TODO: Revise 'supported' fields */
/* Note: PHOTO and CATEGORIES are special-cased (see contacts_edit_pane_show) */
static const ContactsField contacts_fields[] = {
	{ "FN", E_CONTACT_FULL_NAME, NULL, FALSE, 10, TRUE },
	{ "ORG", E_CONTACT_ORG, NULL, FALSE, 15, TRUE },
	{ "TEL", 0, N_("Phone"), FALSE, 20, FALSE },
	{ "EMAIL", 0, N_("Email"), FALSE, 30, FALSE },
	{ "ADR", 0, N_("Address"), FALSE, 40, FALSE },
	{ "NICKNAME", E_CONTACT_NICKNAME, NULL, FALSE, 110, TRUE },
	{ "URL", E_CONTACT_HOMEPAGE_URL, N_("Homepage"), FALSE, 120, FALSE },
	{ "NOTE", E_CONTACT_NOTE, NULL, TRUE, 130, TRUE },
	{ NULL }
};



static const ContactsStructuredField contacts_sfields[] = {
	{ "ADR", 0, N_("PO Box"), FALSE },
	{ "ADR", 1, N_("Ext."), TRUE },
	{ "ADR", 2, N_("Street"), TRUE },
	{ "ADR", 3, N_("Locality"), FALSE },
	{ "ADR", 4, N_("Region"), FALSE },
	{ "ADR", 5, N_("Post Code"), FALSE },
	{ "ADR", 6, N_("Country"), FALSE },
	{ NULL, 0, NULL, FALSE }
};

struct contacts_field_types_def {
	char *field;
	gchar **types;
};

static const struct contacts_field_types_def contacts_field_types[] = {
        /* TODO: can these be i18n-ized? */
	{ "TEL", (gchar*[]){"Home", "Msg", "Work", "Pref", "Voice", "Fax",
			    "Cell", "Video", "Pager", "BBS", "Modem", "Car",
			    "ISDN", "PCS", NULL }},
	{ "EMAIL", (gchar*[]){"Internet", "X400", "Pref", NULL}},
	{ "ADR", (gchar*[]){"Dom", "Intl", "Postal", "Parcel", "Home", "Work", "Pref", NULL}},
	{ NULL, NULL }
};

const gchar **
contacts_get_field_types (const gchar *attr_name)
{
	guint i;

	for (i = 0; contacts_field_types[i].field; i++) {
		if (strcmp (contacts_field_types[i].field, attr_name) == 0)
			return (const gchar **)contacts_field_types[i].types;
	}
	
	return NULL;
}

const ContactsStructuredField *
contacts_get_structured_field (const gchar *attr_name, guint field)
{
	guint i;
	
	for (i = 0; contacts_sfields[i].attr_name; i++) {
		if (strcmp (contacts_sfields[i].attr_name, attr_name) == 0) {
			if (contacts_sfields[i].field == field)
				return &contacts_sfields[i];
		}
	}
	
	return NULL;
}

guint
contacts_get_structured_field_size (const gchar *attr_name)
{
	guint i, size = 1;
	
	for (i = 0; contacts_sfields[i].attr_name; i++)
		if (strcmp (contacts_sfields[i].attr_name, attr_name) == 0)
			if (contacts_sfields[i].field+1 > size)
				size = contacts_sfields[i].field+1;
	
	return size;
}

const ContactsField *
contacts_get_contacts_field (const gchar *vcard_field)
{
	guint i;
	
	for (i = 0; (contacts_fields[i].vcard_field) && (vcard_field); i++) {
		if (strcmp (contacts_fields[i].vcard_field, vcard_field) == 0)
			return &contacts_fields[i];
	}
	
	return NULL;
}

const ContactsField *
contacts_get_contacts_fields ()
{
	return contacts_fields;
}

const gchar *
contacts_field_pretty_name (const ContactsField *field)
{
	if (field->pretty_name) {
		return gettext(field->pretty_name);
	} else if (field->econtact_field > 0) {
		return e_contact_pretty_name (field->econtact_field);
	} else
		return NULL;
}

EContact *
contacts_contact_from_tree_path (GtkTreeModel *model, GtkTreePath *path, GHashTable *contacts_table)
{
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter (model, &iter, path)) {
		gchar *uid;
		EContactListHash *hash;
		gtk_tree_model_get (model, &iter, CONTACT_UID_COL, &uid, -1);
		if (uid) {
			hash = g_hash_table_lookup (contacts_table, uid);
			g_free (uid);
			if (hash)
				return hash->contact;
		}
	}
	return NULL;
}

EContact *
contacts_contact_from_selection (GtkTreeSelection *selection,
				 GHashTable *contacts_table)
{
	GtkTreeModel *model;
	GList *selected_paths;
	GtkTreePath *path;
	EContact *result;
	
	if (!selection || !GTK_IS_TREE_SELECTION (selection))
		return NULL;

	selected_paths = gtk_tree_selection_get_selected_rows (selection, &model);
	if (!selected_paths)
		return NULL;
	path = selected_paths->data;

	result = contacts_contact_from_tree_path (model, path, contacts_table);

	g_list_foreach (selected_paths, (GFunc) gtk_tree_path_free, NULL);
	g_list_free (selected_paths);
	return result;
}

EContact *
contacts_get_selected_contact (ContactsData *data, GHashTable *contacts_table)
{
	GtkWidget *widget;
	GtkTreeSelection *selection;
	EContact *contact;

	/* Get the currently selected contact */
	widget = data->ui->contacts_treeview;
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
	
	if (!selection || !GTK_IS_TREE_SELECTION (selection))
		return NULL;
		
	contact = contacts_contact_from_selection (selection, contacts_table);
	
	return contact;
}

void
contacts_set_selected_contact (ContactsData *data, const gchar *uid)
{
	GtkTreeView *treeview;
	GtkTreeModel *model;
	GtkTreeIter iter;

	treeview = GTK_TREE_VIEW (
		data->ui->contacts_treeview);
	model = /*gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER (*/
		gtk_tree_view_get_model (treeview)/*))*/;
	
	if (!gtk_tree_model_get_iter_first (model, &iter)) return;
	
	do {
		const gchar *ruid;
		gtk_tree_model_get (model, &iter, CONTACT_UID_COL, &ruid, -1);
		if (strcmp (uid, ruid) == 0) {
			GtkTreeSelection *selection =
				gtk_tree_view_get_selection (treeview);
			if (selection)
				gtk_tree_selection_select_iter (
					selection, &iter);
			return;
		}
	} while (gtk_tree_model_iter_next (model, &iter));

}

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

GtkImage *
contacts_load_photo (EContact *contact)
{
	GtkImage *image = NULL;
	EContactPhoto *photo;
	
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
				GdkPixbuf *pixbuf =
				    gdk_pixbuf_loader_get_pixbuf (loader);
				if (pixbuf) {
					image = GTK_IMAGE (
						gtk_image_new_from_pixbuf (
							g_object_ref (pixbuf)));
				}
				g_object_unref (loader);
			}
		}
		e_contact_photo_free (photo);
	}
	return image ? image : GTK_IMAGE (gtk_image_new_from_icon_name 
					("stock_person", GTK_ICON_SIZE_DIALOG));
}

/* This removes any vCard attributes that are just "", or have no associated
 * value. Evolution tends to add empty fields like this a lot - as does
 * contacts, to show required fields (but it removes them after editing, via
 * this function).
 * TODO: This really doesn't need to be recursive.
 */
void
contacts_clean_contact (EContact *contact)
{
	GList *attributes, *c;

	attributes = e_vcard_get_attributes (E_VCARD (contact));
	for (c = attributes; c; c = c->next) {
		EVCardAttribute *a = (EVCardAttribute*)c->data;
		GList *values = e_vcard_attribute_get_values (a);
		gboolean remove = TRUE;
		for (; values; values = values->next) {
			if (g_utf8_strlen ((const gchar *)values->data, -1) > 0)
				remove = FALSE;
		}
		if (remove) {
			e_vcard_remove_attribute (E_VCARD (contact), a);
			contacts_clean_contact (contact);
			break;
		}
	}
}

gboolean
contacts_contact_is_empty (EContact *contact)
{
	GList *attributes, *c;
	
	attributes = e_vcard_get_attributes (E_VCARD (contact));
	for (c = attributes; c; c = c->next) {
		EVCardAttribute *a = (EVCardAttribute*)c->data;
		GList *values = e_vcard_attribute_get_values (a);
		for (; values; values = values->next) {
			if (g_utf8_strlen ((const gchar *)values->data, -1) > 0)
				return FALSE;
		}
	}
	
	return TRUE;
}

gchar *
contacts_string_list_as_string (GList *list, const gchar *separator,
				gboolean include_empty)
{
	gchar *old_string, *new_string;
	GList *c;
	
	if (!include_empty)
		for (; ((list) && (strlen ((const gchar *)list->data) == 0));
		     list = list->next);

	if (!list) return NULL;
	
	new_string = g_strdup (list->data);
	for (c = list->next; c; c = c->next) {
		if ((strlen ((const gchar *)c->data) > 0) || (include_empty)) {
			old_string = new_string;
			new_string = g_strdup_printf ("%s%s%s", new_string,
				separator, (const gchar *)c->data);
			g_free (old_string);
		}
	}
	
	return new_string;
}

GList *
contacts_get_types (GList *params)
{
	GList *list = NULL;

	for (; params; params = params->next) {
		EVCardAttributeParam *p = (EVCardAttributeParam *)params->data;
		if (strcmp ("TYPE", e_vcard_attribute_param_get_name (p)) == 0)
			list = g_list_append (list, p);
	}
	
	return list;
}

GList *
contacts_get_type_strings (GList *params)
{
	GList *list = NULL;

	for (; params; params = params->next) {
		EVCardAttributeParam *p = (EVCardAttributeParam *)params->data;
		if (strcmp ("TYPE", e_vcard_attribute_param_get_name (p)) == 0){
			GList *types = e_vcard_attribute_param_get_values (p);
			for (; types; types = types->next) {
				list = g_list_append (list, types->data);
			}
		}
	}
	
	return list;
}

void
contacts_choose_photo (GtkWidget *button, EContact *contact)
{
	GtkWidget *filechooser, *photo;
	GtkFileFilter *filter;
	gint result;
	GList *widgets;
	
	/* Get a filename */
	/* Note: I don't use the GTK_WINDOW cast as gtk_widget_get_ancestor
	 * can return NULL and this would probably throw a critical Gtk error.
	 */
	filechooser = gtk_file_chooser_dialog_new (_("Open image"),
						   (GtkWindow *)
						   gtk_widget_get_ancestor (
						   	button,
						   	GTK_TYPE_WINDOW),
						   GTK_FILE_CHOOSER_ACTION_OPEN,
						   GTK_STOCK_CANCEL,
						   GTK_RESPONSE_CANCEL,
						   GTK_STOCK_OPEN,
						   GTK_RESPONSE_ACCEPT,
						   _("No image"),
						   NO_IMAGE,
						   NULL);
	/* Set filter by supported EContactPhoto image types */
	filter = gtk_file_filter_new ();
	/* mime types taken from e-contact.c */
	gtk_file_filter_add_mime_type (filter, "image/gif");
	gtk_file_filter_add_mime_type (filter, "image/jpeg");
	gtk_file_filter_add_mime_type (filter, "image/png");
	gtk_file_filter_add_mime_type (filter, "image/tiff");
	gtk_file_filter_add_mime_type (filter, "image/ief");
	gtk_file_filter_add_mime_type (filter, "image/cgm");
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (filechooser), filter);

	/* If a file was selected, get the image and set the contact to that
	 * image.
	 */
	widgets = contacts_set_widgets_desensitive (
		gtk_widget_get_ancestor (button, GTK_TYPE_WINDOW));
	result = gtk_dialog_run (GTK_DIALOG (filechooser));
	if (result == GTK_RESPONSE_ACCEPT) {
		gchar *filename = gtk_file_chooser_get_filename 
					(GTK_FILE_CHOOSER (filechooser));
		if (filename) {
			if (contact) {
				EContactPhoto new_photo;
				guchar **data;
				int *length;
#if HAVE_PHOTO_TYPE
				new_photo.type = E_CONTACT_PHOTO_TYPE_INLINED;
				data = &new_photo.data.inlined.data;
				length = &new_photo.data.inlined.length;
				new_photo.data.inlined.mime_type = NULL;
#else
				data = &new_photo.data;
				length = &new_photo.length;
#endif
				if (g_file_get_contents (filename, 
							 (gchar **)data,
							 (gsize *)length,
							 NULL)) {
					e_contact_set (contact, E_CONTACT_PHOTO,
						       &new_photo);
					g_free (*data);
					/* Re-display contact photo */
					gtk_container_foreach (
						GTK_CONTAINER (button),
						(GtkCallback)gtk_widget_destroy,
						NULL);
					photo = GTK_WIDGET
						(contacts_load_photo (contact));
					gtk_container_add (
						GTK_CONTAINER (button),
						photo);
					gtk_widget_show (photo);
				}
			}
			g_free (filename);
		}
	} else if (result == NO_IMAGE) {
		if (contact && E_IS_CONTACT (contact)) {
			e_contact_set (contact, E_CONTACT_PHOTO, NULL);
			/* Re-display contact photo */
			gtk_container_foreach (GTK_CONTAINER (button),
				(GtkCallback)gtk_widget_destroy, NULL);
			photo = GTK_WIDGET (contacts_load_photo (contact));
			gtk_container_add (GTK_CONTAINER (button), photo);
			gtk_widget_show (photo);
		}
	}
	
	contacts_set_widgets_sensitive (widgets);
	gtk_widget_destroy (filechooser);
}

void
contacts_free_list_hash (gpointer data)
{
	EContactListHash *hash = (EContactListHash *)data;
	
	if (hash) {
		GtkListStore *model = GTK_LIST_STORE 
			(gtk_tree_model_filter_get_model 
			 (GTK_TREE_MODEL_FILTER (gtk_tree_view_get_model 
			  (GTK_TREE_VIEW (hash->contacts_data->ui->contacts_treeview)))));
		gtk_list_store_remove (model, &hash->iter);
		g_object_unref (hash->contact);
		g_free (hash);
	}
}

/* Helper method to harvest user-input from GtkContainer's */
GList *
contacts_entries_get_values (GtkWidget *widget, GList *list) {
	if (GTK_IS_ENTRY (widget)) {
		return g_list_append (list, g_strdup (
				gtk_entry_get_text (GTK_ENTRY (widget))));
	} else if (GTK_IS_COMBO_BOX_ENTRY (widget)) {
		return g_list_append (list, g_strdup (
			gtk_entry_get_text (
				GTK_ENTRY (GTK_BIN (widget)->child))));
	} else if (GTK_IS_TEXT_VIEW (widget)) {
		GtkTextIter start, end;
		GtkTextBuffer *buffer =
			gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
		gtk_text_buffer_get_start_iter (buffer, &start);
		gtk_text_buffer_get_end_iter (buffer, &end);
		return g_list_append (list, 
			gtk_text_buffer_get_text (buffer, &start, &end, FALSE));
	} else if (GTK_IS_CONTAINER (widget)) {
		GList *c, *children =
			gtk_container_get_children (GTK_CONTAINER (widget));
		for (c = children; c; c = c->next) {
			list = contacts_entries_get_values (
				GTK_WIDGET (c->data), list);
		}
		g_list_free (children);
	}
	
	return list;
}

gboolean
contacts_chooser (ContactsData *data, const gchar *title, const gchar *label_markup,
		  GList *choices, GList *chosen, gboolean allow_custom,
		  GList **results)
{
	GList *c, *d, *widgets;
	GtkWidget *label_widget;
	gboolean multiple_choice = chosen ? TRUE : FALSE;
	GtkWidget *dialog = data->ui->chooser_dialog;
	GtkTreeView *tree = GTK_TREE_VIEW (data->ui->chooser_treeview);
	GtkTreeModel *model = gtk_tree_view_get_model (tree);
	GtkWidget *add_custom = data->ui->chooser_add_hbox;
	gint dialog_code;
	gboolean returnval = FALSE;
	
	if (allow_custom)
		gtk_widget_show (add_custom);
	else
		gtk_widget_hide (add_custom);
	
	label_widget = data->ui->chooser_label;
	if (label_markup) {
		gtk_label_set_markup (GTK_LABEL (label_widget), label_markup);
		gtk_widget_show (label_widget);
	} else {
		gtk_widget_hide (label_widget);
	}
	
	gtk_list_store_clear (GTK_LIST_STORE (model));
	for (c = choices, d = chosen; c; c = c->next) {
		GtkTreeIter iter;
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter,
				    CHOOSER_NAME_COL,
				    (const gchar *)c->data, -1);
		if (multiple_choice) {
			if (d) {
				gboolean chosen =
					(gboolean)GPOINTER_TO_INT (d->data);
				gtk_list_store_set (GTK_LIST_STORE (model),
						    &iter, CHOOSER_TICK_COL,
						    chosen, -1);
				d = d->next;
			} else {
				gtk_list_store_set (GTK_LIST_STORE (model),
						    &iter, CHOOSER_TICK_COL,
						    FALSE, -1);
			}
		}
	}
	if (multiple_choice)
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column (
			tree, CHOOSER_TICK_COL), TRUE);
	else
		gtk_tree_view_column_set_visible (gtk_tree_view_get_column (
			tree, CHOOSER_TICK_COL), FALSE);
	
	gtk_window_set_title (GTK_WINDOW (dialog), title);
	
	widgets = contacts_set_widgets_desensitive (
		data->ui->main_window);
	dialog_code = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_hide (dialog);
	if (dialog_code == GTK_RESPONSE_OK) {
		if (multiple_choice) {
			GtkTreeIter iter;
			gboolean valid =
				gtk_tree_model_get_iter_first (model, &iter);
			
			while (valid) {
				gboolean selected;
				gtk_tree_model_get (model, &iter,
					CHOOSER_TICK_COL, &selected, -1);
				if (selected) {
					gchar *name;
					gtk_tree_model_get (model, &iter,
						CHOOSER_NAME_COL, &name, -1);
					*results =
						g_list_append (*results, name);
				}
			
				valid = gtk_tree_model_iter_next (model, &iter);
			}
			
			returnval = TRUE;
		} else {
			gchar *selection_name;
			GtkTreeSelection *selection =
				gtk_tree_view_get_selection (tree);
			GtkTreeIter iter;
			
			if (!gtk_tree_selection_get_selected (
				selection, NULL, &iter)) {
				returnval = FALSE;
			} else {
				gtk_tree_model_get (model, &iter,
					CHOOSER_NAME_COL, &selection_name, -1);
				*results = g_list_append (NULL, selection_name);				
				returnval = TRUE;
			}
		}
	}
	
	contacts_set_widgets_sensitive (widgets);
	
	return returnval;
}

static GList *
contacts_set_widget_desensitive_recurse (GtkWidget *widget, GList **widgets)
{
	if (GTK_IS_WIDGET (widget)) {
		gboolean sensitive;
		
		g_object_get (G_OBJECT (widget), "sensitive", &sensitive, NULL);
		if (sensitive) {
			gtk_widget_set_sensitive (widget, FALSE);
			*widgets = g_list_append (*widgets, widget);
		}
		
		if (GTK_IS_TABLE (widget) || GTK_IS_HBOX (widget) ||
		    GTK_IS_VBOX (widget) || GTK_IS_EVENT_BOX (widget)) {
			GList *c, *children = gtk_container_get_children (
				GTK_CONTAINER (widget));
			
			for (c = children; c; c = c->next) {
				contacts_set_widget_desensitive_recurse (
					c->data, widgets);
			}
			g_list_free (children);
		}
	}
	
	return *widgets;
}

GList *
contacts_set_widgets_desensitive (GtkWidget *widget)
{
	GList *list = NULL;
	
	contacts_set_widget_desensitive_recurse (widget, &list);
	
	return list;
}

void
contacts_set_widgets_sensitive (GList *widgets)
{
	GList *w;
	
	for (w = widgets; w; w = w->next) {
		gtk_widget_set_sensitive (GTK_WIDGET (w->data), TRUE);
	}
}
