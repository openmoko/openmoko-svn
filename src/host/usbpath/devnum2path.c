/*
 * devnum2path.c - Translate a USB device number to the bus/port/... path
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <usb.h>
#include <linux/usbdevice_fs.h>

#include "usbpath.h"


#define MAX_LEVELS 10


static int port_by_devnum(const struct usb_device *hub, int devnum)
{
	struct usbdevfs_hub_portinfo portinfo;
	int i;

	usb_get_portinfo(hub, &portinfo);
	for (i = 0; i != portinfo.nports; i++)
		if (portinfo.port[i] == devnum)
			return i;
	return -1;
}


static int recurse_dev(const struct usb_device *devs, int *ports,
    int num_ports, const struct usb_device *child)
{
	const struct usb_device *dev;
	int n = 0, i;

	for (dev = devs; dev; dev = dev->next)
		for (i = 0; i != dev->num_children; i++)
			if (dev->children[i] == child)
				goto found;
	return 0;

found:
	n = recurse_dev(devs, ports, num_ports, dev);
	if (n < 0)
		return i;
	if (n == num_ports)
		return -1;
	ports[n] = port_by_devnum(dev, child->devnum);
	assert(ports[n] >= 0);
	return n+1;
}


int usb_devnum2portlist(const struct usb_bus *bus, int devnum, int *ports,
    int num_ports)
{
	const struct usb_device *dev;

	for (dev = bus->devices; dev; dev = dev->next)
		if (dev->devnum == devnum)
			break;
	if (!dev)
		return 0;
	return recurse_dev(bus->devices, ports, num_ports, dev)+1;
}


static int add_int(char **path, int *path_size, int value)
{
	int v, n = 0;

	for (v = value; v; v /= 10)
		n++;
	if (!n)
		n++;
	if (*path_size < n-1)
		return -1;
	n = sprintf(*path, "%d", value);
	(*path) += n;
	(*path_size) += n;
	return n;
}


int usb_devnum2path(int busnum, int devnum, char *path, int path_size)
{
	const struct usb_bus *bus;
	int ports[MAX_LEVELS];
	int n, i, len, res;

	for (bus = usb_get_busses(); bus; bus = bus->next)
		if (atoi(bus->dirname) == busnum)
			break;
	n = usb_devnum2portlist(bus, devnum, ports, MAX_LEVELS);
	if (n < 0)
		return -1;
	if (!n) {
		if (path_size)
			*path = 0;
		return 0;
	}
	len = add_int(&path, &path_size, atoi(bus->dirname));
	for (i = 0; i != n-1; i++) {
		if (!path_size--)
			return -1;
		*path++ = i ? '.' : '-';
		res = add_int(&path, &path_size, ports[i]+1);
		if (res < 0)
			return -1;
		len += res+1;
	}
	return len+1;
}
