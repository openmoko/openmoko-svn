#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define _GNU_SOURCE
#include <getopt.h>

#include <libgsmd/libgsmd.h>

static struct lgsm_handle *lgsmh;
static int verbose = 0;

static struct option opts[] = {
	{ "help", 0, 0, 'h' },
	{ "version", 0, 0, 'V' },
	{ "verbose", 0, 0, 'v' },
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
	int rc, i;

	printf("libgsm-tool - (C) 2006 by Harald Welte\n"
		"This program is Free Software and has ABSOLUTELY NO WARRANTY\n\n");

	while (1) {
		int c, option_index = 0;
		c = getopt_long(argc, argv, "hVv", opts, &option_index);
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
		}
	}

	lgsmh = lgsm_init(LGSMD_DEVICE_GSMD);
	if (!lgsmh) {
		fprintf(stderr, "Can't connect to gsmd\n");
		exit(1);
	}


	exit(0);
}
