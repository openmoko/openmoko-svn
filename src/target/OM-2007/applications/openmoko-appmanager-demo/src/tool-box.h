/**
 *  @file tool-box.h
 *  @brief The tool box in the main window
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
#ifndef _FIC_TOOL_BAR_H
#define _FIC_TOOL_BAR_H

#include <gtk/gtk.h>

#include <libmokoui/moko-paned-window.h>
#include <libmokoui/moko-tool-box.h>

MokoToolBox *tool_box_new_for_window (MokoPanedWindow *window);

#endif

