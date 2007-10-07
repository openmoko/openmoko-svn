/*
 * usbpath.c - Look up the number of a USB device by its physical location
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

#include <usb.h>
#include "usbpath.h"


#define MAX_PATH 256


static void usage(const char *name)
{
	fprintf(stderr, "usage: %s bus-port. ... .port\n", name);
	fprintf(stderr, "       %s bus/dev\n", name);
	exit(1);
}


static void do_path(const char *path)
{
	int res;

	res = usb_path2devnum(path);
	if (res < 0) {
		fprintf(stderr, "invalid path syntax\n");
		exit(1);
	}
	if (!res) {
		fprintf(stderr, "no such device\n");
		exit(1);
	}
	printf("%d/%d\n", atoi(path), res);
}


static void do_devnum(const char *spec)
{
	char path[MAX_PATH];
	int bus, devnum, res;

	if (sscanf(spec, "%d/%d", &bus, &devnum) != 2) {
		fprintf(stderr," invalid bus/devnum syntax\n");
		exit(1);
	}
	res = usb_devnum2path(bus, devnum, path, MAX_PATH);
	if (res < 0) {
		fprintf(stderr, "buffer overflow\n");
		exit(1);
	}
	if (!res) {
		fprintf(stderr, "no such device\n");
		exit(1);
	}
	printf("%s\n", path);
}


int main(int argc, char **argv)
{
	if (argc != 2)
		usage(*argv);

	usb_init();
	usb_find_busses();
	usb_find_devices();

	if (strchr(argv[1], '/'))
		do_devnum(argv[1]);
	else
		do_path(argv[1]);
	return 0;
}
