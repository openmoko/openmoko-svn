/*
 * Copyright (c) 2007 OpenMoko, Inc.
 * Modified for use in QEMU by Andrzej Zaborowski <andrew@openedhand.com>
 */
/*

  $Id: compat.h,v 1.54 2007/05/08 19:41:35 pkot Exp $

  G N O K I I

  A Linux/Unix toolset and driver for the mobile phones.

  This file is part of gnokii.

  Gnokii is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Gnokii is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with gnokii; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Copyright (C) 1999-2000 Hugh Blemings, Pavel Janik
  Copyright (C) 2002-2004 BORBELY Zoltan, Pawel Kot
  Copyright (C) 2002      Feico de Boer, Markus Plail
  Copyright (C) 2003      Marcus Godehardt, Ladis Michl

  Header file for various platform compatibility.

*/

#ifndef	_gnokii_compat_h
#define	_gnokii_compat_h

#include <stdlib.h>

#include <stdarg.h>
#include <strings.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <limits.h>
#include <unistd.h>
#include <termios.h>

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;

#include "config-host.h"
#define VERSION	"QEMU " QEMU_VERSION

struct gsmmodem_info_s {
	void (*write)(void *opaque, const char *fmt, ...);
	void *opaque;
};

#define GNOKII_API

#undef timerisset
#undef timerclear
#undef timercmp
#undef timeradd
#undef timersub

/* The following code is borrowed from glibc, please don't reindent it */

/* Convenience macros for operations on timevals.
   NOTE: `timercmp' does not work for >= or <=.  */
# define timerisset(tvp)	((tvp)->tv_sec || (tvp)->tv_usec)
# define timerclear(tvp)	((tvp)->tv_sec = (tvp)->tv_usec = 0)
# define timercmp(a, b, CMP) 						      \
  (((a)->tv_sec == (b)->tv_sec) ? 					      \
   ((a)->tv_usec CMP (b)->tv_usec) : 					      \
   ((a)->tv_sec CMP (b)->tv_sec))
# define timeradd(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;			      \
    if ((result)->tv_usec >= 1000000)					      \
      {									      \
	++(result)->tv_sec;						      \
	(result)->tv_usec -= 1000000;					      \
      }									      \
  } while (0)
# define timersub(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;			      \
    if ((result)->tv_usec < 0) {					      \
      --(result)->tv_sec;						      \
      (result)->tv_usec += 1000000;					      \
    }									      \
  } while (0)

/*
 * The following code was taken from W. Richard Stevens'
 * "UNIX Network Programming", Volume 1, Second Edition.
 *
 * We need the newer CMSG_LEN() and CMSG_SPACE() macros, but few
 * implementations support them today.  These two macros really need
 * an ALIGN() macro, but each implementation does this differently.
 */

#ifndef CMSG_LEN
#  define CMSG_LEN(size) (sizeof(struct cmsghdr) + (size))
#endif

#ifndef CMSG_SPACE
#  define CMSG_SPACE(size) (sizeof(struct cmsghdr) + (size))
#endif

/* Get rid of long defines. Use #if __unices__ */
#define __unices__ defined(__svr4__) || defined(__FreeBSD__) || defined(__bsdi__) || defined(__MACH__) || defined(__OpenBSD__) || defined(__NetBSD__)

#define _(x) (x)
#define N_(x) (x)

#endif
