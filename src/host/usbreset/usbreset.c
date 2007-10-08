/*
 * usbreset.c - Reset one or all USB devices
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
#include <errno.h>
#include <usb.h>


static void usage(const char *name)
{
	fprintf(stderr, "usage: %s [bus/device]\n", name);
	exit(1);
}


int main(int argc, const char **argv)
{
	struct usb_bus *usb_bus;
	struct usb_device *dev;
	int all = 0;
	int bus_num, dev_num;

	switch (argc) {
		case 1:
			all = 1;
			break;
		case 2:
			if (sscanf(argv[1], "%d/%d", &bus_num, &dev_num) != 2)
				usage(*argv);
			break;
		case 3:
			usage(*argv);
	}

	usb_init();
	usb_find_busses();
	usb_find_devices();
        for (usb_bus = usb_get_busses(); usb_bus; usb_bus = usb_bus->next)
		for (dev = usb_bus->devices; dev; dev = dev->next) {
			struct usb_dev_handle *handle;

			if (!all &&
			    (atoi(usb_bus->dirname) != bus_num ||
			    dev->devnum != dev_num))
				continue;

			handle = usb_open(dev);
			if (!handle) {
				fprintf(stderr, "usb_open: %s\n",
				    usb_strerror());
				continue;
			}
			if (usb_reset(handle) < 0 && 
			    (dev->descriptor.bDeviceClass != USB_CLASS_HUB ||
			    errno != EISDIR)) {
				fprintf(stderr, "usb_reset: %s\n",
				    usb_strerror());
				continue;
			}

			if (!all)
				return 0;

			fprintf(stderr,"reset %s/%s\n",
			    usb_bus->dirname, dev->filename);
		}

	if (!all) {
		fprintf(stderr, "no device %d/%d found\n", bus_num, dev_num);
		return 1;
	}

	return 0;
}
