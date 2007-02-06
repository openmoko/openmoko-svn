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

#include <libebook/e-book.h>
#include <gtk/gtk.h>

#ifndef DEFS_H
#define DEFS_H

enum {
	ADR_POBOX = 0,
	ADR_EXT,	/* Multiple line */
	ADR_STREET,	/* Multiple line */
	ADR_LOCAL,
	ADR_REGION,
	ADR_CODE,
	ADR_COUNTRY
};

enum {
	CONTACT_NAME_COL,
	CONTACT_UID_COL,
	CONTACT_CELLPHONE_COL,
	CONTACT_LAST_COL
};

enum {
	CHOOSER_TICK_COL = 0,
	CHOOSER_NAME_COL
};

enum {
	CONTACTS_CONTACT_PANE = 0,
	CONTACTS_GROUPS_PANE
};

typedef struct {
	GtkWidget *chooser_add_hbox;
	GtkWidget *chooser_dialog;
	GtkWidget *chooser_entry;
	GtkWidget *chooser_label;
	GtkWidget *chooser_treeview;

	GtkWidget *contact_delete;
	GtkWidget *contact_export;
	GtkWidget *contact_menu;

	GtkWidget *contacts_import;
	GtkWidget *contacts_menu;
	GtkWidget *contacts_treeview;

	GtkWidget *new_menuitem;
	GtkWidget *copy_menuitem;
	GtkWidget *cut_menuitem;
	GtkWidget *delete_menuitem;
	GtkWidget *delete_button;
	GtkWidget *edit_menuitem;
	GtkWidget *edit_button;
	GtkWidget *edit_done_button;
	GtkWidget *edit_groups;
	GtkWidget *edit_menu;
	GtkWidget *main_menubar;
	GtkWidget *main_notebook;
	GtkWidget *main_window;
	GtkWidget *new_button;
	GtkWidget *paste_menuitem;
	GtkWidget *photo_image;
	GtkWidget *preview_header_hbox;

	GtkWidget *groups_vbox;

	GtkWidget *add_field_button;
	GtkWidget *remove_field_button;

	GtkWidget *search_entry;
	GtkWidget *search_entry_hbox;
	GtkWidget *search_hbox;
	GtkWidget *search_tab_hbox;
	GtkWidget *symbols_radiobutton;

	GtkWidget *contact_pane;
} ContactsUI;

typedef struct {
	EBook *book;
	EBookView *book_view;
	EContact *contact;
	ContactsUI *ui;
	GHashTable *contacts_table;
	GList *contacts_groups;
	GHashTable *groups_widgets_hash;
	gchar *selected_group;
	gboolean changed;
	gchar *file;
	GtkListStore *contacts_liststore;
	GtkTreeModelFilter *contacts_filter;
	gboolean initialising;
	gchar *search_string;
	gboolean search_enabled;
} ContactsData;

typedef struct {
	EContact *contact;
	GtkTreeIter iter;
	ContactsData *contacts_data;
} EContactListHash;

typedef struct {
	EContact *contact;
	EVCardAttribute *attr;
	GtkWidget *widget; /* The widget containing the data */
	gboolean *changed;
} EContactChangeData;

typedef struct {
	const gchar *attr_name;
	EVCardAttributeParam *param;
	gboolean *changed;
} EContactTypeChangeData;

typedef struct {
	const gchar *vcard_field;
	EContactField econtact_field; /* >0, gets used for pretty name */
	const gchar *pretty_name; /* Always takes priority over above */
	gboolean multi_line;
	guint priority;
	gboolean unique;
} ContactsField;

/* Contains structured field data
 * TODO: Replace this with a less ugly construct?
 */

typedef struct {
	const gchar *attr_name;
	guint field;
	const gchar *subfield_name;
	gboolean multiline;
} ContactsStructuredField;

#define REQUIRED 100	/* Contacts with priority <= REQUIRED have to be shown
			 * when creating a new contact.
			 */

#define NO_IMAGE 1	/* For the image-chooser dialog */

#define str_equal(x, y) (x != NULL && y != NULL && strcmp (x, y) == 0)

#endif
