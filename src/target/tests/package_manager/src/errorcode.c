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
#ifdef DEBUG

gchar *trans_error_code(gint err)
{
  static gchar       unknown[] = "unkown error";
  static gchar       mem_malloc_fail[] = "OP_MEMORY_MALLOC_FAIL";
  static gchar       find_section[] = "OP_FIND_CORRESPONDING_SECTION";
  static gchar       reach_end[] = "OP_REACH_SECTION_END";
  static gchar       find_position[] = "OP_FIND_THE_POSITION";

  switch(err)
    {
      case OP_MEMORY_MALLOC_FAIL:
        return mem_malloc_fail;

      case OP_FIND_CORRESPONDING_SECTION:
        return find_section;

      case OP_REACH_SECTION_END:
        return reach_end;

      case OP_FIND_THE_POSITION:
        return find_position;

      default:
        return unknown;
    }

  return unknown;
}

#else // ifndef DEBUG

gchar *trans_error_code (gint err)
{
  return NULL;
}

#endif // end ifndef DEBUG
