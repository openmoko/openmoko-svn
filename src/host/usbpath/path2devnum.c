/*
 * path2devnum.c - Translate a bus/port/... path to the USB device number
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


static int devnum_by_port(const struct usb_device *hub, int port)
{
	struct usbdevfs_hub_portinfo portinfo;

	usb_get_portinfo(hub, &portinfo);
	if (port >= portinfo.nports)
		return 0;
	return portinfo.port[port];
}


static int find_hub_port(const struct usb_device *hub, const int *ports,
    int num_ports)
{
	const struct usb_device *dev = NULL;
	int devnum, i;

	devnum = devnum_by_port(hub, *ports);
	if (!devnum)
		return 0;
	if (num_ports == 1)
		return devnum;
	for (i = 0; i != hub->num_children; i++) {
		dev = hub->children[i];
		if (dev->devnum == devnum)
			break;
	}
	assert(dev);
	if (dev->descriptor.bDeviceClass != USB_CLASS_HUB)
		return 0;
	return find_hub_port(dev, ports+1, num_ports-1);
}


static int is_child(const struct usb_device *devs,
    const struct usb_device *child)
{
	while (devs) {
		int i;

		for (i = 0; i != devs->num_children; i++)
			if (devs->children[i] == child)
				return 1;
		devs = devs->next;
	}
	return 0;
}


static const struct usb_device *find_root_hub(struct usb_device *devs)
{
	const struct usb_device *hub;

	for (hub = devs; hub; hub = hub->next) {
		if (hub->descriptor.bDeviceClass != USB_CLASS_HUB)
			continue;
		if (!is_child(devs, hub))
			break;
	}
	return hub;
}


int usb_portlist2devnum(const struct usb_bus *bus, const int *ports,
    int num_ports)
{
	const struct usb_device *root;

	root = find_root_hub(bus->devices);
	if (!num_ports)
		return root->devnum;
	return find_hub_port(root, ports, num_ports);
}


int usb_path2devnum(const char *path)
{
	const struct usb_bus *bus;
	const char *p;
	int nums = 1, n = 0, res;
	int *num;

	if (!*path)
		return -1;
	for (p = path; *p; p++)
		if (*p == '-' || *p == '.')
			nums++;
	num = malloc(sizeof(int)*nums);
	if (!num) {
		perror("malloc");
		exit(1);
	}
	while (1) {
		char *end;

		num[n] = strtoul(path, &end, 10);
		if (!num[n]) {
			free(num);
			return -1;
		}
		if (n)
			num[n]--;
		n++;
		if (!*end && n == nums)
			break;
		if (*end != (n == 1 ? '-' : '.')) {
			free(num);
			return -1;
		}
		path = end+1;
	}
	for (bus = usb_get_busses(); bus; bus = bus->next)
		if (atoi(bus->dirname) == num[0])
			break;
	if (!bus) {
		free(num);
		return 0;
	}
	res = usb_portlist2devnum(bus, num+1, nums-1);
	free(num);
	return res;
}
