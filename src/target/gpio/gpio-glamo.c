/*
 * gpio-glamo - Read and set GPIO pins (of the Smedia Glamo 3362)
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
#include <fcntl.h>
#include <sys/mman.h>


#define BASE 0x08000000


static volatile void *mem;


#define CON_IN	0
#define CON_OUT	1
#define CON_F1	2
#define CON_F2	3


/* 2442 port assignment */

static struct port {
	const char *name;
	int offset;
	int start;
	enum { pt_gpio, pt_dgpio } type;
} ports[] = {
	{ "GPIO",  0x50,  0, pt_gpio },
	{ "GPIO",  0x52,  4, pt_gpio },
	{ "GPIO",  0x54,  8, pt_gpio },
	{ "GPIO",  0x56, 12, pt_gpio },
	{ "GPIO",  0x58, 16, pt_gpio },
	{ "DGPIO", 0x5c,  0, pt_dgpio },
	{ "DGPIO", 0x5e,  4, pt_dgpio },
	{ "DGPIO", 0x60,  8, pt_dgpio },
	{ "DGPIO", 0x62, 12, pt_dgpio },
	{ NULL, }
};


static void print_n(const char *name, int start)
{
	int i;

	printf("%s ", name);
	for (i = 0; i != 4; i++)
		printf("%2d ", start+i);
	putchar('\n');
}


/* ----- Read a pin -------------------------------------------------------- */


static const char *pin(const struct port *p, int n)
{
	static char res[4];
	uint16_t reg;

	reg = *(uint16_t *) (mem+p->offset);
	if (p->type == pt_dgpio || ((reg >> (n+12)) & 1))
		res[0] = res[0] = (reg >> n) & 1 ? ' ' : '>';
	else
		res[0] = 'F';
	res[1] = (reg >> (n+8)) & 1 ? '1' : '0';
	if (res[0] == ' ')
		res[2] = ' ';
	else
		res[2] = ((reg >> (n+4)) ^ (reg >> (n+8))) & 1 ? '!' : ' ';
	return res;
}


/* ----- Dump all ports ---------------------------------------------------- */


static void dump_all(void)
{
	const struct port *p;
	int i;

	for (p = ports; p->name; p++) {
		print_n(p->name, p->start);
		printf("%*s ", strlen(p->name), "");
		for (i = 0; i != 4; i++)
			printf("%s ", pin(p, i));
		putchar('\n');
	}
}


/* ----- Command-line parsing ---------------------------------------------- */


static void __attribute__((noreturn)) usage(const char *name)
{
	fprintf(stderr,
"usage: %s [pin[=value] ...]\n\n"
"  pin = <letter><number>, e.g., A5, b10\n"
"  value = <control><pull-down>, with\n"
"    <control> = 0 (output 0), 1 (output 1), Z (input), F (primary\n"
"                function), X (secondary function)\n"
"    <pull-down> = R (pull-down active) or nothing\n"
"    As a short-cut, ZR can be abbreviated as R.\n\n"
"  Examples: A5=ZR, b10=0\n",
    name);
	exit(1);
}


int main(int argc, char **argv)
{
	int fd;

	fd = open("/dev/mem", O_RDWR);
        if (fd < 0) {
		perror("/dev/mem");
		exit(1);
	}
	mem = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BASE);
	if (mem == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	if (argc == 1)
		dump_all();
	else
		usage(*argv);
	return 0;
}
