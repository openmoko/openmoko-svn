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

/** @brief operation success*/
#define OP_SUCCESS                               0

/** @brief The size of window is too small, can not resize the widget */
#define OP_WINDOW_SIZE_MINIMUM                  1001

/** @brief The widget window don't need resize */
#define OP_WINDOW_SIZE_NEED_NOT_CHANGE          1002

/** @brief Some widget not find */
#define OP_WINDOW_SIZE_WIDGET_NOT_FIND          1003


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

