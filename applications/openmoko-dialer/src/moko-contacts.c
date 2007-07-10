/*
 *  moko-contacts; A rework of the dialers contact list, some e-book code taken
 *  from the orignal dialer contacts.c (written by tonyguan@fic-sh.com.cn)
 *
 *  Authored by OpenedHand Ltd <info@openedhand.com>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
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

#include <glib.h>

#include <libebook/e-book.h>

#include "moko-contacts.h"

G_DEFINE_TYPE (MokoContacts, moko_contacts, G_TYPE_OBJECT)

#define MOKO_CONTACTS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_CONTACTS, MokoContactsPrivate))

struct _MokoContactsPrivate
{
  GList *contacts;
  GList *entries;
};

static void
moko_contacts_add_contact (MokoContacts *contacts, EContact *e_contact)
{
  MokoContactsPrivate *priv;
  MokoContact *m_contact = NULL;
  const gchar *name;
  gint         i;

  g_return_if_fail (MOKO_IS_CONTACTS (contacts));
  g_return_if_fail (E_IS_CONTACT (e_contact));
  priv = contacts->priv;

  name = e_contact_get_const (e_contact, E_CONTACT_NAME_OR_ORG);
  if (!name || (g_utf8_strlen (name, -1) <= 0))
    name = "Unnamed";
    
  /* Create the contact & append to the list */
  m_contact = g_new0 (MokoContact, 1);
  m_contact->name = g_strdup (name);
  m_contact->uid = e_contact_get (e_contact, E_CONTACT_UID);
  m_contact->photo = NULL;

  priv->contacts = g_list_append (priv->contacts, m_contact);
   
  /* Now go through the numbers,creating MokoNumber for them */
  for (i = E_CONTACT_FIRST_PHONE_ID; i < E_CONTACT_LAST_PHONE_ID; i++)
  {
    MokoContactEntry  *entry;
    const gchar *phone;

    phone = e_contact_get_const (e_contact, i);
    if (phone)
    {
      entry = g_new0 (MokoContactEntry, 1);
      entry->desc = g_strdup (e_contact_field_name (i));
      entry->number = g_strdup (phone);
      entry->contact = m_contact;

      priv->entries = g_list_append (priv->entries, (gpointer)entry);
    }
  }
}

static void
on_ebook_contact_added (EBookView    *view, 
                        GList        *c_list, 
                        MokoContacts *contacts)
{
  MokoContactsPrivate *priv;
  GList *c;

  g_return_if_fail (MOKO_IS_CONTACTS (contacts));
  priv = contacts->priv;

  for (c = c_list; c != NULL; c = c->next)
    moko_contacts_add_contact (contacts, E_CONTACT (c->data));
}

/* GObject functions */
static void
moko_contacts_dispose (GObject *object)
{
  G_OBJECT_CLASS (moko_contacts_parent_class)->dispose (object);
}

static void
moko_contacts_finalize (GObject *contacts)
{
  G_OBJECT_CLASS (moko_contacts_parent_class)->finalize (contacts);
}


static void
moko_contacts_class_init (MokoContactsClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = moko_contacts_finalize;
  obj_class->dispose = moko_contacts_dispose;

  g_type_class_add_private (obj_class, sizeof (MokoContactsPrivate)); 
}


static void
moko_contacts_init (MokoContacts *contacts)
{
  MokoContactsPrivate *priv;
  EBook *book;
  EBookView *view;
  EBookQuery *query;
  GList *contact, *c;

  priv = contacts->priv = MOKO_CONTACTS_GET_PRIVATE (contacts);

  priv->contacts = priv->entries = NULL;

  query = e_book_query_any_field_contains ("");

  /* Open the system book and check that it is valid */
  book = e_book_new_system_addressbook (NULL);
  if (!book)
  {
    g_warning ("Failed to create system book\n");
    return;
  }
  if (!e_book_open (book, TRUE, NULL))
  {
    g_warning ("Failed to open system book\n");
    return;
  }
  if (!e_book_get_contacts (book, query, &contact, NULL))
  {
    g_warning ("Failed to get contacts from system book\n");
    return;
  }
  
  /* Go through the contacts, creating the contact structs, and entry structs*/
  for (c = contact; c != NULL; c = c->next)
  {
    moko_contacts_add_contact (contacts, E_CONTACT (c->data));
  }

  /* Connect to contact added signals */
  if (e_book_get_book_view (book, query, NULL, 0, &view, NULL))
  {
    g_signal_connect (G_OBJECT (view), "contacts-added",
                      G_CALLBACK (on_ebook_contact_added), (gpointer)contacts);
  }
}

MokoContacts*
moko_contacts_get_default (void)
{
  static MokoContacts *contacts = NULL;
  
  if (contacts == NULL)
    contacts = g_object_new (MOKO_TYPE_CONTACTS, 
                             NULL);

  return contacts;
}
