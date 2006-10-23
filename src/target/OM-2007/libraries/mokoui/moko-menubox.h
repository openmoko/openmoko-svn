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
#ifndef _MOKO_MENU_BOX_H_
#define _MOKO_MENU_BOX_H_

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkmenu.h>

G_BEGIN_DECLS

#define MOKO_TYPE_MENU_BOX            (moko_menu_box_get_type())
#define MOKO_MENU_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_MENU_BOX, MokoMenuBox))
#define MOKO_MENU_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_MENU_BOX, MokoMenuBoxClass))
#define IS_MOKO_MENU_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_MENU_BOX))
#define IS_MOKO_MENU_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_MENU_BOX))

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
    void (*moko_menu_box) (MokoMenuBox *self);
};

GType          moko_menu_box_get_type    (void);
GtkWidget*     moko_menu_box_new         (void);
void           moko_menu_box_clear       (MokoMenuBox *self);

void           moko_menu_box_set_application_menu(MokoMenuBox* self, GtkMenu* menu);
void           moko_menu_box_set_filter_menu(MokoMenuBox* self, GtkMenu* menu);

G_END_DECLS

#endif /* _MOKO_MENU_BOX_H_ */
