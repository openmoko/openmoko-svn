/*
 * portinfo.c - Obtain the port to device mapping of a USB hub
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
#include <fcntl.h>
#include <sys/ioctl.h>

#include <usb.h>
#include <linux/usbdevice_fs.h>


#define USBDEVFS_PREFIX "/proc/bus/usb"
#define MAXPATH 256


void usb_get_portinfo(const struct usb_device *hub,
    struct usbdevfs_hub_portinfo *portinfo)
{
	struct usbdevfs_ioctl command;
	char buf[MAXPATH];
	int fd, ret;

	snprintf(buf, MAXPATH, "%s/%s/%s", USBDEVFS_PREFIX, hub->bus->dirname,
	    hub->filename);
	fd = open(buf, O_RDWR);
	if (fd < 0) {
		perror(buf);
		exit(1);
	}
	command.ifno = 0;
	command.ioctl_code = USBDEVFS_HUB_PORTINFO;
	command.data = portinfo;
	ret = ioctl(fd, USBDEVFS_IOCTL, &command);
	if (ret < 0) {
		perror("ioctl(USBDEVFS_IOCTL)");
		exit(1);
	}
	if (close(fd) < 0) {
		perror("close");
		exit(1);
	}
}
