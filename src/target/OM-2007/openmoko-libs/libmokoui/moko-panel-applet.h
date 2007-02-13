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

#define DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#else
#define moko_debug(fmt,...)
#endif

#include <gtk/gtkalignment.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkwindow.h>

#include <gdk/gdkpixbuf.h>

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
    GtkAlignment parent;
    GtkImage* icon;
    GtkWidget* popup[LAST_POPUP_TYPE];
    GtkWindow* toplevelwindow;
    GtkEventBox* eventbox;
} MokoPanelApplet;

typedef struct {
    GtkAlignmentClass parent_class;

    void (*clicked) (MokoPanelApplet* self); // override to add custom behaviour on click
    void (*tap_hold) (MokoPanelApplet* self); // override to add custom behaviour on tap-with-hold

} MokoPanelAppletClass;

/* type interface */
GType moko_panel_applet_get_type();
GtkWidget* moko_panel_applet_new();
void moko_panel_system_init( int* argc, char*** argv );

/* simple interface */
void moko_panel_applet_set_icon(MokoPanelApplet* self, const gchar* filename, gboolean scaling);
void moko_panel_applet_set_pixbuf(MokoPanelApplet* self, GdkPixbuf* pixbuf);
void moko_panel_applet_set_widget(MokoPanelApplet* self, GtkWidget* widget);
void moko_panel_applet_get_positioning_hint(MokoPanelApplet* self, GtkWidget* popup, int* x, int* y);
void moko_panel_applet_set_popup(MokoPanelApplet* self, GtkWidget* popup, MokoPanelAppletPopupType type);
void moko_panel_applet_open_popup(MokoPanelApplet* self, MokoPanelAppletPopupType type);
void moko_panel_applet_close_popup(MokoPanelApplet* self);

G_END_DECLS

#endif // _MOKO_PANEL_APPLET_H_

