/*
 * gpio-s3c6410.c - Read and set GPIO pins (of the Samsung S3C6410 MCU)
 *
 * Copyright (C) 2008, 2009 by OpenMoko, Inc.
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


#define BASE 0x7f008000


static volatile void *mem;


#define CON_IN	0
#define CON_OUT	1
#define CON_F1	2
#define CON_F2	3
#define CON_F3	4
#define CON_F4	5
#define CON_F5	6
#define CON_INT	7

#define	R_Z	0
#define	R_DOWN	1
#define	R_UP	2


/* 6410 port assignment */

static struct port {
	const char *name;
	int offset;
	int last;
	enum { pt_a, pt_f, pt_h } type;
} ports[] = {
	{ "A", 0x000,  8, pt_a },
	{ "B", 0x020,  7, pt_a },
	{ "C", 0x040,  8, pt_a },
	{ "D", 0x060,  5, pt_a },
	{ "E", 0x080,  5, pt_a },
	{ "F", 0x0a0, 16, pt_f },
	{ "G", 0x0c0,  7, pt_a },
	{ "H", 0x0e0, 10, pt_h },
	{ "I", 0x100, 16, pt_f },
	{ "J", 0x120, 12, pt_f },
	{ "K", 0x800, 16, pt_h },
	{ "L", 0x810, 15, pt_h },
	{ "M", 0x820,  6, pt_a },
	{ "N", 0x830, 16, pt_f },
	{ "O", 0x140, 16, pt_f },
	{ "P", 0x160, 15, pt_f },
	{ "Q", 0x180,  9, pt_f },
	{ NULL, }
};


static void print_n(const char *name, int last)
{
	int i;

	printf("%s ", name);
	for (i = 0; i <= last; i++)
		printf("%2d ", i);
	putchar('\n');
}


/* ----- Read a pin -------------------------------------------------------- */


static const char *pin_a(int con, int dat, int pud)
{
	static char res[4];

	res[0] = " >ABCDEIXXXXXXXX"[con];
	res[1] = dat ? '1' : '0';
	res[2] = " DUX"[pud];
	return res;
}


static const char *pin_f(int con, int dat, int pud)
{
	static char res[4];

	res[0] = " >FI"[con];
	res[1] = dat ? '1' : '0';
	res[2] = " DUX"[pud];
	return res;
}


static const char *pin(const struct port *p, int num)
{
	uint32_t con, con1, dat, pud;

	con = *(uint32_t *) (mem+p->offset);
	if (p->type != pt_h) {
		dat = *(uint32_t *) (mem+p->offset+4);
		pud = *(uint32_t *) (mem+p->offset+8);
	} else {
		con1 = *(uint32_t *) (mem+p->offset+4);
		dat = *(uint32_t *) (mem+p->offset+8);
		pud = *(uint32_t *) (mem+p->offset+12);
	}

	switch (p->type) {
	case pt_a:
		return pin_a(
		    (con >> (num*4)) & 15,
		    (dat >> num) & 1,
		    (pud >> (num*2)) & 3);
	case pt_f:
		return pin_f(
		    (con >> (num*2)) & 3,
		    (dat >> num) & 1,
		    (pud >> (num*2)) & 3);
	case pt_h:
		return pin_a(
		    num < 8 ? (con >> (num*4)) & 15 : (con1 >> (num*4-32)) & 15,
		    (dat >> num) & 1,
		    (pud >> (num*2)) & 3);
	default:
		abort();
	}
}


/* ----- Set a pin --------------------------------------------------------- */


static void set_a(const struct port *p, int num, int c, int d, int r)
{
	uint32_t con, dat, pud;

	con = *(uint32_t *) (mem+p->offset);
	con = (con & ~(15 << (num*4))) | (c << (num*4));
	*(uint32_t *) (mem+p->offset) = con;
	
	if (d != -1) {
		dat = *(uint32_t *) (mem+p->offset+4);
		dat = (dat & ~(1 << num)) | (d << num);
		*(uint32_t *) (mem+p->offset+4) = dat;
	}

	pud = *(uint32_t *) (mem+p->offset+8);
	pud = (pud & ~(15 << (num*2))) | (r << (num*2));
	*(uint32_t *) (mem+p->offset+8) = pud;
}


static void set_f(const struct port *p, int num, int c, int d, int r)
{
	uint32_t con, dat, pud;

	con = *(uint32_t *) (mem+p->offset);
	con = (con & ~(3 << (num*2))) | (c << (num*2));
	*(uint32_t *) (mem+p->offset) = con;
	
	if (d != -1) {
		dat = *(uint32_t *) (mem+p->offset+4);
		dat = (dat & ~(1 << num)) | (d << num);
		*(uint32_t *) (mem+p->offset+4) = dat;
	}

	pud = *(uint32_t *) (mem+p->offset+8);
	pud = (pud & ~(3 << (num*2))) | (r << (num*2));
	*(uint32_t *) (mem+p->offset+8) = pud;
}


