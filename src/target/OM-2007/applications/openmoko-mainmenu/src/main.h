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

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>
#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-pixmap-button.h>

#include "mainmenu.h"
#include "app-history.h"

typedef struct _MokoMainmenuApp MokoMainmenuApp;

struct _MokoMainmenuApp {
    MokoApplication *app;
    
    MokoFingerWindow *window;
    MokoFingerWheel *wheel;
    MokoFingerToolBox *toolbox;
    MokoMainMenu *mm;
    MokoPixmapButton *history[MAX_RECORD_APP];
};

#endif /*main.h*/
