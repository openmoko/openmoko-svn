/**
 * @file alldialog.h 
 * @brief Manager all dialog that be used in the application manager.
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
 *
 * @author Chaowei Song(songcw@fic-sh.com.cn)
 * @date 2006-10-23
 */

#ifndef _FIC_ALLDIALOG_H
#define _FIC_ALLDIALOG_H

#include <gtk/gtk.h>


GtkWidget *init_apply_dialog (void);

void show_message_to_user (gchar *msg);

gboolean create_file_selection (gchar **name);

#endif //_FIC_ALLDIALOG_H
