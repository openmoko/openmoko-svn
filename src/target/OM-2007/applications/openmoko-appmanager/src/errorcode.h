/**
 * @file errorcode.h 
 * @brief Error code and debug infomation
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
 * @date 2006-9-30
 */

#ifndef _FIC_ERRORCODE_H
#define _FIC_ERRORCODE_H

/**
 * @brief The error code define.
 */
enum error_code {
  OP_SUCCESS = 0, ///<operation success

  OP_MEMORY_MALLOC_FAIL, ///<Memory malloc fail

  OP_NOT_FIND_TREE_VIEW_PACKAGE,  ///<The widget tree view package not find 

  OP_PACKAGE_STORE_NOT_INIT_CORRECTLY,  ///<The store of package list not init correctly.

  OP_PACKAGE_LIST_NOT_INIT_CORRECTLY,    ///<The package list of a section is not correct initial.

  OP_INIT_IPKG_MODEL_FAIL,   ///<Can't init the ipkg model


  OP_SPLIT_OF_SYSTEM_AND_COMMON_ERROR,  ///<The system error and common error split


  OP_SECTION_NAME_NULL,   ///<The name of section is NULL.

  OP_PACKAGE_NAME_NULL,   ///<The name of package is NULL.

  OP_INSERT_PACKAGE_SUCCESS,   ///<Insert the package to list success.

  OP_PACKAGE_IS_NOT_UPGRADEABLE,      ///<The package is not an upgradeable package

  OP_PACKAGE_IS_UPGRADEABLE,    ///<Insert the package to list, and the package is upgradeable.

  OP_GET_INSTALLED_PACKAGE_LIST_ERROR,  ///<Get installed package list error

  OP_FILTER_ID_NOT_CORRECT,  ///<The filter id is not a correct one

  OP_MALLOC_MEMORY_ERROR,  ///<Can't malloc memory

  OP_FIND_CORRESPONDING_SECTION,  ///<Find corresponding section */

  OP_REACH_SECTION_END,   ///<Not find corresponding section and reach the end of section queue

  OP_FIND_THE_POSITION   ///<Not find corresponding section and find the insert position
};

#ifndef _FIC_DEBUG
#define _FIC_DEBUG
#endif

#ifdef  _FIC_DEBUG
/** @brief Define a debug message output */
#define DBG(x...)     do { \
            g_print("%s : %s : %d:\n  ",__FILE__,__FUNCTION__,__LINE__);    \
            g_print(x);     \
            g_print("\n");     \
            }while (0)

/** @brief Define a error message output */
#define ERROR(x...)    do {      \
              g_print("ERROR:\n");      \
              DBG(x);             \
              }while (0)
#else
#define DBG(x...)
#define ERROR(x...)
#endif

#endif

