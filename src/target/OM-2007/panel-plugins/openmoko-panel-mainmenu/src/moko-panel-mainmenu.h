/*  moko-panel-mainmenu.h
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

#ifndef _MOKO_PANEL_MAINMENU_H_
#define _MOKO_PANEL_MAINMENU_H_

#include <libmokoui/moko-panel-applet.h>

#include <glib-object.h>

G_BEGIN_DECLS


#define MOKO_TYPE_PANEL_MAINMENU 			moko_panel_mainmenu_get_type()
#define MOKO_PANEL_MAINMENU(obj) 				(G_TYPE_CHECK_INSTANCE_CAST ((obj), \
									MOKO_TYPE_PANEL_MAINMENU, MokoPanelMainmenu))
#define MOKO_PANEL_MAINMENU_CLASS(klass) 		(G_TYPE_CHECK_CLASS_CAST ((klass), \
									MOKO_TYPE_PANEL_MAINMENU, MokoPanelMainmenuClass))
#define MOKO_IS_PANEL_MAINMENU(obj) 			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
									MOKO_TYPE_PANEL_MAINMENU))
#define MOKO_IS_PANEL_MAINMENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
									MOKO_TYPE_PANEL_MAINMENU))
#define MOKO_PANEL_MAINMENU_GET_CLASS(obj) 	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
									MOKO_TYPE_PANEL_MAINMENU, MokoPanelMainmenuClass))
					

typedef struct _MokoPanelMainmenu MokoPanelMainmenu;
typedef struct _MokoPanelMainmenuClass MokoPanelMainmenuClass;
typedef struct _MokoPanelMainmenuPrivate  MokoPanelMainmenuPrivate;

struct _MokoPanelMainmenu {
    MokoPanelApplet parent;

    MokoPanelMainmenuPrivate *priv;
};

struct _MokoPanelMainmenuClass {
    MokoPanelAppletClass parent_class;
};

GType moko_panel_mainmenu_get_type ();

MokoPanelMainmenu* moko_panel_mainmenu_new ();

G_END_DECLS

#endif /*_MOKO_PANEL_MAINMENU_H_*/

