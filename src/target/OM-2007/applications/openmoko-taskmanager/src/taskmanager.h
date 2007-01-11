/**
 * @file taskmanager.h
 * @brief openmoko-taskmanager taskmanager.c based on callbacks.h and list_view.h.
 * @author Sun Zhiyong
 * @date 2006-10
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef _TASK_MANAGER_H
#define _TASK_MANAGER_H

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>
#include <libmokoui/moko-pixmap-button.h>


#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <X11/Xlib.h>

#include "list_view.h"

//#ifndef GTK_STOCK_CLOSE
//#define GTK_STOCK_CLOSE "button_colse"
//#endif

#define TASK_MANAGER_PROPERTY_WIDTH		200
#define TASK_MANAGER_PROPERTY_HEIGHT	564 
#define TASK_MANAGER_PROPERTY_X		0
#define TASK_MANAGER_PROPERTY_Y 		45 

#define _(string) (string)

typedef struct _MokoTaskManager MokoTaskManager;

struct _MokoTaskManager {
    MokoApplication *app;
    
    MokoFingerWindow *window;//??
    GtkWidget *gtk_window;//??
    MokoFingerWheel *wheel;
    MokoFingerToolBox *toolbox;
    MokoTaskList *l;
    
    MokoPixmapButton *go_to;
    MokoPixmapButton *kill;
    MokoPixmapButton *kill_all;
    MokoPixmapButton *quit;

    };
 
#endif /*taskmanager.h*/
