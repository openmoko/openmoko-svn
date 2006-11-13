/*  moko-finger-wheel.c
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

#include "moko-finger-wheel.h"

G_DEFINE_TYPE (MokoFingerWheel, moko_finger_wheel, MOKO_TYPE_FIXED)

#define FINGER_WHEEL_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_WHEEL, MokoFingerWheelPrivate))

typedef struct _MokoFingerWheelPrivate
{
} MokoFingerWheelPrivate;

/* forward declarations */
/* ... */

static void
moko_finger_wheel_dispose (GObject *object)
{
    if (G_OBJECT_CLASS (moko_finger_wheel_parent_class)->dispose)
        G_OBJECT_CLASS (moko_finger_wheel_parent_class)->dispose (object);
}

static void
moko_finger_wheel_finalize (GObject *object)
{
    G_OBJECT_CLASS (moko_finger_wheel_parent_class)->finalize (object);
}

static void
moko_finger_wheel_class_init (MokoFingerWheelClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (MokoFingerWheelPrivate));

    /* hook virtual methods */
    /* ... */

    /* install properties */
    /* ... */

    object_class->dispose = moko_finger_wheel_dispose;
    object_class->finalize = moko_finger_wheel_finalize;
}

static void
moko_finger_wheel_init (MokoFingerWheel *self)
{
}

MokoFingerWheel*
moko_finger_wheel_new (void)
{
    return g_object_new (MOKO_TYPE_FINGER_WHEEL, NULL);
}
