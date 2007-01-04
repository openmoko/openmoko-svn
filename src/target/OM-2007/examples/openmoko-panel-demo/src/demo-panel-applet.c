/*  demo-panel-applet.c
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2007 Vanille-Media
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

#include "demo-panel-applet.h"

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (DemoPanelApplet, demo_panel_applet, MOKO_TYPE_PANEL_APPLET);

#define PANEL_APPLET_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), DEMO_TYPE_PANEL_APPLET, DemoPanelAppletPrivate))

typedef struct _DemoPanelAppletPrivate
{
} DemoPanelAppletPrivate;

/* parent class pointer */
MokoPanelAppletClass* parent_class = NULL;

/* forward declarations */
/* ... */

static void
demo_panel_applet_dispose(GObject* object)
{
    if (G_OBJECT_CLASS (demo_panel_applet_parent_class)->dispose)
        G_OBJECT_CLASS (demo_panel_applet_parent_class)->dispose (object);
}

static void
demo_panel_applet_finalize(GObject* object)
{
    G_OBJECT_CLASS (demo_panel_applet_parent_class)->finalize (object);
}

static void
demo_panel_applet_class_init(DemoPanelAppletClass* klass)
{
    /* hook parent */
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* add private */
    g_type_class_add_private (klass, sizeof(DemoPanelAppletPrivate));

    /* hook destruction */
    object_class->dispose = demo_panel_applet_dispose;
    object_class->finalize = demo_panel_applet_finalize;

    /* virtual methods */

    /* install properties */
}

DemoPanelApplet*
demo_panel_applet_new(void)
{
    return g_object_new(DEMO_TYPE_PANEL_APPLET, NULL);
}

static void
demo_panel_applet_init(DemoPanelApplet* self)
{
    /* Populate your instance here */
}
