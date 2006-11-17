/*  moko-pixmap-button.h
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 Vanille-Media
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: mickey $]
 */

#ifndef _MOKO_PIXMAP_BUTTON_H_
#define _MOKO_PIXMAP_BUTTON_H_

#include <gtk/gtkbutton.h>
#include <gtk/gtkmenu.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_PIXMAP_BUTTON moko_pixmap_button_get_type()
#define MOKO_PIXMAP_BUTTON(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj),     MOKO_TYPE_PIXMAP_BUTTON, MokoPixmapButton))
#define MOKO_PIXMAP_BUTTON_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),     MOKO_TYPE_PIXMAP_BUTTON, MokoPixmapButtonClass))
#define MOKO_IS_PIXMAP_BUTTON(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj),     MOKO_TYPE_PIXMAP_BUTTON))
#define MOKO_IS_PIXMAP_BUTTON_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass),     MOKO_TYPE_PIXMAP_BUTTON))
#define MOKO_PIXMAP_BUTTON_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj),     MOKO_TYPE_PIXMAP_BUTTON, MokoPixmapButtonClass))

typedef struct {
    GtkButton parent;
} MokoPixmapButton;

typedef struct {
    GtkButtonClass parent_class;
} MokoPixmapButtonClass;

GType moko_pixmap_button_get_type(void);

GtkWidget* moko_pixmap_button_new(void);
void moko_pixmap_button_set_menu(MokoPixmapButton* self, GtkMenu* menu);

G_END_DECLS

#endif // _MOKO_PIXMAP_BUTTON_H_

