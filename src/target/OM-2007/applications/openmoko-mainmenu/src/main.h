/**
 *  @file main.h
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

#ifndef _MAIN_MENU_MAIN_H
#define _MAIN_MENU_MAIN_H

#include "fingermenu.h"
#include "stylusmenu.h"
#include "dbus-conn.h"
#include "mokodesktop.h"

typedef struct _MokoMainmenuApp MokoMainmenuApp;

struct _MokoMainmenuApp {
	MokoFingerMenu *fm;
	MokoStylusMenu *sm;
    MokoDesktopItem *top_item;
};

#endif /*_MAIN_MENU_MAIN_H*/
