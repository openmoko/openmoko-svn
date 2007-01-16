/*  moko-panel-mainmenu.c
 *
 *  Authored By Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
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
#include "moko-panel-mainmenu.h"

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <libmb/mbpixbuf.h>

#define DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoPanelMainmenu, moko_panel_mainmenu, MOKO_TYPE_PANEL_APPLET);

#define PANEL_APPLET_GET_PRIVATE(obj)   (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOKO_TYPE_PANEL_MAINMENU, MokoPanelMainmenuPrivate))

typedef struct _MokoPanelMainmenuPrivate
{
};

/* parent class pointer */
MokoPanelAppletClass* parent_class = NULL;

/* forward declarations */
void moko_panel_mainmenu_clicked(MokoPanelMainmenu* self);
void moko_panel_mainmenu_tap_hold(MokoPanelMainmenu* self);
void moko_panel_mainmenu_paint(MokoPanelMainmenu* self, Drawable drw);
void moko_panel_mainmenu_resize (MokoPanelMainmenu *self, Drawable drw);

static void
moko_panel_mainmenu_dispose(GObject* object)
{
    if (G_OBJECT_CLASS (moko_panel_mainmenu_parent_class)->dispose)
        G_OBJECT_CLASS (moko_panel_mainmenu_parent_class)->dispose (object);
}

static void
moko_panel_mainmenu_finalize(GObject* object)
{
    G_OBJECT_CLASS (moko_panel_mainmenu_parent_class)->finalize (object);
}

static void
moko_panel_mainmenu_class_init(MokoPanelMainmenuClass* klass)
{
    /* hook parent */
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* add private */
    g_type_class_add_private (klass, sizeof(MokoPanelMainmenuPrivate));

    /* hook destruction */
    object_class->dispose = moko_panel_mainmenu_dispose;
    object_class->finalize = moko_panel_mainmenu_finalize;

    /* virtual methods */
    MokoPanelAppletClass* applet_class = MOKO_PANEL_APPLET_CLASS(klass);
    applet_class->clicked = moko_panel_mainmenu_clicked;
    applet_class->tap_hold = moko_panel_mainmenu_tap_hold;
    applet_class->paint_callback = moko_panel_mainmenu_paint;
    applet_class->resize_callback = moko_panel_mainmenu_resize;

    /* install properties */
}

MokoPanelMainmenu*
moko_panel_mainmenu_new (void)
{
    return g_object_new(MOKO_TYPE_PANEL_MAINMENU, NULL);
}

static void
moko_panel_mainmenu_init(MokoPanelMainmenu* self)
{
    moko_debug ("moko panel mainmenu init");
    MokoPanelApplet *panel = MOKO_PANEL_APPLET(self);
    //g_free (panel->mb_applet);
    //mb_tray_app_main_init( panel->mb_applet );

}

void 
moko_panel_mainmenu_clicked(MokoPanelMainmenu* self)
{
    g_debug( "demo-panel-applet CLICKED" );
}

void 
moko_panel_mainmenu_tap_hold(MokoPanelMainmenu* self)
{
    g_debug( "demo-panel-applet TAP-HOLD" );
}

void 
moko_panel_mainmenu_paint(MokoPanelMainmenu* self, Drawable drw)
{
    g_debug ("moko panel mainmenu paint");
    MokoPanelApplet* panel = MOKO_PANEL_APPLET(self);
    MBPixbufImage* background = mb_tray_app_get_background( panel->mb_applet, panel->mb_pixbuf );
    mb_pixbuf_img_copy_composite (panel->mb_pixbuf, background, 
			       panel->mb_pixbuf_image, 
			       0, 0,
			       mb_tray_app_width (panel->mb_applet),
			       mb_tray_app_height (panel->mb_applet),
			       0, 0 );

    mb_pixbuf_img_render_to_drawable (panel->mb_pixbuf, background, drw, 0, 0);

    mb_pixbuf_img_free (panel->mb_pixbuf, background );

    g_debug( "demo-panel-applet PAINT" );
}

void
moko_panel_mainmenu_resize(MokoPanelMainmenu * self, Drawable drw)
{
    MokoPanelApplet *panel = MOKO_PANEL_APPLET(self);
    g_debug ("moko panel mainmenu resize");
    if (!panel->mb_pixbuf_image)
    	return;
    
    int  width  = mb_pixbuf_img_get_width (panel->mb_pixbuf_image);
    int  height = mb_pixbuf_img_get_height (panel->mb_pixbuf_image);

    moko_debug ("pixbuf: %d, %d", width, height);
    mb_tray_app_request_size (panel->mb_applet, width, height);

    width  = mb_tray_app_width (panel->mb_applet);
    height = mb_tray_app_height (panel->mb_applet);
    moko_debug ("tray app: %d, %d", width, height);
}

static void 
fork_exec(char *cmd)
{
  switch (fork())
    {
    case 0:
      setpgid(0, 0); /* Stop us killing child */
      mb_exec(cmd);
      fprintf(stderr, "openmoko-panel-gsm: Failed to Launch '%s'\n", cmd);
      exit(1);
    case -1:
      fprintf(stderr, "openmoko-panel-gsm: Failed to Launch '%s'", cmd);
      break;
    }
}

