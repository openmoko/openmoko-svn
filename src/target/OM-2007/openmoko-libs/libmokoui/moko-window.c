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

#include "moko-application.h"
#include "moko-window.h"

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

/* add your signals here */
enum {
    MOKO_WINDOW_SIGNAL,
    LAST_SIGNAL
};

static void moko_window_class_init          (MokoWindowClass *klass);
static void moko_window_init                (MokoWindow      *self);

static guint moko_window_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (MokoWindow, moko_window, GTK_TYPE_WINDOW)

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
    moko_debug( "moko_window_init" );
    gtk_widget_set_size_request( GTK_WIDGET(self), 480, 640 ); //FIXME get from style
    MokoApplication* app = moko_application_get_instance();
    if ( !moko_application_get_main_window( app ) )
        moko_application_set_main_window( app, self );
    else
        g_warning( "moko_window_init: there is already a main window" );
}

GtkWidget* moko_window_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(moko_window_get_type(), NULL));
}

void moko_window_clear(MokoWindow *self) /* Destruction */
{
    /* destruct your widgets here */
}
