/*
 *  Footer - Task manager menu
 *
 *  Authored by Daniel Willmann <daniel@totalueberwachung.de>
 *
 *  Copyright (C) 2007 OpenMoko, Inc.
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
 */

#ifndef _TASKMENU_H_
#define _TASKMENU_H_

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>

#include <gtk/gtk.h>

typedef struct _MokoTaskMenu
{
    GtkMenu *menu;
    Window *list;
    int listnr;
} MokoTaskMenu;


void moko_taskmenu_init (MokoTaskMenu *tm);
gboolean moko_update_task_list (Display *dpy, MokoTaskMenu *tm);
void moko_taskmenu_populate(Display *dpy, MokoTaskMenu *tm);
void moko_taskmenu_popup_positioning_cb( GtkMenu* menu, gint* x, gint* y, gboolean* push_in, GtkWidget *parent );

#endif
