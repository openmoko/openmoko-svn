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
#include <libebook/e-book.h>
#include "contacts-defs.h"

void contacts_added_cb (EBookView *book_view, const GList *contacts,
			ContactsData *data);

void contacts_changed_cb (EBookView *book_view, const GList *contacts,
			  ContactsData *data);

void contacts_removed_cb (EBookView *book_view, const GList *ids,
			  ContactsData *data);

void contacts_sequence_complete_cb (EBookView *book_view, const GList *ids,
			  ContactsData *data);
