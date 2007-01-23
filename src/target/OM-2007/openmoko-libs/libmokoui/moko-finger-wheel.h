/*  moko-finger-wheel.h
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
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
 *  Current Version: $Rev$ ($Date) [$Author: mickey $]
 */

#ifndef _MOKO_FINGER_WHEEL_H_
#define _MOKO_FINGER_WHEEL_H_

#include "moko-fixed.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_FINGER_WHEEL moko_finger_wheel_get_type()
#define MOKO_FINGER_WHEEL(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj),     MOKO_TYPE_FINGER_WHEEL, MokoFingerWheel))
#define MOKO_FINGER_WHEEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),     MOKO_TYPE_FINGER_WHEEL, MokoFingerWheelClass))
#define MOKO_IS_FINGER_WHEEL(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj),     MOKO_TYPE_FINGER_WHEEL))
#define MOKO_IS_FINGER_WHEEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass),     MOKO_TYPE_FINGER_WHEEL))
#define MOKO_FINGER_WHEEL_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj),     MOKO_TYPE_FINGER_WHEEL, MokoFingerWheelClass))

typedef struct {
    MokoFixed parent;
    gint area_id;
} MokoFingerWheel;

typedef struct {
    MokoFixedClass parent_class;

    void (* press_left_up) (GtkWidget *button);
    void (* press_right_down) (GtkWidget *button);
    void (* press_bottom) (GtkWidget *button);
    void (* long_press_left_up) (GtkWidget *button);
    void (* long_press_right_down) (GtkWidget *button);
} MokoFingerWheelClass;

GType moko_finger_wheel_get_type();
GtkWidget* moko_finger_wheel_new(GtkWidget * parent);
void moko_finger_wheel_raise(MokoFingerWheel* self);

G_END_DECLS

#endif // _MOKO_FINGER_WHEEL_H_

