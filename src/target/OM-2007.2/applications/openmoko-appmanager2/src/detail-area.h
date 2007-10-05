/*
 *  @file detail-area.h
 *  @brief The detail area in the main window
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
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
#ifndef _FIC_DETAIL_AREA_H
#define _FIC_DETAIL_AREA_H

#include <gtk/gtk.h>

#include "appmanager-data.h"

GtkWidget *detail_area_new (ApplicationManagerData *appdata);

void detail_area_update_info (ApplicationManagerData *appdata, 
                              gpointer pkg);

#endif

