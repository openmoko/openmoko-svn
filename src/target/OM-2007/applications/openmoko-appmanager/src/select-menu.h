/**
 *  @file select-menu.h
 *  @brief The select menu that popuo in the treeview
 *
 *  Copyright (C) 2006 First International Computer Inc.
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 *  @author Chaowei Song (songcw@fic-sh.com.cn)
 */
#ifndef _FIC_SELECT_MENU_H
#define _FIC_SELECT_MENU_H

#include <gtk/gtk.h>

#include "appmanager-data.h"

G_BEGIN_DECLS

#define MOKO_TYPE_SELECT_MENU (moko_select_menu_get_type ())
#define MOKO_SELECT_MENU(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_SELECT_MENU, MokoSelectMenu))
#define MOKO_SELECT_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_SELECT_MENU, MokoSelectMenuClass))
#define MOKO_IS_SELECT_MENU(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_SELECT_MENU))
#define MOKO_IS_SELECT_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_SELECT_MENU))
#define MOKO_SELECT_MENU_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_SELECT_MENU, MokoSelectMenuClass))

typedef struct _MokoSelectMenu MokoSelectMenu;
typedef struct _MokoSelectMenuClass MokoSelectMenuClass;

struct _MokoSelectMenu {
  GtkMenu   parent;
};

struct _MokoSelectMenuClass {
  GtkMenuClass  parent_class;
};

GType moko_select_menu_get_type (void);

MokoSelectMenu *moko_select_menu_new (ApplicationManagerData *appdata);

void moko_select_menu_popup (MokoSelectMenu *menu, 
                             GdkEventButton *event,
                             ApplicationManagerData *appdata, 
                             gpointer pkg);

G_END_DECLS

#endif


