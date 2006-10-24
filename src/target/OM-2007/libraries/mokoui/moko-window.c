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
#include "moko-window.h"

/* add your signals here */
enum {
    MOKO_WINDOW_SIGNAL,
    LAST_SIGNAL
};

static void moko_window_class_init          (MokoWindowClass *klass);
static void moko_window_init                (MokoWindow      *self);

static guint moko_window_signals[LAST_SIGNAL] = { 0 };

GType moko_window_get_type (void) /* Typechecking */
{
    static GType self_type = 0;

    if (!self_type)
    {
        static const GTypeInfo f_info =
        {
            sizeof (MokoWindowClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moko_window_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoWindow),
            0,
            (GInstanceInitFunc) moko_window_init,
        };

        /* add the type of your parent class here */
        self_type = g_type_register_static(GTK_TYPE_WINDOW, "MokoWindow", &f_info, 0);
    }

    return self_type;
}

static void moko_window_class_init (MokoWindowClass *klass) /* Class Initialization */
{
    moko_window_signals[MOKO_WINDOW_SIGNAL] = g_signal_new ("moko_window",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoWindowClass, moko_window),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

static void moko_window_init (MokoWindow *self) /* Instance Construction */
{
    g_debug( "moko_window_init" );
    gtk_widget_set_size_request( GTK_WIDGET(self), 480, 640 ); //FIXME get from style
}

GtkWidget* moko_window_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(moko_window_get_type(), NULL));
}

void moko_window_clear(MokoWindow *self) /* Destruction */
{
    /* destruct your widgets here */
}

/* add new methods here */
