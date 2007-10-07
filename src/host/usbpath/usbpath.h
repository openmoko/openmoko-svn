/*
 * usbpath.h - Functions to convert the physical location of a device to its
 *             USB device number
 *
 * (C) 2007 by OpenMoko, Inc.
 * Written by Werner Almesberger <werner@openmoko.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef USBPATH_H
#define USBPATH_H

#include <usb.h>
#include <linux/usbdevice_fs.h>

/* portinfo.c */

void usb_get_portinfo(const struct usb_device *hub,
    struct usbdevfs_hub_portinfo *portinfo);


/* path2devnum.c */

/*
 * Note that the port numbers in ports[] are 0-based.
 *
 * Return codes:
 * 0  device not found
 * N  device number
 */

int usb_portlist2devnum(const struct usb_bus *bus, const int *ports,
    int num_ports);

/*
 * Note that the port numbers in the string are 1-based.
 *
 * Return codes:
 * -1  invalid path specification
 * 0   device not found
 * N   device number
 */

int usb_path2devnum(const char *path);


/* devnum2path.c */

/*
 * Note that the port numbers in ports[] are 0-based.
 *
 * Return codes:
 * -1  buffer overflow
 * 0   device not found
 * N   N-1 ports placed into "ports".
 */

int usb_devnum2portlist(const struct usb_bus *bus, int devnum, int *ports,
    int num_ports);

/*
 * Note that the port numbers in the string are 1-based.
 *
 * Return codes:
 * -1  buffer overflow
 * 0   device not found
 * N   returned N characters (including the trailing NUL)
 */

int usb_devnum2path(int busnum, int devnum, char *path, int path_size);

#endif /* !USBPATH_H */
