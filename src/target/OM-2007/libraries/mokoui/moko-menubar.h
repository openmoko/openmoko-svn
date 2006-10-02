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
#ifndef _MOKO_MENUBAR_H_
#define _MOKO_MENUBAR_H_

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkmenubar.h>

G_BEGIN_DECLS

#define MOKO_TYPE_MENUBAR            (moko_menubar_get_type())
#define MOKO_MENUBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_MENUBAR, MokoMenuBar))
#define MOKO_MENUBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_MENUBAR, MokoMenuBarClass))
#define IS_MOKO_MENUBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_MENUBAR))
#define IS_MOKO_MENUBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_MENUBAR))

typedef struct _MokoMenuBar       MokoMenuBar;
typedef struct _MokoMenuBarClass  MokoMenuBarClass;

struct _MokoMenuBar
{
    GtkMenuBar parent;
    /* add pointers to new members here */
};

struct _MokoMenuBarClass
{
    /* add your parent class here */
    GtkMenuBarClass parent_class;
    void (*moko_menubar) (MokoMenuBar *self);
};

GType          moko_menubar_get_type    (void);
GtkWidget*     moko_menubar_new         (void);
void           moko_menubar_clera       (MokoMenuBar *self);

/* add additional methods here */

G_END_DECLS

#endif /* _MOKO_MENUBAR_H_ */
