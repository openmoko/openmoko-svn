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
#ifndef _MOKO_WINDOW_H_
#define _MOKO_WINDOW_H_

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkwindow.h>

G_BEGIN_DECLS

#define MOKO_WINDOW_TYPE            (moko_window_get_type())
#define MOKO_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_WINDOW_TYPE, MokoWindow))
#define MOKO_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_WINDOW_TYPE, MokoWindowClass))
#define IS_MOKO_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_WINDOW_TYPE))
#define IS_MOKO_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_WINDOW_TYPE))

typedef struct _MokoWindow       MokoWindow;
typedef struct _MokoWindowClass  MokoWindowClass;

struct _MokoWindow
{
    GtkWindow parent;
    /* add pointers to new members here */

};

struct _MokoWindowClass
{
    /* add your parent class here */
    GtkWindowClass parent_class;
    void (*moko_window) (MokoWindow *self);
};

GType          moko_window_get_type        (void);
GtkWidget*     moko_window_new             (void);
void           moko_window_clear           (MokoWindow *self);

/* add additional methods here */

G_END_DECLS

#endif /* _MOKO_WINDOW_H_ */
