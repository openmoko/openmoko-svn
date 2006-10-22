#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define _GNU_SOURCE
#include <getopt.h>

#include <libgsmd/libgsmd.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static struct lgsm_handle *lgsmh;
static int verbose = 0;

enum mode_enum {
	MODE_NONE,
	MODE_SHELL,
	MODE_EVENTLOG,
};

static char *modes[] = {
	[MODE_NONE]	= "",
	[MODE_SHELL]	= "shell",
	[MODE_EVENTLOG]	= "eventlog",
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
	{ 0, 0, 0, 0 }
};

static void help(void)
{
	printf("Usage:\n"
		"\t-h\t--help\tPrint this Help message\n"
		"\t-V\t--version\tPrint version number\n"
		"\t-v\t--verbose\tBe more verbose\n");
}

int main(int argc, char **argv)
{
	int rc, i, mode;

	printf("libgsm-tool - (C) 2006 by Harald Welte\n"
		"This program is Free Software and has ABSOLUTELY NO WARRANTY\n\n");

	while (1) {
		int c, option_index = 0;
		c = getopt_long(argc, argv, "vVhm:", opts, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'v':
			verbose = 1;
			break;
		case 'V':
			/* FIXME */
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
		}
	}

	lgsmh = lgsm_init(LGSMD_DEVICE_GSMD);
	if (!lgsmh) {
		fprintf(stderr, "Can't connect to gsmd\n");
		exit(1);
	}

	switch (mode) {
	case MODE_SHELL:
		shell_main(lgsmh);
		break;
	}

	exit(0);
}
