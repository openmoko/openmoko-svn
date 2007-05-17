/*
 *  Footer - Task manager application
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

#ifndef _TASKMANAGER_H_
#define _TASKMANAGER_H_

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>
#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-pixmap-button.h>

#include <gtk/gtk.h>

typedef struct _MokoTaskManager
{
  MokoApplication *app;
  MokoFingerWindow *window;
  MokoFingerWheel *wheel;
  MokoFingerToolBox *toolbox;
  GtkTable *table;
  MokoPixmapButton *go_to;
  MokoPixmapButton *kill;
  MokoPixmapButton *kill_all;
  MokoPixmapButton *quit;
} MokoTaskManager;


void taskmanager_init (MokoTaskManager *tm);

#endif
