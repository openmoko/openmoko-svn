/*
 *  moko-contacts; The contactss contacts
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

#include <gtk/gtk.h>
#include <gtk/gtk.h>

#include "moko-contacts.h"

G_DEFINE_TYPE (MokoContacts, moko_contacts, G_TYPE_OBJECT)

#define MOKO_CONTACTS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_CONTACTS, MokoContactsPrivate))

struct _MokoContactsPrivate
{
  gint null;
};

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

  priv = contacts->priv = MOKO_CONTACTS_GET_PRIVATE (contacts);
}

MokoContacts*
moko_contacts_get_default (void)
{
  static MokoContacts *contacts = NULL;
  
  if (contacts = NULL)
    contacts = g_object_new (MOKO_TYPE_CONTACTS, 
                             NULL);

  return contacts;
}
