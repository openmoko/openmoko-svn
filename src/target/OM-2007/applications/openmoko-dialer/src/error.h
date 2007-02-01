/***************************************************************************
 *            error.h
 *
 *  Fri Oct 13 18:54:40 2006
 *  Copyright  2006  User
 *  Email
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _ERROR_H
#define _ERROR_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef  _FIC_DEBUG
#define _FIC_DEBUG 1
#endif

#define DBG_ERROR_ON 1
#define DBG_WARNING_ON 1
#define DBG_FUN_ENTER_ON 1
#define DBG_FUN_LEAVE_ON 1
#define DBG_MESSAGE_ON 1
#define DBG_TRACE_ON 1



#ifdef  _FIC_DEBUG
/** @brief Define a debug message output */
#define DBG_MESSAGE(x...)   {if(DBG_MESSAGE_ON) {g_print(x);g_print("\n");}}

/** @brief Define a error message output */
#define DBG_ERROR(x...)  {if(DBG_ERROR_ON) {g_print("%s : %s : %d\nERROR:\n",__FILE__,__FUNCTION__,__LINE__);g_print(x);g_print("\n");}}

#define DBG_WARN(x...)  {if(DBG_WARNING_ON) {g_print("WARN:\n");g_print(x);g_print("\n");}}
#define DBG_ENTER()  {if(DBG_FUN_ENTER_ON) {g_print(">>>>>>>>%s : %d\n",__FUNCTION__,__LINE__);}}
#define DBG_LEAVE()  {if(DBG_FUN_LEAVE_ON) {g_print("%s : %d>>>>>>>\n",__FUNCTION__,__LINE__);}}
#define DBG_TRACE()  {if(DBG_TRACE_ON) {g_print("TRACE: %s : %d\n\n",__FUNCTION__,__LINE__);}}


#else

#define DBG(x...)
#define DBG_ERROR(x...)
#define DBG_WARN(x...)
#define DBG_ENTER()
#define DBG_LEAVE()
#define DBG_TRACE()

#endif






#ifdef __cplusplus
}
#endif

#endif                          /* _ERROR_H */
