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
#define _OPENMOKO_DIALER_WINDOW_HISTORY_H

#ifdef __cplusplus



extern "C"
{
#endif


  GtkWidget *create_window_history_content (MOKO_DIALER_APP_DATA *
                                            p_dialer_data);
  gint window_history_init (MOKO_DIALER_APP_DATA * p_dialer_data);
  gint history_build_history_list_view (MOKO_DIALER_APP_DATA * p_dialer_data);
  void window_history_prepare (MOKO_DIALER_APP_DATA * appdata);
  GtkWidget *history_create_menu_history (MOKO_DIALER_APP_DATA *
                                          p_dialer_data);
  gint add_histroy_entry (MOKO_DIALER_APP_DATA * appdata, HISTORY_TYPE type,
                          const char *name, const char *number,
                          const char *picpath, char *time, char *date,
                          int durationsec);
  gint history_list_view_add (MOKO_DIALER_APP_DATA * appdata,
                              HISTORY_ENTRY * entry);
#ifdef __cplusplus
}
#endif

#endif
