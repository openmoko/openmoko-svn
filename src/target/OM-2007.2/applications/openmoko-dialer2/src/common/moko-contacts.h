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

#ifndef _HAVE_MOKO_CONTACTS_H
#define _HAVE_MOKO_CONTACTS_H

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MOKO_TYPE_CONTACTS (moko_contacts_get_type ())

#define MOKO_CONTACTS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        MOKO_TYPE_CONTACTS, MokoContacts))

#define MOKO_CONTACTS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        MOKO_TYPE_CONTACTS, MokoContactsClass))

#define MOKO_IS_CONTACTS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
        MOKO_TYPE_CONTACTS))

#define MOKO_IS_CONTACTS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
        MOKO_TYPE_CONTACTS))

#define MOKO_CONTACTS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
        MOKO_TYPE_CONTACTS, MokoContactsClass))

typedef struct _MokoContacts MokoContacts;
typedef struct _MokoContactsClass MokoContactsClass;
typedef struct _MokoContactsPrivate MokoContactsPrivate;
typedef struct _MokoContact MokoContact;
typedef struct _MokoContactEntry MokoContactEntry;

struct _MokoContacts
{
  GObject         parent;

  /*< private >*/
  MokoContactsPrivate   *priv;
};

struct _MokoContactsClass 
{
  /*< private >*/
  GObjectClass    parent_class;
  
 /* future padding */
  void (*_moko_contacts_1) (void);
  void (*_moko_contacts_2) (void);
  void (*_moko_contacts_3) (void);
  void (*_moko_contacts_4) (void);
};

struct _MokoContact
{
  gchar       *uid;
  gchar       *name;
  GdkPixbuf   *photo;
};

struct _MokoContactEntry
{
  gchar       *desc;
  gchar       *number;
  MokoContact *contact;
};

GType moko_contacts_get_type (void) G_GNUC_CONST;

MokoContacts*
moko_contacts_get_default (void);

gpointer
moko_contacts_get_backend (MokoContacts *contacts);

MokoContactEntry*
moko_contacts_lookup (MokoContacts *contacts, const gchar *number);

GList*
moko_contacts_fuzzy_lookup (MokoContacts *contacts, const gchar *number);

void
moko_contacts_get_photo (MokoContacts *contacts, MokoContact *m_contact);

G_END_DECLS

#endif /* _HAVE_MOKO_CONTACTS_H */
