/**
 * @file main.h
 * @brief openmoko-taskmanager head files based on main.h.
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

#ifndef _MAIN_H_
#define _MAIN_H_

#include <gtk/gtk.h>
#include "footer.h"

typedef struct _MokoFooterApp
{
    GtkWidget *toplevel_window;
    GtkWidget *footer;
    GdkWindow *target_window;
} MokoFooterApp;

#endif /* main.h */
