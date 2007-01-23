/**
 *  @file errorcode.h
 *  @brief The error code of the all function return
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 *  @author Chaowei Song (songcw@fic-sh.com.cn)
 */
#ifndef _FIC_ERROR_CODE_H
#define _FIC_ERROR_CODE_H

/**
 * @brief All available error code
 */
typedef enum {
  OP_SUCCESS = 0,                /* Operation success */
  OP_MAMORY_MALLOC_ERROR,        /* Mamory malloc error */

  OP_SECTION_NAME_NULL,          /* The section of a package is NULL */

  OP_ERROR                       /* Operation error */
} ErrorCode;

#endif

