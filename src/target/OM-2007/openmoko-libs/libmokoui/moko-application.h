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

#ifndef _MOKO_APPLICATION_H_
#define _MOKO_APPLICATION_H_

#include "moko-window.h"
#include "moko-dialog-window.h"

#include <gtk/gtkmenu.h>
#include <gtk/gtktoolbar.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_APPLICATION (moko_application_get_type())
#define MOKO_APPLICATION(obj) (G_TYPE_CHECK_INSTANCE_CAST (obj, MOKO_TYPE_APPLICATION, MokoApplication))
#define MOKO_IS_APPLICATION(obj) (G_TYPE_CHECK_INSTANCE_TYPE (obj, MOKO_TYPE_APPLICATION))

typedef struct _MokoApplication
{
    GObject parent;
} MokoApplication;

typedef struct _MokoApplicationClass
{
    GObjectClass parent;
} MokoApplicationClass;

GType moko_application_get_type();

/* Public methods */
MokoApplication* moko_application_get_instance();
gboolean moko_application_get_is_topmost(MokoApplication* self);
void moko_application_set_main_window(MokoApplication* self, MokoWindow* window);
GtkWidget* moko_application_get_main_window(MokoApplication* self);
void moko_application_add_window (MokoApplication *self, MokoWindow *window);

gchar* moko_application_get_style_pixmap_dir();
void moko_application_add_stock_icons(MokoApplication* self, ...);

GtkWidget* moko_application_execute_dialog(MokoApplication* self, const gchar* title, GtkWidget* contents);

G_END_DECLS
#endif /* _MOKO_APPLICATION_H_ */
