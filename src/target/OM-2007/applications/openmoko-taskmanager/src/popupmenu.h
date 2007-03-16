/**
 *  popupmenu.h
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
 */

#ifndef _TASK_MANAGER_POPUP_MENU_H
#define _TASK_MANAGER_POPUP_MENU_H

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "list_view.h"
#include "misc.h"

void
moko_init_popup_menu (GtkWidget *my_widget, GdkEventButton *event, MokoTaskList *l);


#endif
