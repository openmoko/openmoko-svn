#ifndef _A_OSAPI_H_
#define _A_OSAPI_H_
/*
 * $Id: //depot/sw/releases/olca2.0-GPL/host/include/a_osapi.h#1 $
 *
 * This file contains the definitions of the basic atheros data types.
 * It is used to map the data types in atheros files to a platform specific
 * type.
 *
 * Copyright 2003-2005 Atheros Communications, Inc.,  All Rights Reserved.
 *
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation;
 * 
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 * 
 * 
 *
 */

#ifdef __linux__
#include "../os/linux/include/osapi_linux.h"
#endif

#ifdef UNDER_CE
#include "../os/wince/include/osapi_wince.h"
#include "../os/wince/ndis/netbuf.h"
#if defined __cplusplus || defined __STDC__
extern "C"
#endif
A_UINT32 a_copy_from_user(void *to, const void *from, A_UINT32 n);
#endif

#ifdef REXOS
#include "../os/rexos/include/common/osapi_rexos.h"
#endif

#endif /* _OSAPI_H_ */