static void set_h(const struct port *p, int num, int c, int d, int r)
{
	uint32_t con, dat, pud;

	if (num < 8) {
		con = *(uint32_t *) (mem+p->offset);
		con = (con & ~(15 << (num*4))) | (c << (num*4));
		*(uint32_t *) (mem+p->offset) = con;
	} else {
		con = *(uint32_t *) (mem+p->offset+4);
		con = (con & ~(15 << (num*4-32))) | (c << (num*4-32));
		*(uint32_t *) (mem+p->offset+4) = con;
	}
	
	if (d != -1) {
		dat = *(uint32_t *) (mem+p->offset+8);
		dat = (dat & ~(1 << num)) | (d << num);
		*(uint32_t *) (mem+p->offset+8) = dat;
	}

	pud = *(uint32_t *) (mem+p->offset+12);
	pud = (pud & ~(15 << (num*4))) | (r << (num*4));
	*(uint32_t *) (mem+p->offset+12) = pud;
}


static void set_pin(const struct port *p, int num, int c, int d, int r)
{
	if (num > p->last) {
		fprintf(stderr, "invalid pin %s%d\n", p->name, num);
		exit(1);
	}
	switch (p->type) {
	case pt_a:
		set_a(p, num, c, d, r);
		break;
	case pt_f:
		if (c > 3) {
			fprintf(stderr, "pin %s%d is F-type\n", p->name, num);
			exit(1);
		}
		set_f(p, num, c, d, r);
		break;
	case pt_h:
		set_h(p, num, c, d, r);
		break;
	default:
		abort();
	}
}


/* ----- Dump all ports ---------------------------------------------------- */


static void dump_all(void)
{
fprintf(stderr, "not yet implemented\n");
exit(1);
#if 0
	const struct port *p;
	uint32_t con, dat, pud;
	int i;

	for (p = ports; p->name; p++) {
		con = *(uint32_t *) (mem+p->offset);
		dat = *(uint32_t *) (mem+p->offset+4);
		if (p->type == pt_a) {
			print_n(p->name, p->last);
			printf("%*s ", strlen(p->name), "");
			for (i = 0; i <= p->last; i++)
				printf("%s ",
				    pin_a((con >> i) & 1, (dat >> i) & 1));
			putchar('\n');
		}
		else {
			pud = *(uint32_t *) (mem+p->offset+8);
			print_n(p->name, p->last);
			printf("%*s ", strlen(p->name), "");
			for (i = 0; i <= p->last; i++)
				printf("%s",
				    pin_a((con >> (i*2)) & 3, (dat >> i) & 1,
				    (pud >> i) & 1));
			putchar('\n');
		}
	}
#endif
}


/* ----- Command-line parsing ---------------------------------------------- */


static void __attribute__((noreturn)) usage(const char *name)
{
	fprintf(stderr,
"usage: %s [pin[=value] ...]\n\n"
"  pin = <letter><number>, e.g., A5, b10\n"
"  value = <control><pull-down>, with\n"
"    <control> = 0 (output 0), 1 (output 1), Z (input), A or F (primary\n"
"                function), B, C, ... E (additional function), I (interrupt)\n"
"    <pull-down> = D (pull-down), U (pull-up), or nothing\n\n"
"  Examples: A5=ZR, b10=0\n",
    name);
	exit(1);
}


static void port_op(const char *name, const char *op)
{
	const char *eq, *s;
	const struct port *p;
	int num, c, d = -1, r = R_Z;
	char *end;

	eq = strchr(op, '=');
	if (!eq)
		eq = strchr(op, 0);
	num = strcspn(op, "0123456789");
	if (!num || op+num >= eq)
		usage(name);
	for (p = ports; p->name; p++)
		if (strlen(p->name) == num && !strncasecmp(p->name, op, num))
			break;
	if (!p->name) {
		fprintf(stderr, "invalid port \"%.*s\"\n", num, op);
		exit(1);
	}
	num = strtoul(op+num, &end, 10);
	if (end != eq)
		usage(name);
	if (!*eq) {
		s = pin(p, num);
		if (*s == ' ')
			s++;
		printf("%s\n", s);
		return;
	}
	switch (eq[1]) {
	case '0':
		d = 0;
		c = CON_OUT;
		break;
	case '1':
		d = 1;
		c = CON_OUT;
		break;
	case 'Z':
	case 'z':
		c = CON_IN;
		break;
	case 'F':
	case 'f':
	case 'A':
	case 'a':
		c = CON_F1;
		break;
	case 'B':
	case 'b':
		c = CON_F2;
		break;
	case 'C':
	case 'c':
		c = CON_F3;
		break;
	case 'D':
	case 'd':
		c = CON_F4;
		break;
	case 'E':
	case 'e':
		c = CON_F5;
		break;
	case 'I':
	case 'i':
		c = CON_INT;
		break;
	default:
		usage(name);
	}
	if (eq[2]) {
		switch (eq[2]) {
		case 'D':
		case 'd':
			r = R_DOWN;
			break;
		case 'U':
		case 'u':
			r = R_UP;
			break;
		default:
			usage(name);
		}
		if (eq[3])
			usage(name);
	}
	set_pin(p, num, c, d, r);
}


int main(int argc, char **argv)
{
	int fd, i;

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
		for (i = 1; i != argc; i++)
			port_op(*argv, argv[i]);
	return 0;
}
