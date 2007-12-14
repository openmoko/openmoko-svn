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

#ifndef _SMS_H
#define _SMS_H

#include <glib.h>
#include <gtk/gtk.h>
#include <libjana-gtk/jana-gtk.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <libebook/e-book.h>

typedef struct {
	GList *unread;	/* List of JanaNote uids for unread messages */
	GList *read;	/* The same for read messages */
	gint assigned;
} SmsNoteCountData;

typedef struct {
	JanaStore *notes;
	JanaStoreView *notes_view;
	GtkTreeModel *note_store;
	GtkTreeModel *note_filter;
	GHashTable *note_count;
	guint note_count_idle;
	GList *unassigned_notes;
	
	EBook *ebook;
	GtkTreeModel *contacts_store;
	GtkTreeModel *contacts_filter;
	GHashTable *contacts;
	GHashTable *numbers;
	
	GtkWidget *window;
	GtkWidget *notebook;
	GtkToolItem *new_button;
	GtkToolItem *delete_all_button;
	GtkToolItem *delete_button;

	GtkWidget *notes_combo;
	GdkPixbuf *author_icon;
	GdkPixbuf *recipient_icon;
	gchar *recipient_number;
	gchar *author_uid;
	GtkWidget *notes_treeview;
	guint notes_scroll_idle;

	GtkWidget *contacts_treeview;
	GtkWidget *contacts_combo;
	GdkPixbuf *no_photo;
	
	GtkWidget *sms_textview;
	GtkWidget *length_label;
	GtkWidget *contact_image;
	GtkWidget *contact_label;
	GtkWidget *number_combo;

	DBusGProxy *sms_proxy;
} SmsData;

typedef enum {
	SMS_PAGE_CONTACTS,
	SMS_PAGE_NOTES,
	SMS_PAGE_COMPOSE,
} SmsPage;

#endif /* _SMS_H */
 
