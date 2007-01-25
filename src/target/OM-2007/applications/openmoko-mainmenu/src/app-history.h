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

#include <libmokoui/moko-pixmap-button.h>

#define MAX_RECORD_APP 4

/**
 * @brief moko_history_app_fill
 * @param btn    MokoPixmapButton **
 * @param path    const char *
 * @return NONE
 */
void moko_hisory_app_fill(MokoPixmapButton **btn, const char *path);
#endif /*_MOKO_APP_HISTORY_H*/
