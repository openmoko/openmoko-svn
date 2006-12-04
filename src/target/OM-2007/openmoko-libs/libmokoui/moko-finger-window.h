/*  moko-finger-window.h
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
 *  Current Version: $Rev$ ($Date) [$Author: mickey $]
 */

#ifndef _MOKO_FINGER_WINDOW_H_
#define _MOKO_FINGER_WINDOW_H_

#include "moko-window.h"

#include "moko-finger-tool-box.h"
#include "moko-finger-wheel.h"

#include <gtk/gtkmenu.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_FINGER_WINDOW moko_finger_window_get_type()
#define MOKO_FINGER_WINDOW(obj)   (G_TYPE_CHECK_INSTANCE_CAST ((obj),   MOKO_TYPE_FINGER_WINDOW, MokoFingerWindow))
#define MOKO_FINGER_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass),   MOKO_TYPE_FINGER_WINDOW, MokoFingerWindowClass))
#define MOKO_IS_FINGER_WINDOW(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj),   MOKO_TYPE_FINGER_WINDOW))
#define MOKO_IS_FINGER_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass),   MOKO_TYPE_FINGER_WINDOW))
#define MOKO_FINGER_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),   MOKO_TYPE_FINGER_WINDOW, MokoFingerWindowClass))

typedef struct {
    MokoWindow parent;
} MokoFingerWindow;

typedef struct {
    MokoWindowClass parent_class;
} MokoFingerWindowClass;

GType moko_finger_window_get_type (void);
GtkWidget* moko_finger_window_new (void);
void moko_finger_window_set_application_menu(MokoFingerWindow* self, GtkMenu* menu);
void moko_finger_window_set_contents(MokoFingerWindow* self, GtkWidget* child);
MokoFingerWheel* moko_finger_window_get_wheel(MokoFingerWindow* self);
MokoFingerToolBox* moko_finger_window_get_toolbox(MokoFingerWindow* self);
gboolean moko_finger_window_get_geometry_hint(MokoFingerWindow* self, GtkWidget* hintee, GtkAllocation* allocation);

G_END_DECLS

#endif // _MOKO_FINGER_WINDOW_H_

