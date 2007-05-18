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

#include <config.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libmokojournal/moko-journal.h>

#ifndef _CONTACTS_HISTORY_H
#define _CONTACTS_HISTORY_H


G_BEGIN_DECLS

#define CONTACTS_TYPE_HISTORY contacts_history_get_type()

#define CONTACTS_HISTORY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	CONTACTS_TYPE_HISTORY, \
	ContactsHistory))

#define CONTACTS_HISTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	CONTACTS_TYPE_HISTORY, \
	ContactsHistoryClass))

#define CONTACTS_IS_HISTORY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	CONTACTS_TYPE_HISTORY))

#define CONTACTS_IS_HISTORY_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	CONTACTS_TYPE_HISTORY))

#define CONTACTS_HISTORY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	CONTACTS_TYPE_HISTORY, \
	ContactsHistoryClass))

typedef struct _ContactsHistory ContactsHistory;
typedef struct _ContactsHistoryClass ContactsHistoryClass;
typedef struct _ContactsHistoryPrivate ContactsHistoryPrivate;

struct _ContactsHistory
{
	GtkVBox         parent;
	
	/* private */
	ContactsHistoryPrivate   *priv;
};

struct _ContactsHistoryClass 
{
	/* private */
	GtkVBoxClass parent_class;

	void (*entry_activated) (ContactsHistory*, MokoJournalEntry*); 
	void (*_moko_reserved_1) (void);
	void (*_moko_reserved_2) (void);
	void (*_moko_reserved_3) (void);
	void (*_moko_reserved_4) (void);
}; 

GType contacts_history_get_type (void) G_GNUC_CONST;

GtkWidget* 
contacts_history_new (void);

void
contacts_history_update_uid (ContactsHistory *history, const gchar *uid);

G_END_DECLS

#endif /*CONTACTS_HISTORY_H*/

