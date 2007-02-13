/*  moko-finger-tool-box.h
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

#ifndef _MOKO_FINGER_TOOL_BOX_H_
#define _MOKO_FINGER_TOOL_BOX_H_

#include "moko-alignment.h"

#include <gtk/gtkbutton.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_FINGER_TOOL_BOX moko_finger_tool_box_get_type()
#define MOKO_FINGER_TOOL_BOX(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj),     MOKO_TYPE_FINGER_TOOL_BOX, MokoFingerToolBox))
#define MOKO_FINGER_TOOL_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),     MOKO_TYPE_FINGER_TOOL_BOX, MokoFingerToolBoxClass))
#define MOKO_IS_FINGER_TOOL_BOX(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj),     MOKO_TYPE_FINGER_TOOL_BOX))
#define MOKO_IS_FINGER_TOOL_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass),     MOKO_TYPE_FINGER_TOOL_BOX))
#define MOKO_FINGER_TOOL_BOX_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj),     MOKO_TYPE_FINGER_TOOL_BOX, MokoFingerToolBoxClass))

typedef struct {
    MokoAlignment parent;
} MokoFingerToolBox;

typedef struct {
    MokoAlignmentClass parent_class;
} MokoFingerToolBoxClass;

GType moko_finger_tool_box_get_type (void);

GtkWidget* moko_finger_tool_box_new ( GtkWidget* parent);

GtkWidget* moko_finger_tool_box_add_button(MokoFingerToolBox* self);
GtkWidget* moko_finger_tool_box_add_button_without_label(MokoFingerToolBox* self);

G_END_DECLS

#endif // _MOKO_FINGER_TOOL_BOX_H_

