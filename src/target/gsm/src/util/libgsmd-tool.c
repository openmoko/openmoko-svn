/* libgsmd tool
 *
 * (C) 2006-2007 by OpenMoko, Inc.
 * Written by Harald Welte <laforge@openmoko.org>
 * All Rights Reserved
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define _GNU_SOURCE
#include <getopt.h>

#include <libgsmd/libgsmd.h>

#include "pin.h"
#include "event.h"
#include "shell.h"
#include "atcmd.h"

#include "../gsmd/gsmd-version.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static struct lgsm_handle *lgsmh;
static int verbose = 0;

enum mode_enum {
	MODE_NONE,
	MODE_SHELL,
	MODE_EVENTLOG,
	MODE_ATCMD,
};

static char *modes[] = {
	[MODE_NONE]	= "",
	[MODE_SHELL]	= "shell",
	[MODE_EVENTLOG]	= "eventlog",
	[MODE_ATCMD]	= "atcmd",
};

static int parse_mode(char *modestr)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(modes); i++) {
		if (!strcmp(modes[i], modestr))
			return i;
	}

	return -1;
}

static struct option opts[] = {
	{ "help", 0, 0, 'h' },
	{ "version", 0, 0, 'V' },
	{ "verbose", 0, 0, 'v' },
	{ "mode", 1, 0, 'm' },
	{ "pin", 1, 0, 'p' },
	{ "wait", 0, 0, 'w' },
	{ 0, 0, 0, 0 }
};

static void help(void)
{
	printf("Usage:\n"
		"\t-h\t--help\tPrint this Help message\n"
		"\t-V\t--version\tPrint version number\n"
		"\t-v\t--verbose\tBe more verbose\n"
		"\t-m\t--mode\tSet mode {shell,eventlog,atcmd}\n"
		"\t-w\t--wait\tIn shell mode wait for responses on exit\n"
		);
}

static void dump_version(void)
{
	printf("Version: " GSMD_VERSION "\n");
}

int main(int argc, char **argv)
{
	char *pin = NULL;
	int mode = MODE_NONE, shellwait = 0;

	printf("libgsm-tool - (C) 2006-2007 by Harald Welte and OpenMoko, Inc.\n"
		"This program is Free Software and has ABSOLUTELY NO WARRANTY\n\n");

	while (1) {
		int c, option_index = 0;
		c = getopt_long(argc, argv, "vVhwm:p:", opts, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'v':
			verbose = 1;
			break;
		case 'V':
			dump_version();
			exit(0);
			break;
		case 'h':
			help();
			exit(0);
			break;
		case 'm':
			mode = parse_mode(optarg);
			if (mode < 0) {
				fprintf(stderr, "unknown/unsupported mode `%s'\n", optarg);
				exit(2);
			}
			break;
		case 'p':
			pin = optarg;
			break;
		case 'w':
			shellwait = 1;
			break;
		}
	}

	lgsmh = lgsm_init(LGSMD_DEVICE_GSMD);
	if (!lgsmh) {
		fprintf(stderr, "Can't connect to gsmd\n");
		exit(1);
	}

	pin_init(lgsmh, pin);
	event_init(lgsmh);

	switch (mode) {
	case MODE_ATCMD:
		atcmd_main(lgsmh);
		break;
	case MODE_SHELL:
		shell_main(lgsmh, shellwait);
		break;
	}

	exit(0);
}
