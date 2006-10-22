#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#define _GNU_SOURCE
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <gsmd/gsmd.h>

#include "gsmd.h"
#include "atcmd.h"
#include "select.h"
#include "usock.h"
#include "vendorplugin.h"

static int gsmd_test_atcb(struct gsmd_atcmd *cmd, void *ctx)
{
	printf("`%s' returned `%s'\n", cmd->buf, cmd->resp);
	free(cmd);
	return 0;
}

static int gsmd_test(struct gsmd *gsmd)
{
	struct gsmd_atcmd *cmd;
	cmd = atcmd_fill("AT+CRC?", 255, &gsmd_test_atcb, NULL);
	return atcmd_submit(gsmd, cmd);
}

static int atcmd_test(struct gsmd *gsmd)
{
	struct gsmd_atcmd *cmd;
	cmd = atcmd_fill("ATE0", 255, &gsmd_test_atcb, NULL);
	return atcmd_submit(gsmd, cmd);
}

static int gsmd_initsettings(struct gsmd *gsmd)
{
	int rc;
	struct gsmd_atcmd *cmd;

	cmd = atcmd_fill("ATV1", 255, &gsmd_test_atcb, NULL);
	rc = atcmd_submit(gsmd, cmd);
	if (rc < 0)
		return rc;

	cmd = atcmd_fill("+CRC=1;+CREG=2;+CMEE=2;+CLIP=1;+COLP=1;+CTZR=1", 255, &gsmd_test_atcb, NULL);
	return atcmd_submit(gsmd, cmd);
}

struct bdrt {
	int bps;
	u_int32_t b;
};

static struct bdrt bdrts[] = {
	{ 0, B0 },
	{ 9600, B9600 },
	{ 19200, B19200 },
	{ 38400, B38400 },
	{ 57600, B57600 },
	{ 115200, B115200 },
};

static int set_baudrate(int fd, int baudrate)
{
	int i;
	u_int32_t bd = 0;
	struct termios ti;

	for (i = 0; i < ARRAY_SIZE(bdrts); i++) {
		if (bdrts[i].bps == baudrate)
			bd = bdrts[i].b;
	}
	if (bd == 0)
		return -EINVAL;
	
	i = tcgetattr(fd, &ti);
	if (i < 0)
		return i;
	
	i = cfsetispeed(&ti, B0);
	if (i < 0)
		return i;
	
	i = cfsetospeed(&ti, bd);
	if (i < 0)
		return i;
	
	return tcsetattr(fd, 0, &ti);
}


static struct gsmd g;

static int gsmd_initialize(struct gsmd *g)
{
	INIT_LLIST_HEAD(&g->users);

	return 0;
}

static struct option opts[] = {
	{ "version", 0, NULL, 'V' },
	{ "daemon", 0, NULL, 'd' },
	{ "help", 0, NULL, 'h' },
	{ "device", 1, NULL, 'p' },
	{ "speed", 1, NULL, 's' },
	{ "logfile", 1, NULL, 'l' },
	{ 0, 0, 0, 0 }
};

static void print_help(void)
{
	printf("gsmd - (C) 2006 by Harald Welte <laforge@gnumonks.org>\n"
	       "This program is FREE SOFTWARE under the terms of GNU GPL\n\n"
	       "Usage:\n"
	       "\t-v\t--version\tDisplay program version\n"
	       "\t-d\t--daemon\tDeamonize\n"
	       "\t-h\t--help\t\tDisplay this help message\n"
	       "\t-p dev\t--device dev\tSpecify serial device to be used\n"
	       "\t-s spd\t--speed spd\tSpecify speed in bps (9600,38400,115200,...)\n"
	       "\t-l file\t--logfile file\tSpecify a logfile to log to\n"
	       );
}

int main(int argc, char **argv)
{
	int fd, argch; 

	int daemonize = 0;
	int bps = 115200;
	char *device = "/dev/ttyUSB0";
	char *logfile = "syslog";

	/*FIXME: parse commandline, set daemonize, device, ... */
	while ((argch = getopt_long(argc, argv, "Vdhp:s:l:", opts, NULL)) != -1) {
		switch (argch) {
		case 'V':
			/* FIXME */
			break;
		case 'd':
			daemonize = 1;
			break;
		case 'h':
			/* FIXME */
			print_help();
			exit(0);
			break;
		case 'p':
			device = optarg;
			break;
		case 's':
			bps = atoi(optarg);
			break;
		case 'l':
			if (gsmdlog_init(optarg)) {
				fprintf(stderr, "can't open logfile `%s'\n", optarg);
				exit(2);
			}
			break;
		}
	}

	/* use direct access to device node ([virtual] tty device) */
	fd = open(device, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "can't open device `%s': %s\n",
			device, strerror(errno));
		exit(1);
	}

	if (set_baudrate(fd, bps) < 0) {
		fprintf(stderr, "can't set baudrate\n");
		exit(1);
	}

	if (gsmd_initialize(&g) < 0) {
		fprintf(stderr, "internal error\n");
		exit(1);
	}

	if (atcmd_init(&g, fd) < 0) {
		fprintf(stderr, "can't initialize UART device\n");
		exit(1);
	}
	atcmd_drain(fd);

	if (usock_init(&g) < 0) {
		fprintf(stderr, "can't open unix socket\n");
		exit(1);
	}

	if (daemonize) {
		if (fork()) {
			exit(0);
		}
		fclose(stdout);
		fclose(stderr);
		fclose(stdin);
		setsid();
	}

	atcmd_test(&g);
	gsmd_test(&g);
	gsmd_initsettings(&g);

	while (1) {
		int ret = gsmd_select_main();
		if (ret == 0)
			continue;

		if (ret < 0) {
			if (errno == -EINTR)
				continue;
			else {
				DEBUGP("select returned error (%s)\n",
					strerror(errno));
				break;
			}
		}
	}

	exit(0);
}
