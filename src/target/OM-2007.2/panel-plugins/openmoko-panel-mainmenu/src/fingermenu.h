/**
 *  @file fingermenu.h
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
#ifndef _MOKO_FINGER_MENU_H_
#define _MOKO_FINGER_MENU_H_

#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>
#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-pixmap-button.h>
#include <libmokoui/moko-application.h>


#include "mainmenu.h"
#include "app-history.h"

G_BEGIN_DECLS
#define FINGERMENU_TYPE    (moko_finger_menu_get_type())
#define FINGERMENU(obj)    (G_TYPE_CHECK_INSTANCE_CAST((obj), FINGERMENU_TYPE, MokoFingerMenu))
#define FINEGERMENU_CLASS(kalss)    (G_TYPE_CHECK_CLASS_CAST((klass), FINGERMENU_TYPE, MokoFingerMenuClass))

typedef struct _MokoFingerMenu    MokoFingerMenu;
typedef struct _MokoFingerMenuClass    MokoFingerMenuClass;

struct _MokoFingerMenu{
    MokoApplication parent;

    MokoApplication *app;
    MokoMainMenu *mm;
    MokoFingerWindow *window;
    MokoFingerWheel *wheel;
    MokoFingerToolBox *toolbox;
    MokoAppHistory *history;
};

struct _MokoFingerMenuClass{
    MokoApplicationClass parent_class;
};

GType moko_finger_menu_get_type(void);

/*************/
/*Public  API*/
/*************/
MokoFingerMenu* moko_finger_menu_new ();

void moko_finger_menu_show (MokoFingerMenu *self);

void moko_finger_menu_hide (MokoFingerMenu *self);

void moko_finger_menu_build (MokoFingerMenu *self, MokoDesktopItem *item);

void moko_finger_menu_update_content (MokoFingerMenu *self, MokoDesktopItem *item);

MokoDesktopItem * moko_finger_menu_get_current_item (MokoFingerMenu *self);

void moko_finger_menu_set_current_item (MokoFingerMenu *self, MokoDesktopItem *item);

void moko_finger_menu_move_cursor_up(MokoFingerMenu *self);

void moko_finger_menu_move_cursor_down(MokoFingerMenu *self);

void moko_finger_menu_set_app_history(MokoFingerMenu *self, GdkPixbuf *pixbuf, MokoDesktopItem *item);

G_END_DECLS

#endif /*_MOKO_FINGER_MENU_H_*/
