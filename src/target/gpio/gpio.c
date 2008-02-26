/*
 * gpio - Read and set GPIO pins (of the Samsung S3C2442 MCU)
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


#define BASE 0x56000000


static volatile void *mem;


#define CON_IN	0
#define CON_OUT	1
#define CON_F1	2
#define CON_F2	3


/* 2442 port assignment */

static struct port {
	const char *name;
	int offset;
	int last;
	enum { pt_a, pt_b } type;
} ports[] = {
	{ "A", 0x00, 22, pt_a },
	{ "B", 0x10, 10, pt_b },
	{ "C", 0x20, 15, pt_b },
	{ "D", 0x30, 15, pt_b },
	{ "E", 0x40, 15, pt_b },
	{ "F", 0x50,  7, pt_b },
	{ "G", 0x60, 15, pt_b },
	{ "H", 0x70, 10, pt_b },
	{ "J", 0xd0, 12, pt_b },
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


static const char *pin_a(int con, int dat)
{
	if (con)
		return dat ? "F1" : "F0";
	else
		return dat ? ">1" : ">0";
}


static const char *pin_b(int con, int dat, int pud)
{
	static char res[4];

	res[0] = " >FX"[con];
	res[1] = dat ? '1' : '0';
	res[2] = pud ? ' ' : 'R';
	return res;
}


static const char *pin(const struct port *p, int num)
{
	uint32_t con, dat, pud;

	con = *(uint32_t *) (mem+p->offset);
	dat = *(uint32_t *) (mem+p->offset+4);
	if (p->type == pt_a)
		return pin_a((con >> num) & 1, (dat >> num) & 1);
	else {
		pud = *(uint32_t *) (mem+p->offset+8);
		return pin_b((con >> (num*2)) & 3, (dat >> num) & 1,
		    (pud >> num) & 1);
	}
}


/* ----- Set a pin --------------------------------------------------------- */


static void set_a(const struct port *p, int num, int c, int d)
{
	uint32_t con, dat;

	con = *(uint32_t *) (mem+p->offset);
	con = (con & ~(1 << num)) | ((c == CON_F1) << num);
	*(uint32_t *) (mem+p->offset) = con;
	
	if (d != -1) {
		dat = *(uint32_t *) (mem+p->offset+4);
		dat = (dat & ~(1 << num)) | (d << num);
		*(uint32_t *) (mem+p->offset+4) = dat;
	}
}


static void set_b(const struct port *p, int num, int c, int d, int r)
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
	pud = (pud & ~(1 << num)) | (!r << num);
	*(uint32_t *) (mem+p->offset+8) = pud;
}


static void set_pin(const struct port *p, int num, int c, int d, int r)
{
	if (num > p->last) {
		fprintf(stderr, "invalid pin %s%d\n", p->name, num);
		exit(1);
	}
	if (p->type == pt_a) {
		if (r) {
			fprintf(stderr, "pin %s%d has no pull-down\n",
			    p->name, num);
			exit(1);
		}
		if (c == CON_IN) {
			fprintf(stderr, "pin %s%d cannot be an input\n",
			    p->name, num);
			exit(1);
		}
		if (c == CON_F2) {
			fprintf(stderr, "pin %s%d has no second function\n",
			    p->name, num);
			exit(1);
		}
		set_a(p, num, c, d);
	}
	else
		set_b(p, num, c, d, r);
}


/* ----- Dump all ports ---------------------------------------------------- */


static void dump_all(void)
{
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
				    pin_b((con >> (i*2)) & 3, (dat >> i) & 1,
				    (pud >> i) & 1));
			putchar('\n');
		}
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


static void port_op(const char *name, const char *op)
{
	const char *eq, *s;
	const struct port *p;
	int num, c, d = -1, r = 0;
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
	case 'R':
	case 'r':
		if (eq[2])
			usage(name);
		c = CON_IN;
		r = 1;
		break;
	case 'F':
	case 'f':
		c = CON_F1;
		break;
	case 'X':
	case 'x':
		c = CON_F2;
		break;
	default:
		usage(name);
	}
	if (eq[2]) {
		if (eq[2] != 'R' && eq[2] != 'r')
			usage(name);
		if (eq[3])
			usage(name);
		r = 1;
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
