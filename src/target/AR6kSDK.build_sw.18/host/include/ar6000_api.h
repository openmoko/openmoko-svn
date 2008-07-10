#ifndef _AR6000_API_H_
#define _AR6000_API_H_
/*
 * Copyright (c) 2004-2005 Atheros Communications Inc.
 * All rights reserved.
 *
 * This file contains the API to access the OS dependent atheros host driver
 * by the WMI or WLAN generic modules.
 *
 * $Id: //depot/sw/releases/olca2.0-GPL/host/include/ar6000_api.h#1 $
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
#include "../os/linux/include/ar6xapi_linux.h"
#endif

#ifdef UNDER_CE
#include "../os/wince/include/ar6xapi_wince.h"
#endif

#ifdef REXOS
#include "../os/rexos/include/common/ar6xapi_rexos.h"
#endif

#endif /* _AR6000_API_H */

