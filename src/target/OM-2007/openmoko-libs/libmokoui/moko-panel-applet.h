/*  moko-panel-applet.h
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
 *  Current Version: $Rev$ ($Date: 2006/12/07 17:20:16 $) [$Author: mickey $]
 */

#ifndef _MOKO_PANEL_APPLET_H_
#define _MOKO_PANEL_APPLET_H_

#include <libmb/mbtray.h>

#include <X11/X.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_PANEL_APPLET moko_panel_applet_get_type()
#define MOKO_PANEL_APPLET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_PANEL_APPLET, MokoPanelApplet))
#define MOKO_PANEL_APPLET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_PANEL_APPLET, MokoPanelAppletClass))
#define MOKO_IS_PANEL_APPLET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_PANEL_APPLET))
#define MOKO_IS_PANEL_APPLET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_PANEL_APPLET))
#define MOKO_PANEL_APPLET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_PANEL_APPLET, MokoPanelAppletClass))

typedef struct {
    GObject parent;
    MBTrayApp* mb_applet;
    MBPixbuf* mb_pixbuf;
    MBPixbufImage* mb_pixbuf_image;
    int* argc;
    char*** argv;
} MokoPanelApplet;

typedef struct {
    GObjectClass parent_class;

    void (*resize_callback) (MokoPanelApplet* self, int w, int h);
    void (*paint_callback) (MokoPanelApplet* self, Drawable drw);
} MokoPanelAppletClass;

GType moko_panel_applet_get_type();
MokoPanelApplet* moko_panel_applet_new();

G_END_DECLS

#endif // _MOKO_PANEL_APPLET_H_

