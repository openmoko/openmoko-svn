/*
 *  libmokoui -- OpenMoko Application Framework UI Library
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */
#ifndef _MOKO_TOOL_BOX_H_
#define _MOKO_TOOL_BOX_H_

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtktoolbar.h>

G_BEGIN_DECLS

#define MOKO_TYPE_TOOL_BOX            (moko_tool_box_get_type())
#define MOKO_TOOL_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_TOOL_BOX, MokoToolBox))
#define MOKO_TOOL_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_TOOL_BOX, MokoToolBoxClass))
#define MOKO_IS_TOOL_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_TOOL_BOX))
#define MOKO_IS_TOOL_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_TOOL_BOX))

typedef struct _MokoToolBox       MokoToolBox;
typedef struct _MokoToolBoxClass  MokoToolBoxClass;

struct _MokoToolBox
{
    GtkVBox parent;
    /* add pointers to new members here */
};

struct _MokoToolBoxClass
{
    /* add your parent class here */
    GtkVBoxClass parent_class;
    void (*moko_tool_box) (MokoToolBox *self);
};

GType          moko_tool_box_get_type    (void);
GtkWidget*     moko_tool_box_new         (void);
void           moko_tool_box_clear       (MokoToolBox* self);

/* add additional methods here */
void moko_tool_box_add_search_button(MokoToolBox* self );
GtkToolbar*    moko_tool_box_get_tool_bar(MokoToolBox* self);

G_END_DECLS

#endif /* _MOKO_TOOL_BOX_H_ */
