/* gsmd core
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
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>

#define _GNU_SOURCE
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/atcmd.h>
#include <gsmd/select.h>
#include <gsmd/usock.h>
#include <gsmd/vendorplugin.h>
#include <gsmd/talloc.h>

static int gsmd_test_atcb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	printf("`%s' returned `%s'\n", cmd->buf, resp);
	return 0;
}

int gsmd_simplecmd(struct gsmd *gsmd, char *cmdtxt)
{
	struct gsmd_atcmd *cmd;
	cmd = atcmd_fill(cmdtxt, strlen(cmdtxt)+1, &gsmd_test_atcb, NULL, 0);
	if (!cmd)
		return -ENOMEM;
	
	return atcmd_submit(gsmd, cmd);
}

int gsmd_initsettings(struct gsmd *gsmd)
{
	int rc;
	
	/* echo on, verbose */
	rc |= gsmd_simplecmd(gsmd, "ATE0V1");
	/* use +CRING instead of RING */
	rc |= gsmd_simplecmd(gsmd, "AT+CRC=1");
	/* enable +CREG: unsolicited response if registration status changes */
	rc |= gsmd_simplecmd(gsmd, "AT+CREG=2");
	/* use +CME ERROR: instead of ERROR */
	rc |= gsmd_simplecmd(gsmd, "AT+CMEE=1");
	/* use +CLIP: to indicate CLIP */
	rc |= gsmd_simplecmd(gsmd, "AT+CLIP=1");
	/* use +COLP: to indicate COLP */
	rc |= gsmd_simplecmd(gsmd, "AT+COLP=1");
	/* use +CTZR: to report time zone changes */
	rc |= gsmd_simplecmd(gsmd, "AT+CTZR=1");
	/* power on the phone */
	rc |= gsmd_simplecmd(gsmd, "AT+CFUN=1");

	if (gsmd->vendorpl && gsmd->vendorpl->initsettings)
		return gsmd->vendorpl->initsettings(gsmd);
	else
		return rc;
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
	{ "leak-report", 0, NULL, 'L' },
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

static void sig_handler(int signr)
{
	switch (signr) {
	case SIGTERM:
	case SIGINT:
		talloc_report_full(gsmd_tallocs, stderr);
		exit(0);
		break;
	case SIGUSR1:
		talloc_report_full(gsmd_tallocs, stderr);
		break;
	}
}

int main(int argc, char **argv)
{
	int fd, argch; 

	int daemonize = 0;
	int bps = 115200;
	char *device = "/dev/ttyUSB0";
	char *logfile = "syslog";

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGSEGV, sig_handler);
	signal(SIGUSR1, sig_handler);
	
	gsmd_tallocs = talloc_named_const(NULL, 1, "GSMD");

	/*FIXME: parse commandline, set daemonize, device, ... */
	while ((argch = getopt_long(argc, argv, "VLdhp:s:l:", opts, NULL)) != -1) {
		switch (argch) {
		case 'V':
			/* FIXME */
			break;
		case 'L':
			talloc_enable_leak_report_full();
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

	/* FIXME: do this dynamically */
	ticalypso_init();

	gsmd_vendor_plugin_find(&g);

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
