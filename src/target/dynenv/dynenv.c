/*
 * dynenv - Read and write the Dynamic Environment Pointer
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
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "mtd-abi.h"


static void get_buf(int fd, uint8_t *buf)
{
	struct mtd_oob_buf oob = {
		.start	= 8,
		.length	= 8,
		.ptr	= buf,
	};

	if (ioctl(fd, MEMREADOOB, &oob) != 0) {
		perror("ioctl(MEMREADOOB)");
		exit(1);
	}

}


static int erased_q(int fd)
{
	uint8_t buf[8];

	get_buf(fd, buf);
	return !memcmp(buf, "\xff\xff\xff\xff\xff\xff\xff\xff", 8);
}


static uint32_t get_ptr(int fd)
{
	uint8_t buf[8];

	get_buf(fd, buf);
	if (memcmp(buf, "ENV0", 4)) {
		fprintf(stderr, "environment pointer marker is absent\n");
		exit(1);
	}
	return buf[4] | (buf[5] << 8) | (buf[6] << 16) | (buf[7] << 24);
}


static void set_ptr(int fd, uint32_t ptr)
{
	uint8_t buf[8] = "ENV0";
	struct mtd_oob_buf oob = {
		.start	= 8,
		.length	= 8,
		.ptr	= buf,
	};

	buf[4] = ptr;
	buf[5] = ptr >> 8;
	buf[6] = ptr >> 16;
	buf[7] = ptr >> 24;
	if (ioctl(fd, MEMWRITEOOB, &oob) != 0) {
		perror("ioctl(MEMWRITEOOB)");
		exit(1);
	}
}


static void usage(const char *name)
{
	fprintf(stderr, "usage: %s device [pointer]\n", name);
	exit(1);
}


int main(int argc, char **argv)
{
	char *end;
	int wr = 0;
	unsigned long ptr;
	int fd;

	switch (argc) {
	case 2:
		break;
	case 3:
		wr = 1;
		ptr = strtoul(argv[2], &end, 0);
		if (*end || (ptr & ~0xffffffffUL))
			usage(*argv);
		break;
	default:
		usage(*argv);
	}

	fd = open(argv[1], wr ? O_RDWR : O_RDONLY);
	if (fd < 0) {
		perror(argv[1]);
		exit(1);
	}
	if (wr) {
		uint32_t got;

		if (!erased_q(fd)) {
			got = get_ptr(fd);
			if (ptr == got)
				return 0;
/*
 * We could use the following check instead of the stricter check for either
 * identity or complete erasure (this is what u-boot does). However, it's quite
 * unlikely that anything good would come from this, so we don't.
 *
 *		if (ptr & ~got) {
 */
			fprintf(stderr, "please erase OOB block first\n");
			exit(1);
		}
		set_ptr(fd, ptr);
		got = get_ptr(fd);
		if (ptr != got) {
			fprintf(stderr, "write failed (0x%lx != 0x%lx)\n",
			    (unsigned long) got, (unsigned long) ptr);
			exit(1);

		}
	}
	else {
		printf("0x%lx\n", (unsigned long) get_ptr(fd));
		exit(1);
	}
	return 0;
}
