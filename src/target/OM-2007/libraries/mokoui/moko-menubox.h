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
#ifndef _MOKO_MENUBOX_H_
#define _MOKO_MENUBOX_H_

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkmenu.h>

G_BEGIN_DECLS

#define MOKO_TYPE_MENUBOX            (moko_menubox_get_type())
#define MOKO_MENUBOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_MENUBOX, MokoMenuBox))
#define MOKO_MENUBOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_MENUBOX, MokoMenuBoxClass))
#define IS_MOKO_MENUBOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_MENUBOX))
#define IS_MOKO_MENUBOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_MENUBOX))

typedef struct _MokoMenuBox       MokoMenuBox;
typedef struct _MokoMenuBoxClass  MokoMenuBoxClass;

struct _MokoMenuBox
{
    GtkHBox parent;
    /* add pointers to new members here */
};

struct _MokoMenuBoxClass
{
    /* add your parent class here */
    GtkHBoxClass parent_class;
    void (*moko_menubox) (MokoMenuBox *self);
};

GType          moko_menubox_get_type    (void);
GtkWidget*     moko_menubox_new         (void);
void           moko_menubox_clear       (MokoMenuBox *self);

void           moko_menubox_set_application_menu(MokoMenuBox* self, GtkMenu* menu);
void           moko_menubox_set_filter_menu(MokoMenuBox* self, GtkMenu* menu);

G_END_DECLS

#endif /* _MOKO_MENUBOX_H_ */
