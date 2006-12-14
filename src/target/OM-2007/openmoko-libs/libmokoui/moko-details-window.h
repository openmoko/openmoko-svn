/*  moko-details-window.h
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
 *  Current Version: $Rev$ ($Date$) [$Author: mickey $]
 */

#ifndef _MOKO_DETAILS_WINDOW_H_
#define _MOKO_DETAILS_WINDOW_H_

#include <gtk/gtkbox.h>
#include <gtk/gtkscrolledwindow.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_DETAILS_WINDOW moko_details_window_get_type()
#define MOKO_DETAILS_WINDOW(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj),     MOKO_TYPE_DETAILS_WINDOW, MokoDetailsWindow))
#define MOKO_DETAILS_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),     MOKO_TYPE_DETAILS_WINDOW, MokoDetailsWindowClass))
#define MOKO_IS_DETAILS_WINDOW(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj),     MOKO_TYPE_DETAILS_WINDOW))
#define MOKO_IS_DETAILS_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass),     MOKO_TYPE_DETAILS_WINDOW))
#define MOKO_DETAILS_WINDOW_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj),     MOKO_TYPE_DETAILS_WINDOW, MokoDetailsWindowClass))

typedef struct {
    GtkScrolledWindow parent;
} MokoDetailsWindow;

typedef struct {
    GtkScrolledWindowClass parent_class;
} MokoDetailsWindowClass;

GType moko_details_window_get_type();
MokoDetailsWindow* moko_details_window_new();

GtkBox* moko_details_window_put_in_box(MokoDetailsWindow* self);

G_END_DECLS

#endif // _MOKO_DETAILS_WINDOW_H_

