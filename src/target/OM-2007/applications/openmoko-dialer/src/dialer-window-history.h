/*  openmoko-dialer-window-history.h
 *
 *  Authored By Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */
#include "moko-dialer-includes.h"

#ifndef _OPENMOKO_DIALER_WINDOW_HISTORY_H

/* columns available in the history list store */
enum history_columns {
  HISTORY_NUMBER_COLUMN, /* number for re-dial */
  HISTORY_DSTART_COLUMN, /* call time, used for sorting */
  HISTORY_ICON_NAME_COLUMN, /* icon name for display */
  HISTORY_DISPLAY_TEXT_COLUMN, /* name or number for display */
  HISTORY_CALL_TYPE_COLUMN, /* Used for identifying the type of call */
  HISTORY_ENTRY_POINTER /*  needed for deletes */
};


gint window_history_init (MokoDialerData * p_dialer_data);

#define _OPENMOKO_DIALER_WINDOW_HISTORY_H

#endif
