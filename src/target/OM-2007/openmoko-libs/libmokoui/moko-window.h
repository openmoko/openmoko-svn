/*
 *  libmokoui -- OpenMoko Application Framework UI Library
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */
#ifndef _MOKO_WINDOW_H_
#define _MOKO_WINDOW_H_

#include <gtk/gtkwindow.h>

#include <glib-object.h>

#include <X11/X.h>

G_BEGIN_DECLS

#define MOKO_TYPE_WINDOW            (moko_window_get_type())
#define MOKO_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_WINDOW, MokoWindow))
#define MOKO_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_WINDOW, MokoWindowClass))
#define MOKO_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_WINDOW))
#define MOKO_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_WINDOW))

typedef struct _MokoWindow
{
    GtkWindow parent;
    /* add pointers to new members here */
} MokoWindow;

typedef struct _MokoWindowClass
{
    /* add your parent class here */
    GtkWindowClass parent_class;
    void (*moko_window) (MokoWindow *self);
} MokoWindowClass;

GType moko_window_get_type();
GtkWidget* moko_window_new();
void moko_window_clear(MokoWindow *self);
void moko_window_update_topmost (MokoWindow* self, Window window_id);

Window moko_window_get_active_window();

void moko_window_set_status_message (MokoWindow *self, gchar *message);
void moko_window_set_status_progress (MokoWindow *self, gdouble progress);

G_END_DECLS

#endif /* _MOKO_WINDOW_H_ */