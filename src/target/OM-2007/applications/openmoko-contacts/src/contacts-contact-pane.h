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

#ifndef _CONTACTS_CONTACT_PANE
#define _CONTACTS_CONTACT_PANE

#include <gtk/gtkvbox.h>
#include <libebook/e-book.h>

G_BEGIN_DECLS

#define CONTACTS_TYPE_CONTACT_PANE contacts_contact_pane_get_type()

#define CONTACTS_CONTACT_PANE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  CONTACTS_TYPE_CONTACT_PANE, ContactsContactPane))

#define CONTACTS_CONTACT_PANE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  CONTACTS_TYPE_CONTACT_PANE, ContactsContactPaneClass))

#define CONTACTS_IS_CONTACT_PANE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  CONTACTS_TYPE_CONTACT_PANE))

#define CONTACTS_IS_CONTACT_PANE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  CONTACTS_TYPE_CONTACT_PANE))

#define CONTACTS_CONTACT_PANE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  CONTACTS_TYPE_CONTACT_PANE, ContactsContactPaneClass))

typedef struct _ContactsContactPanePrivate ContactsContactPanePrivate;

typedef struct {
  GtkVBox parent;
  ContactsContactPanePrivate *priv;
} ContactsContactPane;

typedef struct {
  GtkVBoxClass parent_class;
  void (* fullname_changed) (ContactsContactPane *self, EContact *contact);
  void (* cell_changed) (ContactsContactPane *self, EContact *contact);
} ContactsContactPaneClass;

GType contacts_contact_pane_get_type (void);

GtkWidget* contacts_contact_pane_new (void);

void contacts_contact_pane_set_book_view (ContactsContactPane *pane, EBookView *view);
void contacts_contact_pane_set_contact (ContactsContactPane *pane, EContact *contact);
void contacts_contact_pane_set_editable (ContactsContactPane *pane, gboolean editable);

G_END_DECLS

#endif /* _CONTACTS_CONTACT_PANE */
