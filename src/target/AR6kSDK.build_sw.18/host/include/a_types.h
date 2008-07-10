#ifndef _A_TYPES_H_
#define _A_TYPES_H_
/*
 * $Id: //depot/sw/releases/olca2.0-GPL/host/include/a_types.h#1 $
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

#if defined(__linux__) && !defined(LINUX_EMULATION)
#include "../os/linux/include/athtypes_linux.h"
#endif

#ifdef UNDER_CE
#include "../os/wince/include/athtypes_wince.h"
#endif

#ifdef REXOS
#include "../os/rexos/include/common/athtypes_rexos.h"
#endif

#endif /* _ATHTYPES_H_ */
