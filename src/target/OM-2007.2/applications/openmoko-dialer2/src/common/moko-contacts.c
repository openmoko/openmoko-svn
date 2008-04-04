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
#include <stdio.h>
#include <string.h>
#include <libebook/e-book.h>

#include "moko-contacts.h"

#include "dialer-defines.h"

G_DEFINE_TYPE (MokoContacts, moko_contacts, G_TYPE_OBJECT)

#define MOKO_CONTACTS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_CONTACTS, MokoContactsPrivate))
typedef struct _Digit Digit;

struct _Digit
{
  Digit *digits[11];
  Digit *parent;
  GList *results;

};

struct _MokoContactsPrivate
{
  EBook      *book;

  GList      *contacts;
  GList      *entries;
  GHashTable *prefixes;
  GHashTable *uids;
  Digit *start;
};

static Digit*
new_digit (Digit *parent)
{
  Digit *ret;
  gint i;
  
  ret = g_slice_new0 (Digit);
  ret->parent = parent;
  ret->results = NULL;

  for (i = 0; i <11; i++)
    ret->digits[i] = NULL;
  
  return ret;
}

/* Auto-complete data type */
static void
add_number (Digit **start, MokoContactEntry *entry)
{
  gint len, i;
  Digit *cur;

  if (*start == NULL)
    *start = new_digit (NULL);

  cur = *start;

  len = strlen  (entry->number);
  for (i = 0; i < len; i++)
  {
    gchar c = entry->number[i];
    gint n = c - '0';

    if (n < 0 || n > 9)
      n = 10;

    if (cur->digits[n])
    {
      cur = cur->digits[n];
      if (g_list_length (cur->results) < 3)
        cur->results = g_list_append (cur->results, entry);
      continue;
    }
    else
    {
      cur->digits[n] = new_digit (cur);
      cur = cur->digits[n];
      cur->results = g_list_append (cur->results, entry);
    }
  }
}

GList*
moko_contacts_fuzzy_lookup (MokoContacts *contacts, const gchar *number)
{
  MokoContactsPrivate *priv;
  gint len, i;
  Digit *cur;
  
  g_return_val_if_fail (MOKO_IS_CONTACTS (contacts), NULL);
  priv = contacts->priv;

  cur = priv->start;

  if (!cur)
    return NULL;
  
  if (!number)
    return NULL;

  len = strlen (number);

  for (i = 0; i < len; i++)
  {
    gchar c = number[i];
    gint n = c - '0';

    if (n < 0 || n > 9)
      n = 10;

    if (!cur->digits[n])
      return NULL;

    cur = cur->digits[n];
    if ((i+1) == len)
      return cur->results;
  }

  return NULL;
}


void
moko_contacts_get_photo (MokoContacts *contacts, MokoContact *m_contact)
{
  MokoContactsPrivate *priv;
  EContact *e_contact;
  EContactPhoto *photo;
  GError *err = NULL;
  GdkPixbufLoader *loader;
  
  g_return_if_fail (MOKO_IS_CONTACTS (contacts));
  g_return_if_fail (m_contact);
  priv = contacts->priv;
  
  if (!e_book_get_contact (priv->book, m_contact->uid, &e_contact, &err))
  {
    g_warning ("%s\n", err->message);
    m_contact->photo = gdk_pixbuf_new_from_file (PKGDATADIR"/person.png", NULL);
    if (m_contact->photo)
      g_object_ref (m_contact->photo); 
    return;
  }
  
  photo = e_contact_get (e_contact, E_CONTACT_PHOTO);
  if (!photo)
  {
    m_contact->photo = gdk_pixbuf_new_from_file (PKGDATADIR"/person.png", NULL);
    if (m_contact->photo)
      g_object_ref (m_contact->photo);
    return;
 
  }
  
  loader = gdk_pixbuf_loader_new ();
  gdk_pixbuf_loader_write (loader, 
                           photo->data.inlined.data,
                           photo->data.inlined.length,
                           NULL);
  gdk_pixbuf_loader_close (loader, NULL);
  m_contact->photo = gdk_pixbuf_loader_get_pixbuf (loader);

  if (GDK_IS_PIXBUF (m_contact->photo))
    g_object_ref (m_contact->photo);
  else 
  {
    m_contact->photo = gdk_pixbuf_new_from_file (PKGDATADIR"/person.png", NULL);
    if (m_contact->photo)
      g_object_ref (m_contact->photo); 
  }

  g_object_unref (loader);
  e_contact_photo_free (photo);
}

MokoContactEntry*
moko_contacts_lookup (MokoContacts *contacts, const gchar *number)
{
  MokoContactsPrivate *priv;
  MokoContactEntry *entry;

  g_return_val_if_fail (MOKO_IS_CONTACTS (contacts), NULL);
  g_return_val_if_fail (number, NULL);
  priv = contacts->priv;
  
  entry =  g_hash_table_lookup (priv->prefixes, number);

  if (entry && !GDK_IS_PIXBUF (entry->contact->photo))
    moko_contacts_get_photo (contacts, entry->contact);

  return entry;
}



/* This takes the raw number from econtact, and spits out a 'normalised' 
 * version which does not contain any ' ' or '-' charecters. The reason for
 * this is that when inputing numbers into the dialer, you cannot add these
 * characters, but you can in contacts, which means the strings will not match
 * and autocomplete will not work
 */
static gchar*
normalize (const gchar *string)
{
  gint len = strlen (string);
  gchar buf[len];
  gint i;
  gint j = 0;

  for (i = 0; i < len; i++)
  {
    gchar c = string[i];
    if (c != ' ' && c != '-')
    {
      buf[j] = c;
      j++;
    }
  }
  buf[j] = '\0';
  return g_strdup (buf);
}

