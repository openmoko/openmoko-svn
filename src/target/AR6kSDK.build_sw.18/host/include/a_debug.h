#ifndef _A_DEBUG_H_
#define _A_DEBUG_H_
/*
 * Copyright (c) 2004-2006 Atheros Communications Inc.
 * All rights reserved.
 *
 * Copyright (c) 2004-2007 Atheros Communications Inc.
 * All rights reserved.
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

#include <a_types.h>
#include <a_osapi.h>

#define DBG_INFO		0x00000001
#define DBG_ERROR		0x00000002
#define DBG_WARNING		0x00000004
#define DBG_SDIO		0x00000008
#define DBG_HIF			0x00000010
#define DBG_HTC			0x00000020
#define DBG_WMI			0x00000040
#define DBG_WMI2		0x00000080
#define DBG_DRIVER		0x00000100

#define DBG_DEFAULTS	(DBG_ERROR|DBG_WARNING)

#ifdef UNDER_CE
#include "../os/wince/include/debug_wince.h"
#endif

#if defined(__linux__) && !defined(LINUX_EMULATION)
#include "../os/linux/include/debug_linux.h"
#endif

#ifdef REXOS
#include "../os/rexos/include/common/debug_rexos.h"
#endif

#endif
