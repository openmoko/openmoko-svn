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
    MOKO_TOOLBAR_SIGNAL,
    LAST_SIGNAL
};

static void moko_tool_bar_class_init          (MokoToolBarClass *klass);
static void moko_tool_bar_init                (MokoToolBar      *f);

static guint moko_tool_bar_signals[LAST_SIGNAL] = { 0 };

GType moko_tool_bar_get_type (void) /* Typechecking */
{
    static GType self_type = 0;

    if (!self_type)
    {
        static const GTypeInfo f_info =
        {
            sizeof (MokoToolBarClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moko_tool_bar_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoToolBar),
            0,
            (GInstanceInitFunc) moko_tool_bar_init,
        };

        /* add the type of your parent class here */
        self_type = g_type_register_static(GTK_TYPE_TOOLBAR, "MokoToolBar", &f_info, 0);
    }

    return self_type;
}

static void moko_tool_bar_class_init (MokoToolBarClass *klass) /* Class Initialization */
{
    moko_tool_bar_signals[MOKO_TOOLBAR_SIGNAL] = g_signal_new ("moko_tool_bar",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoToolBarClass, moko_tool_bar),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

static void moko_tool_bar_init (MokoToolBar *f) /* Instance Construction */
{
    /* populate your widget here */
}

GtkWidget* moko_tool_bar_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(moko_tool_bar_get_type(), NULL));
}

void moko_tool_bar_clear(MokoToolBar *f) /* Destruction */
{
    /* destruct your widgets here */
}

/* add new methods here */
