/*  demo-panel-applet.h
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
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
 *  Current Version: $Rev$ ($Date: 2006/12/21 18:03:04 $) [$Author: mickey $]
 */

#ifndef _DEMO_PANEL_APPLET_H_
#define _DEMO_PANEL_APPLET_H_

#include <libmokoui/moko-panel-applet.h>

#include <glib-object.h>

G_BEGIN_DECLS

#define DEMO_TYPE_PANEL_APPLET demo_panel_applet_get_type()
#define DEMO_PANEL_APPLET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), DEMO_TYPE_PANEL_APPLET, DemoPanelApplet))
#define DEMO_PANEL_APPLET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), DEMO_TYPE_PANEL_APPLET, DemoPanelAppletClass))
#define DEMO_IS_PANEL_APPLET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DEMO_TYPE_PANEL_APPLET))
#define DEMO_IS_PANEL_APPLET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DEMO_TYPE_PANEL_APPLET))
#define DEMO_PANEL_APPLET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), DEMO_TYPE_PANEL_APPLET, DemoPanelAppletClass))

typedef struct {
    MokoPanelApplet parent;
} DemoPanelApplet;

typedef struct {
    MokoPanelAppletClass parent_class;
} DemoPanelAppletClass;

GType demo_panel_applet_get_type();
DemoPanelApplet* demo_panel_applet_new();

G_END_DECLS

#endif // _DEMO_PANEL_APPLET_H_

