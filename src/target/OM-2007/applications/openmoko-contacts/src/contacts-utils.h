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

#include <glib.h>
#include <gtk/gtk.h>
#include <libebook/e-book.h>
#include "contacts-defs.h"

char *e_util_unicode_get_utf8 (const char *text, gunichar * out);

const char *kozo_utf8_strstrcasestrip (const char *haystack,
				       const gunichar * needle);

gunichar *kozo_utf8_strcasestrip (const char *str);

const gchar **contacts_get_field_types (const gchar *attr_name);

const ContactsStructuredField *contacts_get_structured_field (
	const gchar *attr_name, guint field);

guint contacts_get_structured_field_size (const gchar *attr_name);

const ContactsField *contacts_get_contacts_field (const gchar *vcard_field);

const ContactsField *contacts_get_contacts_fields ();

const gchar *contacts_field_pretty_name (const ContactsField *field);

EContact *
contacts_contact_from_tree_path (GtkTreeModel *model, GtkTreePath *path,
				GHashTable *contacts_table);

EContact *contacts_contact_from_selection (GtkTreeSelection *selection,
					   GHashTable *contacts_table);

EContact *contacts_get_selected_contact (ContactsData *data,
					 GHashTable *contacts_table);
					 
void contacts_set_selected_contact (ContactsData *data, const gchar *uid);

GtkImage *contacts_load_photo (EContact *contact);

void contacts_clean_contact (EContact *contact);

gboolean contacts_contact_is_empty (EContact *contact);

gchar *contacts_string_list_as_string (GList *list, const gchar *separator,
				       gboolean include_empty);

GList *contacts_get_types (GList *params);

GList *contacts_get_type_strings (GList *params);

void contacts_choose_photo (GtkWidget *button, EContact *contact);

void contacts_free_list_hash (gpointer data);

GList *contacts_entries_get_values (GtkWidget *widget, GList *list);

gboolean contacts_chooser (ContactsData *data, const gchar *title,
			   const gchar *label_markup, GList *choices,
			   GList *chosen, gboolean allow_custom,
			   GList **results);

GList *contacts_set_widgets_desensitive (GtkWidget *widget);

void contacts_set_widgets_sensitive (GList *widgets);
