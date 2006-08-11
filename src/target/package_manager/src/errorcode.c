/**
 * @file errorcode.c - error code and debug infomation
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
 * @date 2006-7-28
 */

#include <gtk/gtk.h>

#include "errorcode.h"

/**
 * Translate error code to error message.
 * @param err Error code
 * @return Error message pointer
 */
gchar *trans_error_code(gint err)
{
  static gchar       unknown[] = "unkown error";

  return unknown;
}

