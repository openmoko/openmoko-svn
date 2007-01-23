/*  fretboard-widget.h
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
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

#ifndef _CHORD_FRETBOARD_H_
#define _CHORD_FRETBOARD_H_

#include <gtk/gtkdrawingarea.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define CHORD_TYPE_FRETBOARD fretboard_widget_get_type()
#define CHORD_FRETBOARD(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj),     CHORD_TYPE_FRETBOARD, FretboardWidget))
#define CHORD_FRETBOARD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),     CHORD_TYPE_FRETBOARD, FretboardWidgetClass))
#define CHORD_IS_FRETBOARD(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj),     CHORD_TYPE_FRETBOARD))
#define CHORD_IS_FRETBOARD_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass),     CHORD_TYPE_FRETBOARD))
#define CHORD_FRETBOARD_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj),     CHORD_TYPE_FRETBOARD, FretboardWidgetClass))

typedef struct {
    GtkDrawingArea parent;
    GdkPixbuf* texture;
    GdkPixbuf* fingerpoint;
    gchar* frets;
} FretboardWidget;

typedef struct {
    GtkDrawingAreaClass parent_class;
} FretboardWidgetClass;

GType fretboard_widget_get_type (void);

FretboardWidget* fretboard_widget_new (void);

void fretboard_widget_set_frets(FretboardWidget* self, const gchar* frets);

G_END_DECLS

#endif // _CHORD_FRETBOARD_H_

