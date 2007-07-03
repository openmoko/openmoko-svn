/*
 *  moko-keypad; The keypads keypad
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

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include <libmokogsmd/moko-gsmd-connection.h>
#include <libmokojournal/moko-journal.h>

#include "moko-keypad.h"

G_DEFINE_TYPE (MokoKeypad, moko_keypad, GTK_TYPE_VBOX)

#define MOKO_KEYPAD_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_KEYPAD, MokoKeypadPrivate))

struct _MokoKeypadPrivate
{
  gint                status;
 
};

enum
{
  DIAL_NUMBER,

  LAST_SIGNAL
};

static guint keypad_signals[LAST_SIGNAL] = {0, };

//* GObject functions */
static void
moko_keypad_dispose (GObject *object)
{
  MokoKeypad *keypad;
  MokoKeypadPrivate *priv;

  keypad = MOKO_KEYPAD (object);
  priv = keypad->priv;

  /* Close journal */
  //moko_journal_close (priv->data->journal);

  /* Free contacts list */
  //contact_release_contact_list (&(priv->data->g_contactlist));

  G_OBJECT_CLASS (moko_keypad_parent_class)->dispose (object);
}

static void
moko_keypad_finalize (GObject *keypad)
{
  G_OBJECT_CLASS (moko_keypad_parent_class)->finalize (keypad);
}


static void
moko_keypad_class_init (MokoKeypadClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = moko_keypad_finalize;
  obj_class->dispose = moko_keypad_dispose;

  g_type_class_add_private (obj_class, sizeof (MokoKeypadPrivate)); 
}

static void
moko_keypad_init (MokoKeypad *keypad)
{
  MokoKeypadPrivate *priv;
  MokoGsmdConnection *conn;

  priv = keypad->priv = MOKO_KEYPAD_GET_PRIVATE (keypad);

}

GtkWidget*
moko_keypad_new (void)
{
  MokoKeypad *keypad = NULL;
  
  keypad = g_object_new (MOKO_TYPE_KEYPAD, 
                         NULL);

  return GTK_WIDGET (keypad);
}
