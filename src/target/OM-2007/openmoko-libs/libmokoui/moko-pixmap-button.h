/*  moko-pixmap-button.h
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
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
 *  Current Version: $Rev$ ($Date) [$Author: mickey $]
 */

#ifndef _MOKO_PIXMAP_BUTTON_H_
#define _MOKO_PIXMAP_BUTTON_H_

#include <gtk/gtkbutton.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkalignment.h>
#include <gtk/gtkmain.h>

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

void moko_pixmap_button_set_action_btn_upper_stock (MokoPixmapButton* self, const gchar *stock_name);
void moko_pixmap_button_set_action_btn_lower_label (MokoPixmapButton* self, const gchar *label);
void moko_pixmap_button_set_center_stock (MokoPixmapButton* self, const gchar *stock_name);
void moko_pixmap_button_set_center_image (MokoPixmapButton* self, GtkWidget* image);
void moko_pixmap_button_set_finger_toolbox_btn_center_image (MokoPixmapButton* self, GtkWidget* image);
void moko_pixmap_button_set_finger_toolbox_btn_center_image_pixbuf (MokoPixmapButton* self, GdkPixbuf* pixbuf);

G_END_DECLS

#endif // _MOKO_PIXMAP_BUTTON_H_

