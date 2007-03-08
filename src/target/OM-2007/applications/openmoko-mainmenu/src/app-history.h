/**
 *  @file app-history.h
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
#ifndef _MOKO_APP_HISTORY_H
#define _MOKO_APP_HISTORY_H

#include "mokodesktop.h"
#include <libmokoui/moko-pixmap-button.h>
#include <libmokoui/moko-finger-tool-box.h>

#define MAX_RECORD_APP    4

typedef struct {
	MokoDesktopItem *item[MAX_RECORD_APP];
	MokoPixmapButton *btn[MAX_RECORD_APP];
} MokoAppHistory;

/**
 * @brief moko_history_app_init
 * @param toolboox    MokoFingerToolBox *
 * @return MokoAppHistory instance
 */
MokoAppHistory * moko_app_history_init (MokoFingerToolBox *toolbox);

/**
 * @brief moko_history_app_set
 * @param self    MokoAppHistory *,
 * @param path    const char *,
 * @param item    MokoDesktopItem *,
 * @return gboolean
 */
gboolean moko_app_history_set (MokoAppHistory *self, GdkPixbuf *pixbuf, MokoDesktopItem *item);

/**
 * @brief moko_app_history_free
 * @brief self    MokoAppHistory *,
 * @return none
 */
void moko_app_history_free (MokoAppHistory *self);

#endif /*_MOKO_APP_HISTORY_H*/
