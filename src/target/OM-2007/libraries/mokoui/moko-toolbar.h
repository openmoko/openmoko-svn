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
#ifndef _MOKO_TOOLBAR_H_
#define _MOKO_TOOLBAR_H_

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktoolbar.h>

G_BEGIN_DECLS

#define MOKO_TYPE_TOOLBAR            (moko_toolbar_get_type())
#define MOKO_TOOLBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_TOOLBAR, MokoToolBar))
#define MOKO_TOOLBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_TOOLBAR, MokoToolBarClass))
#define IS_MOKO_TOOLBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_TOOLBAR))
#define IS_MOKO_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_TOOLBAR))

typedef struct _MokoToolBar       MokoToolBar;
typedef struct _MokoToolBarClass  MokoToolBarClass;

struct _MokoToolBar
{
    GtkToolbar parent;
    /* add pointers to new members here */
};

struct _MokoToolBarClass
{
    /* add your parent class here */
    GtkToolbarClass parent_class;
    void (*moko_toolbar) (MokoToolBar *self);
};

GType          moko_toolbar_get_type    (void);
GtkWidget*     moko_toolbar_new         (void);
void           moko_toolbar_clera       (MokoToolBar *self);

/* add additional methods here */

G_END_DECLS

#endif /* _MOKO_TOOLBAR_H_ */
