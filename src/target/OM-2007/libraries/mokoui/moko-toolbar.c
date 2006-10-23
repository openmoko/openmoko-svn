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
#include "moko-toolbar.h"

/* add your signals here */
enum {
    MOKO_TOOL_BOX_SIGNAL,
    LAST_SIGNAL
};

static void moko_tool_box_class_init          (MokoToolBoxClass *klass);
static void moko_tool_box_init                (MokoToolBox      *f);

static guint moko_tool_box_signals[LAST_SIGNAL] = { 0 };

GType moko_tool_box_get_type (void) /* Typechecking */
{
    static GType self_type = 0;

    if (!self_type)
    {
        static const GTypeInfo f_info =
        {
            sizeof (MokoToolBoxClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moko_tool_box_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoToolBox),
            0,
            (GInstanceInitFunc) moko_tool_box_init,
        };

        /* add the type of your parent class here */
        self_type = g_type_register_static(GTK_TYPE_TOOLBAR, "MokoToolBox", &f_info, 0);
    }

    return self_type;
}

static void moko_tool_box_class_init (MokoToolBoxClass *klass) /* Class Initialization */
{
    moko_tool_box_signals[MOKO_TOOL_BOX_SIGNAL] = g_signal_new ("moko_tool_box",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoToolBoxClass, moko_tool_box),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

static void moko_tool_box_init (MokoToolBox *f) /* Instance Construction */
{
    /* populate your widget here */
}

GtkWidget* moko_tool_box_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(moko_tool_box_get_type(), NULL));
}

void moko_tool_box_clear(MokoToolBox *f) /* Destruction */
{
    /* destruct your widgets here */
}

/* add new methods here */
