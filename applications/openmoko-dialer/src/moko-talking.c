/*
 *  moko-talking; a GObject wrapper for the talking/incoming/outgoing page.
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

#include <libmokogsmd/moko-gsmd-connection.h>
#include <libmokojournal/moko-journal.h>

#include "moko-talking.h"

G_DEFINE_TYPE (MokoTalking, moko_talking, GTK_TYPE_HBOX)

#define MOKO_TALKING_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE(obj, \
        MOKO_TYPE_TALKING, MokoTalkingPrivate))

struct _MokoTalkingPrivate
{
  gint                status;
   
};
/* GObject functions */
static void
moko_talking_dispose (GObject *object)
{
  G_OBJECT_CLASS (moko_talking_parent_class)->dispose (object);
}

static void
moko_talking_finalize (GObject *talking)
{
  G_OBJECT_CLASS (moko_talking_parent_class)->finalize (talking);
}

static void
moko_talking_class_init (MokoTalkingClass *klass)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (klass);

  obj_class->finalize = moko_talking_finalize;
  obj_class->dispose = moko_talking_dispose;

 g_type_class_add_private (obj_class, sizeof (MokoTalkingPrivate)); 
}

static void
moko_talking_init (MokoTalking *talking)
{
  MokoTalkingPrivate *priv;
  MokoGsmdConnection *conn;

  priv = talking->priv = MOKO_TALKING_GET_PRIVATE (talking);
}

GtkWidget*
moko_talking_new (void)
{
  MokoTalking *talking = NULL;
    
  talking = g_object_new (MOKO_TYPE_TALKING, NULL);

  return talking;
}
