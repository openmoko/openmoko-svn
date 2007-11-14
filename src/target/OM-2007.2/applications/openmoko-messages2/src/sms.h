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

typedef struct {
	JanaStore *notes;
	JanaStoreView *notes_view;
	GtkTreeModel *contacts_store;
	GtkTreeModel *contacts_filter;
	GtkTreeModel *note_store;
	GtkTreeModel *note_filter;
	
	GtkWidget *window;
	GtkWidget *notebook;
	GtkToolItem *new_button;
	GtkToolItem *delete_all_button;
	GtkToolItem *delete_button;
	GtkWidget *contacts_treeview;
	GtkWidget *notes_combo;
	GtkWidget *sms_hbox;
	GtkWidget *sms_textview;
	
	gulong delete_all_handler;
	gulong delete_handler;
} SmsData;

typedef enum {
	SMS_PAGE_CONTACTS,
	SMS_PAGE_NOTES,
} SmsPage;

#endif /* _SMS_H */
 
