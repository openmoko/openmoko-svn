/*
 * norwp.c - Control the GTA02 NOR_WP line through the debug v3 board
 *
 * Copyright (C) 2008 by OpenMoko, Inc.
 * Written by Werner Almesberger <werner@openmoko.org>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ftdi.h>


#define nNOR_WP	0x10	/* nNOR_WP is at pin 20, ADBUS4/GPIOL0 */


static const struct id {
	uint16_t vendor;
	uint16_t product;
} ids[] = {
	{ 0x0403, 0x6010 },	/* FTDI default IDs */
	{ 0x1457, 0x5118 },	/* Neo1973 debug board (FIC) */
	{ 0x1d50, 0x5118 },	/* Neo1973 debug board (OpenMoko) */
	{ 0 }
};


static struct ftdi_context ftdi;


static void open_ftdi(void)
{
	const struct id *id;

	if (ftdi_init(&ftdi) < 0) {
		fprintf(stderr, "ftdi_init failed\n");
		exit(1);
	}
	for (id = ids; id->vendor; id++)
		if (ftdi_usb_open(&ftdi, id->vendor, id->product) >= 0)
			break;
	if (!id->vendor) {
		fprintf(stderr, "no debug board found\n");
		exit(1);
	}

#if 0	/* not needed */
	fprintf(stderr, "%04x:%04x\n", id->vendor, id->product);
	ftdi_set_interface(&ftdi, INTERFACE_A);
	ftdi_usb_reset(&ftdi);

	ftdi_set_latency_timer(&ftdi, 2);
#endif

	if (ftdi_set_bitmode(&ftdi, 0x0b, BITMODE_MPSSE) < 0) {
		fprintf(stderr, "ftdi_set_bitmode: %s\n",
		    ftdi_get_error_string(&ftdi));
		exit(1);
	}
}


static void norwp_query(void)
{
	uint8_t buf[1];
	ssize_t got;

	/* @@@FIXME: it takes 2-3 tries to get the device to respond. Adding
           a "SEND_IMMEDIATE" command doesn't help. */
again:
	buf[0] = GET_BITS_LOW;
	if (ftdi_write_data(&ftdi, buf, 1) < 0) {
		fprintf(stderr, "ftdi_write_data: %s\n",
		    ftdi_get_error_string(&ftdi));
		exit(1);
	}
	do {
		got = ftdi_read_data(&ftdi, buf, 1);
		if (got < 0) {
			fprintf(stderr, "ftdi_read_data: %s\n",
			    ftdi_get_error_string(&ftdi));
			exit(1);
		}
		if (!got)
			goto again;
	}
	while (!got);
	printf("%s\n", buf[0] & nNOR_WP ? "rw" : "ro");
}


static void norwp_set(int rw)
{
	uint8_t buf[3];

	/*
	 * Mode  nNOR_WP  GPIOL0 (pin 20)
	 *
	 * ro    L	  input (very weak pull-up)
	 * rw    H	  output-H
	 */

	buf[0] = SET_BITS_LOW;

	/*
	 * We use the defaults from OpenOCD's jtag/ft2232.c. Note that it
	 * already sets nNOR_WP (which it calls nOE).
	 */
	buf[1] = 0x08 | (rw ? nNOR_WP : 0);
	buf[2] = 0x1b | nNOR_WP;
	if (ftdi_write_data(&ftdi, buf, 3) < 0) {
		fprintf(stderr, "ftdi_write_data: %s\n",
		    ftdi_get_error_string(&ftdi));
		exit(1);
	}
}


static void usage(const char *name)
{
	fprintf(stderr, "usage; %s [ro|rw]\n", name);
	exit(1);
}


int main(int argc, char **argv)
{
	int set = 0, rw = 0;

	if (argc != 1 && argc != 2)
		usage(*argv);
	if (argc == 2) {
		if (!strcmp(argv[1], "ro")) {
			set = 1;
		}
		else if (!strcmp(argv[1], "rw")) {
			set = rw = 1;
		}
		else
			usage(*argv);
	}
	open_ftdi();
	if (set) {
		/* @@@FIXME: for some obscure reason, just sending the command
		   once has no effect :-( */
		norwp_set(rw);
		norwp_set(rw);
	}
	else
		norwp_query();
	return 0;
}