/* Calbacks */
static void
moko_contacts_add_contact (MokoContacts *contacts, EContact *e_contact)
{
  MokoContactsPrivate *priv;
  MokoContact *m_contact = NULL;
  const gchar *name, *uid;
  GList *attributes, *params, *numbers;

  g_return_if_fail (MOKO_IS_CONTACTS (contacts));
  g_return_if_fail (E_IS_CONTACT (e_contact));
  priv = contacts->priv;

  uid = e_contact_get_const (e_contact, E_CONTACT_UID);
  if (g_hash_table_lookup (priv->uids, uid))
	  return;
  
  name = e_contact_get_const (e_contact, E_CONTACT_FULL_NAME);
  if (!name || (g_utf8_strlen (name, -1) <= 0))
    name = "Unknown";
  
  /* Create the contact & append to the list */
  m_contact = g_new0 (MokoContact, 1);
  m_contact->name = g_strdup (name);
  m_contact->uid = g_strdup (uid);
  m_contact->photo = NULL;

  priv->contacts = g_list_append (priv->contacts, m_contact);
  g_hash_table_insert (priv->uids,
                       g_strdup (uid), 
                       m_contact);

  /* Now go through the numbers,creating MokoNumber for them */
  for (attributes = e_vcard_get_attributes (E_VCARD(e_contact)); attributes; attributes = attributes->next)
  {
    MokoContactEntry  *entry;
    const gchar *phone;
    const char *attr;

    attr = e_vcard_attribute_get_name (attributes->data);
    if (!strcmp (attr, EVC_TEL))
    {
      for (numbers = e_vcard_attribute_get_values (attributes->data); numbers; numbers = numbers->next)
      {
        phone = numbers->data;
        if (phone)
        {
          entry = g_new0 (MokoContactEntry, 1);

          params = e_vcard_attribute_get_param (attributes->data, "TYPE");
          if (params)
            entry->desc = g_strdup (params->data);

          entry->number = normalize (phone);
          entry->contact = m_contact;

          priv->entries = g_list_append (priv->entries, (gpointer)entry);
          g_hash_table_insert (priv->prefixes, 
                               g_strdup (entry->number), 
                               (gpointer)entry);
          add_number (&priv->start, entry);
        }
      }
    }
  }
}

static void
on_ebook_contacts_added (EBookView    *view, 
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

static void
on_ebook_contacts_changed (EBookView    *view,
                           GList        *c_list,
                           MokoContacts *contacts)
{
  g_debug ("Contacts changed");
}

static void
on_ebook_contacts_removed (EBookView    *view,
                           GList        *c_list,
                           MokoContacts *contacts)
{
  g_debug ("Contacts removed");
}

/* GObject functions */
static void
moko_contacts_dispose (GObject *object)
{
  G_OBJECT_CLASS (moko_contacts_parent_class)->dispose (object);
}

static void
free_digit (Digit *digit)
{
  gint i;

  for (i = 0; i < 11; i++)
  {
    if (digit->digits[i])
      free_digit (digit->digits[i]);
  }
  g_list_free (digit->results);
  g_slice_free (Digit, digit);
}

static void
moko_contacts_finalize (GObject *contacts)
{
  MokoContactsPrivate *priv;
  GList *l;
  
  g_return_if_fail (MOKO_IS_CONTACTS (contacts));
  priv = MOKO_CONTACTS (contacts)->priv;

  g_hash_table_destroy (priv->prefixes);
  g_hash_table_destroy (priv->uids);

  for (l = priv->contacts; l != NULL; l = l->next)
  {
    MokoContact *contact = (MokoContact*)l->data;
    if (contact)
    {
      g_free (contact->uid);
      g_free (contact->name);
      if (G_IS_OBJECT (contact->photo))
        g_object_unref (contact->photo);
    }
  }
  g_list_free (priv->contacts);
  
  for (l = priv->entries; l != NULL; l = l->next)
  {
    MokoContactEntry *entry = (MokoContactEntry*)l->data;
    if (entry)
    {
      g_free (entry->desc);
      g_free (entry->number);
      entry->contact = NULL;
    }
  }
  g_list_free (priv->entries);

  if (priv->start)
  {
    free_digit (priv->start);
  } 

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

  priv->contacts = NULL;
  priv->entries = NULL;
  priv->start = NULL;
  priv->prefixes = g_hash_table_new ((GHashFunc)g_str_hash,
                                     (GEqualFunc)g_str_equal);
  priv->uids = g_hash_table_new ((GHashFunc)g_str_hash,
                                     (GEqualFunc)g_str_equal);
  query = e_book_query_any_field_contains ("");

  /* Open the system book and check that it is valid */
  book = priv->book = e_book_new_system_addressbook (NULL);
  if (!book)
  {
    g_warning ("Failed to create system book\n");
    return;
  }
  
  if (!e_book_open (book, FALSE, NULL))
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

  /* Connect to the ebookviews signals */
  if (e_book_get_book_view (book, query, NULL, 0, &view, NULL))
  {
    g_signal_connect (G_OBJECT (view), "contacts-added",
                      G_CALLBACK (on_ebook_contacts_added), (gpointer)contacts);
    g_signal_connect (G_OBJECT (view), "contacts-changed",
                    G_CALLBACK (on_ebook_contacts_changed), (gpointer)contacts);
    g_signal_connect (G_OBJECT (view), "contacts-removed",
                    G_CALLBACK (on_ebook_contacts_removed), (gpointer)contacts);

    e_book_view_start (view);
  }
  e_book_query_unref(query);
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

gpointer
moko_contacts_get_backend (MokoContacts *contacts)
{
  MokoContactsPrivate *priv = MOKO_CONTACTS_GET_PRIVATE (contacts);

  return (gpointer) priv->book;
}
