/*  moko-panel-applet.h
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
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
 *  Current Version: $Rev$ ($Date$) [$Author: mickey $]
 */

#ifndef _MOKO_PANEL_APPLET_H_
#define _MOKO_PANEL_APPLET_H_

#include <libmb/mbtray.h>

#include <gtk/gtkwidget.h>
#include <gtk/gtkwindow.h>

#include <X11/X.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_PANEL_APPLET moko_panel_applet_get_type()
#define MOKO_PANEL_APPLET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_PANEL_APPLET, MokoPanelApplet))
#define MOKO_PANEL_APPLET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_PANEL_APPLET, MokoPanelAppletClass))
#define MOKO_IS_PANEL_APPLET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_PANEL_APPLET))
#define MOKO_IS_PANEL_APPLET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_PANEL_APPLET))
#define MOKO_PANEL_APPLET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_PANEL_APPLET, MokoPanelAppletClass))

typedef enum {
    MOKO_PANEL_APPLET_CLICK_POPUP,
    MOKO_PANEL_APPLET_TAP_HOLD_POPUP,
    LAST_POPUP_TYPE,
} MokoPanelAppletPopupType;

typedef struct {
    GObject parent;
    MBTrayApp* mb_applet;
    MBPixbuf* mb_pixbuf;
    MBPixbufImage* mb_pixbuf_image;
    MBPixbufImage* mb_pixbuf_image_scaled;
    int* argc;
    char*** argv;
    GtkWidget* popup[LAST_POPUP_TYPE];
    GtkWindow* window;
} MokoPanelApplet;

typedef struct {
    GObjectClass parent_class;

    /* these may be overridden in derived classes */
    void (*resize_callback) (MokoPanelApplet* self, int w, int h); // override to add custom resize handling
    void (*paint_callback) (MokoPanelApplet* self, Drawable drw); // override to add custom paint
    void (*clicked) (MokoPanelApplet* self); // override to add custom behaviour on click
    void (*tap_hold) (MokoPanelApplet* self); // override to add custom behaviour on tap-with-hold

    /* usually, there's no need to override these */
    void (*button_press_callback) (MokoPanelApplet* self, int x, int y);
    void (*button_release_callback) (MokoPanelApplet* self, int x, int y);

} MokoPanelAppletClass;

/* type interface */
GType moko_panel_applet_get_type();
MokoPanelApplet* moko_panel_applet_new();
void moko_panel_system_init( int* argc, char*** argv );

/* simple interface */
void moko_panel_applet_set_icon(MokoPanelApplet* self, const gchar* filename, gboolean scaling);
void moko_panel_applet_get_positioning_hint(MokoPanelApplet* self, GtkWidget* popup, int* x, int* y);
void moko_panel_applet_set_popup(MokoPanelApplet* self, GtkWidget* popup, MokoPanelAppletPopupType type);
void moko_panel_applet_open_popup(MokoPanelApplet* self, MokoPanelAppletPopupType type);
void moko_panel_applet_close_popup(MokoPanelApplet* self);
void moko_panel_applet_show(MokoPanelApplet* self);
void moko_panel_applet_show(MokoPanelApplet* self);

/* advanced interface */
void moko_panel_applet_request_size(MokoPanelApplet* self, int x, int y);
void moko_panel_applet_request_offset(MokoPanelApplet* self, int offset);

G_END_DECLS

#endif // _MOKO_PANEL_APPLET_H_

