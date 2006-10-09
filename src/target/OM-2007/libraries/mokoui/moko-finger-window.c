
/*  moko_finger_window.c
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 Vanille-Media
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: mickey $]
 */

#include "moko-finger-window.h"

G_DEFINE_TYPE (MokoFingerWindow, moko_finger_window, MOKO_TYPE_WINDOW);

#define FINGER_WINDOW_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_WINDOW, MokoFingerWindowPrivate))

typedef struct _MokoFingerWindowPrivate MokoFingerWindowPrivate;

struct _MokoFingerWindowPrivate
{
};

static void
moko_finger_window_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (moko_finger_window_parent_class)->dispose)
    G_OBJECT_CLASS (moko_finger_window_parent_class)->dispose (object);
}

static void
moko_finger_window_finalize (GObject *object)
{
  G_OBJECT_CLASS (moko_finger_window_parent_class)->finalize (object);
}

static void
moko_finger_window_class_init (MokoFingerWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MokoFingerWindowPrivate));

  object_class->dispose = moko_finger_window_dispose;
  object_class->finalize = moko_finger_window_finalize;
}

static void
moko_finger_window_init (MokoFingerWindow *self)
{
}

MokoFingerWindow*
moko_finger_window_new (void)
{
  return g_object_new (MOKO_TYPE_FINGER_WINDOW, NULL);
}
