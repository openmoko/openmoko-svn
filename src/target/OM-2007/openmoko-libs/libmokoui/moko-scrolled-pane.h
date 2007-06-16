/*  moko-scrolled-pane.h
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2007 OpenMoko, Inc.
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
 *  Current Version: $Rev$ ($Date: 2007/04/13 13:56:22 $) [$Author: mickey $]
 */

#ifndef _MOKO_SCROLLED_PANE_H_
#define _MOKO_SCROLLED_PANE_H_

#include <gtk/gtkhbox.h>
#include <gtk/gtkscrolledwindow.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_SCROLLED_PANE moko_scrolled_pane_get_type()
#define MOKO_SCROLLED_PANE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_SCROLLED_PANE, MokoScrolledPane))
#define MOKO_SCROLLED_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_SCROLLED_PANE, MokoScrolledPaneClass))
#define MOKO_IS_SCROLLED_PANE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_SCROLLED_PANE))
#define MOKO_IS_SCROLLED_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_SCROLLED_PANE))
#define MOKO_SCROLLED_PANE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_SCROLLED_PANE, MokoScrolledPaneClass))

typedef struct {
    GtkHBox parent;
} MokoScrolledPane;

typedef struct {
    GtkHBoxClass parent_class;
} MokoScrolledPaneClass;

GType moko_scrolled_pane_get_type();
GtkWidget* moko_scrolled_pane_new();

void moko_scrolled_pane_pack (MokoScrolledPane *pane, GtkWidget *child);
void moko_scrolled_pane_unpack (MokoScrolledPane *pane, GtkWidget *child);
GtkWidget *moko_scrolled_pane_get_child (MokoScrolledPane *pane);
void moko_scrolled_pane_pack_with_viewport (MokoScrolledPane *pane, GtkWidget *child);
void moko_scrolled_pane_set_fullscreen_position( MokoScrolledPane* self, gboolean top );

G_END_DECLS

#endif // _MOKO_SCROLLED_PANE_H_

