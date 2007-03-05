/**
 *  @file mainmenu.c
 *  @brief The Main Menu in the Openmoko
 *  
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *  
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 */

#ifndef _OPEN_MOKO_MAIN_MENU_H
#define _OPEN_MOKO_MAIN_MENU_H

#include <gtk/gtkwidget.h>
#include <glib-object.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>

#include "mokoiconview.h"
#include "mokodesktop.h"

enum { 
    PIXBUF_COLUMN,
    TEXT_COLUMN,
    OBJECT_COLUMN,
    NUM_COLs
    };

G_BEGIN_DECLS
/* property(s)*/
#define COLUMN_NUM    3
#define ITEM_WIDTH    140
#define ITEM_MARGIN    10
#define ROW_SPACING    0
#define COLUMN_SPACING    20
#define ITME_TOTAL_WIDTH    5
#define DECORATION_WIDTH    10

#define PIXBUF_WIDTH    120 
#define PIXBUF_HEIGHT   120

#define SECTION_ALG_X    0.6
#define SECTION_ALG_Y    0.5
#define SECTION_X_PADDING    0
#define SECTION_Y_PADDING    5
#define ITEM_TOTAL_ALG_X    0.5
#define ITEM_TOTAL_ALG_Y    0.9

/*Font styles*/
#define FONT_SIZE_SECTION 	12*PANGO_SCALE
#define FONT_SIZE_ITEM 		11*PANGO_SCALE
	
#define MAINMENU_TYPE				(moko_main_menu_get_type())
#define MAINMENU(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), MAINMENU_TYPE, MokoMainMenu))
#define MAINMENU_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), MAINMENU_TYPE, MokoMainMenuClass))
#define IS_MAINMENU(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), MAINMENU_TYPE))
#define IS_MAINMENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), MAINMENU_TYPE))


typedef struct _MokoMainMenu 			MokoMainMenu;
typedef struct _MokoMainMenuClass 	MokoMainMenuClass;

struct _MokoMainMenu {
    GtkVBox vbox;

    MokoDesktopItem *top_item;
    MokoDesktopItem *current;
    MokoIconView *icon_view;
    GtkListStore *list_store;
    GtkWidget *scrolled;
    GtkHBox *hbox;
    GtkLabel *section_name;
    GtkLabel *item_total;
};

struct _MokoMainMenuClass {
    GtkVBoxClass parent_class;

    void(*moko_main_menu_function)(MokoMainMenu *mm);
    };

GType 
moko_main_menu_get_type (void);

GtkWidget*
moko_main_menu_new ();

void 
moko_main_menu_clear (MokoMainMenu *mm);

gboolean
moko_main_menu_update_content (MokoMainMenu *mm, MokoDesktopItem *item);

void
moko_main_menu_update_item_total_label (MokoMainMenu *mm);

void
moko_main_menu_update_section_name_label (MokoIconView *mm);

G_END_DECLS

#endif /*main_menu.h*/
