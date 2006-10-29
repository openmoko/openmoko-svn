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
#ifndef _MOKO_PANED_WINDOW_H_
#define _MOKO_PANED_WINDOW_H_

#include "moko-window.h"
#include "moko-menubox.h"
#include "moko-toolbox.h"

#include <gtk/gtkmenu.h>

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_PANED_WINDOW            (moko_paned_window_get_type())
#define MOKO_PANED_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_PANED_WINDOW, MokoPanedWindow))
#define MOKO_PANED_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_PANED_WINDOW, MokoPanedWindowClass))
#define MOKO_IS_PANED_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_PANED_WINDOW))
#define MOKO_IS_PANED_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_PANED_WINDOW))

typedef struct _MokoPanedWindow       MokoPanedWindow;
typedef struct _MokoPanedWindowClass  MokoPanedWindowClass;

struct _MokoPanedWindow
{
    MokoWindow parent;
    /* add pointers to new members here */
};

struct _MokoPanedWindowClass
{
    /* add your parent class here */
    MokoWindowClass parent_class;
    void (*moko_paned_window) (MokoPanedWindow *self);
};

GType          moko_paned_window_get_type        (void);
GtkWidget*     moko_paned_window_new             (void);
void           moko_paned_window_clear           (MokoPanedWindow *self);

/* menu */
MokoMenuBox* moko_paned_window_get_menubox(MokoPanedWindow* self);
void moko_paned_window_set_application_menu(MokoPanedWindow* self, GtkMenu* menu);
void moko_paned_window_set_filter_menu(MokoPanedWindow* self, GtkMenu* menu);

/* toolbox */
void moko_paned_window_add_toolbox(MokoPanedWindow* self, MokoToolBox* toolbox);

/* panes */
void moko_paned_window_set_upper_pane(MokoPanedWindow* self, GtkWidget* child);
void moko_paned_window_set_lower_pane(MokoPanedWindow* self, GtkWidget* child);

G_END_DECLS

#endif /* _MOKO_PANED_WINDOW_H_ */
