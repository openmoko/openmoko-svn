/**
 * @file errorcode.h - error code and debug infomation
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
 * @date 2006-8-10
 */

#ifndef _FIC_ERRORCODE_H
#define _FIC_ERRORCODE_H

enum error_code {
  /** @brief operation success*/
  OP_SUCCESS = 0,

  /** @brief Memory malloc fail */
  OP_MEMORY_MALLOC_FAIL,

  /** @brief The widget tree view package not find */
  OP_NOT_FIND_TREE_VIEW_PACKAGE,

  /** @brief Can't init the ipkg model */
  OP_INIT_IPKG_MODEL_FAIL,



  /** @brief The system error and common error split */
  OP_SPLIT_OF_SYSTEM_AND_COMMON_ERROR,



  /** @brief The size of window is too small, can not resize the widget */
  OP_WINDOW_SIZE_MINIMUM,

  /** @brief The widget window don't need resize */
  OP_WINDOW_SIZE_NEED_NOT_CHANGE,

  /** @brief Some widget not find */
  OP_WINDOW_SIZE_WIDGET_NOT_FIND,

  /** @brief Get installed package list error */
  OP_GET_INSTALLED_PACKAGE_LIST_ERROR,

  /** @brief Can't malloc memory */
  OP_MALLOC_MEMORY_ERROR,

  /** @brief Find corresponding section */
  OP_FIND_CORRESPONDING_SECTION,

  /** @brief Not find corresponding section and reach the end of section queue */
  OP_REACH_SECTION_END,

  /** @brief Not find corresponding section and find the insert position */
  OP_FIND_THE_POSITION
};

#ifndef _FIC_DEBUG
#define _FIC_DEBUG
#endif

#ifdef  _FIC_DEBUG
/** @brief Define a debug message output */
#define DBG(x...) g_print("%s : %s : %d:\n  ",__FILE__,__FUNCTION__,__LINE__);g_print(x)

/** @brief Define a error message output */
#define ERROR(x...) g_print("ERROR:\n");DBG(x)
#else
#define DBG(x...)
#define ERROR(x...)
#endif

gchar *trans_error_code(gint err);

#endif

