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
#include "gsmd-version.h"

#include <gsmd/gsmd.h>
#include <gsmd/atcmd.h>
#include <gsmd/select.h>
#include <gsmd/usock.h>
#include <gsmd/vendorplugin.h>
#include <gsmd/talloc.h>
#include <gsmd/sms.h>
#include <gsmd/unsolicited.h>

#define GSMD_ALIVECMD		"AT"
#define GSMD_ALIVE_INTERVAL	5*60
#define GSMD_ALIVE_TIMEOUT	30

static struct gsmd g;
static int daemonize = 0;

/* alive checking
 * either OK or ERROR is allowed since, both mean the modem still responds
 */

static int gsmd_alive_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd *g = ctx;

	if (!strcmp(resp, "OK") || !strcmp(resp, "ERROR") ||
	    ((g->flags & GSMD_FLAG_V0) && resp[0] == '0'))
		g->alive_responded = 1;
	return 0;
}

static void alive_tmr_cb(struct gsmd_timer *tmr, void *data)
{
	struct gsmd *g = data;

	DEBUGP("gsmd_alive timer expired\n");

	if (g->alive_responded == 0) {
		gsmd_log(GSMD_FATAL, "modem dead!\n");
		exit(3);
	} else
		gsmd_log(GSMD_INFO, "modem alive!\n");

	/* FIXME: update some global state */

	gsmd_timer_free(tmr);
}

static struct gsmd_timer * alive_timer(struct gsmd *g)
{
 	struct timeval tv;
	tv.tv_sec = GSMD_ALIVE_TIMEOUT;
	tv.tv_usec = 0;

	return gsmd_timer_create(&tv, &alive_tmr_cb, g);
}

static int gsmd_modem_alive(struct gsmd *gsmd)
{
	struct gsmd_atcmd *cmd;

	gsmd->alive_responded = 0;

	cmd = atcmd_fill(GSMD_ALIVECMD, strlen(GSMD_ALIVECMD)+1, 
			 &gsmd_alive_cb, gsmd, 0, alive_timer);
	if (!cmd) {
		return -ENOMEM;
	}

	return atcmd_submit(gsmd, cmd);
}

static void alive_interval_tmr_cb(struct gsmd_timer *tmr, void *data)
{
	struct gsmd *gsmd = data;

	DEBUGP("interval expired, starting next alive inquiry\n");

	/* start a new alive check iteration */
	gsmd_modem_alive(gsmd);

	/* re-add the timer for the next interval */
	tmr->expires.tv_sec = GSMD_ALIVE_INTERVAL;
	tmr->expires.tv_usec = 0;

	gsmd_timer_register(tmr);
}

int gsmd_alive_start(struct gsmd *gsmd)
{
	struct timeval tv;

	tv.tv_sec = GSMD_ALIVE_INTERVAL;
	tv.tv_usec = 0;

	if (!gsmd_timer_create(&tv, &alive_interval_tmr_cb, gsmd))
		return -1;

	return 0;
}


/* initial startup code */

static int gsmd_test_atcb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	DEBUGP("`%s' returned `%s'\n", cmd->buf, resp);
	return 0;
}

static int gsmd_get_imsi_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd *g = ctx;

	DEBUGP("imsi : %s\n", resp);
	strlcpy(g->imsi, resp, sizeof(g->imsi));

	return 0;
}

int gsmd_simplecmd(struct gsmd *gsmd, char *cmdtxt)
{
	struct gsmd_atcmd *cmd;
	cmd = atcmd_fill(cmdtxt, strlen(cmdtxt)+1, &gsmd_test_atcb, NULL, 0, NULL);
	if (!cmd)
		return -ENOMEM;
	
	return atcmd_submit(gsmd, cmd);
}

static int gsmd_initsettings2(struct gsmd *gsmd)
{
	int rc = 0;
	
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
	/* set it 0 to disable subscriber info and avoid cme err 512 */
	rc |= gsmd_simplecmd(gsmd, "AT+COLP=0");
	/* use +CCWA: to indicate waiting call */
	rc |= gsmd_simplecmd(gsmd, "AT+CCWA=1,1");
	/* configure message format as PDU mode*/
	/* FIXME: TEXT mode support!! */
	rc |= gsmd_simplecmd(gsmd, "AT+CMGF=0");
	/* reueset imsi */
	atcmd_submit(gsmd, atcmd_fill("AT+CIMI", 7+1,
					&gsmd_get_imsi_cb, gsmd, 0, NULL));


	sms_cb_init(gsmd);

	if (gsmd->vendorpl && gsmd->vendorpl->initsettings){
		rc |= gsmd->vendorpl->initsettings(gsmd);
		if (gsmd->machinepl && gsmd->machinepl->initsettings)
			rc |= gsmd->machinepl->initsettings(gsmd);
		return rc;
	}	
	else
		return rc;
}

static int firstcmd_response = 0;

/* we submit the first atcmd and wait synchronously for a valid response */
static int firstcmd_atcb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd *gsmd = ctx;

	if (strcmp(resp, "OK") &&
	    (!(gsmd->flags & GSMD_FLAG_V0) || resp[0] != '0')) {
		// temporarily changed to GSMD_ERROR instead of GSMD_FATAL + commented out exit(4) :M:
		gsmd_log(GSMD_ERROR, "response '%s' to initial command invalid", resp);
		//exit(4);
	}

	firstcmd_response = 1;

	if (daemonize) {
		if (fork()) {
			exit(0);
		}
		fclose(stdout);
		fclose(stderr);
		fclose(stdin);
		setsid();
	}

	return gsmd_initsettings2(gsmd);
}


int gsmd_initsettings(struct gsmd *gsmd)
{
	struct gsmd_atcmd *cmd;

	cmd = atcmd_fill("ATZ", strlen("ATZ")+1, &firstcmd_atcb, gsmd, 0, NULL);
	if (!cmd)
		return -ENOMEM;

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
	{ 230400, B230400 },
	{ 460800, B460800 },
	{ 921600, B921600 },
};

static int set_baudrate(int fd, int baudrate, int hwflow)
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
	if (i < 0) {
                return -errno;
        }
	
	i = cfsetispeed(&ti, B0);
	if (i < 0) {
                return -errno;
        }
	
	i = cfsetospeed(&ti, bd);
	if (i < 0) {
                return -errno;
        }
	
	if (hwflow)
		ti.c_cflag |= CRTSCTS;
	else
		ti.c_cflag &= ~CRTSCTS;

	return tcsetattr(fd, 0, &ti) ? -errno : 0;
}

static int gsmd_initialize(struct gsmd *g)
{
	INIT_LLIST_HEAD(&g->users);

	g->mlbuf = talloc_array(gsmd_tallocs, unsigned char, MLPARSE_BUF_SIZE);
	if (!g->mlbuf)
		return -ENOMEM;

	return 0;
}

static struct option opts[] = {
	{ "version", 0, NULL, 'V' },
	{ "daemon", 0, NULL, 'd' },
	{ "help", 0, NULL, 'h' },
	{ "device", 1, NULL, 'p' },
	{ "speed", 1, NULL, 's' },
	{ "logfile", 1, NULL, 'l' },
	{ "hwflow", 0, NULL, 'F' },
	{ "leak-report", 0, NULL, 'L' },
	{ "vendor", 1, NULL, 'v' },
	{ "machine", 1, NULL, 'm' },
	{ "wait", 1, NULL, 'w' },
	{ 0, 0, 0, 0 }
};

static void print_header(void)
{
	printf("gsmd - (C) 2006-2007 by OpenMoko, Inc. and contributors\n"
	       "This program is FREE SOFTWARE under the terms of GNU GPL\n\n");
}

static void print_version(void)
{
	printf("gsmd, version %s\n",GSMD_VERSION);
}

static void print_usage(void)
{
	printf("Usage:\n"
	       "\t-V\t--version\tDisplay program version\n"
	       "\t-d\t--daemon\tDeamonize\n"
	       "\t-h\t--help\t\tDisplay this help message\n"
	       "\t-p dev\t--device dev\tSpecify serial device to be used\n"
	       "\t-s spd\t--speed spd\tSpecify speed in bps (9600,38400,115200,...)\n"
	       "\t-F\t--hwflow\tHardware Flow Control (RTS/CTS)\n"
	       "\t-L\t--leak-report\tLeak Report of talloc memory allocator\n"
	       "\t-l file\t--logfile file\tSpecify a logfile to log to\n"
	       "\t-v\t--vendor v\tSpecify GSM modem vendor plugin\n"
	       "\t-m\t--machine m\tSpecify GSM modem machine plugin\n"
	       "\t-w\t--wait m\tWait for the AT Interpreter Ready message\n"
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
	case SIGALRM:
		gsmd_timer_check_n_run();
		break;
	}
}

int main(int argc, char **argv)
{
	int fd, argch; 

	int bps = 115200;
	int hwflow = 0;
	char *device = NULL;
	char *vendor_name = NULL;
	char *machine_name = NULL;
	int wait = -1;

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGUSR1, sig_handler);
	signal(SIGALRM, sig_handler);
	
	gsmd_tallocs = talloc_named_const(NULL, 1, "GSMD");

	print_header();

	/*FIXME: parse commandline, set daemonize, device, ... */
	while ((argch = getopt_long(argc, argv, "FVLdhp:s:l:v:m:w:", opts, NULL)) != -1) {
		switch (argch) {
		case 'V':
			print_version();
			exit(0);
			break;
		case 'L':
			talloc_enable_leak_report_full();
			break;
		case 'F':
			hwflow = 1;
			break;
		case 'd':
			daemonize = 1;
			break;
		case 'h':
			/* FIXME */
			print_usage();
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
		case 'v':
			vendor_name = optarg;
			break;
		case 'm':
			machine_name = optarg;
			break;
		case 'w':
			wait = atoi(optarg);
			break;
		}
	}

	if (!device) {
		fprintf(stderr, "ERROR: you have to specify a port (-p port)\n");
		print_usage();
		exit(2);
	}

	/* use direct access to device node ([virtual] tty device) */
	fd = open(device, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "can't open device `%s': %s\n",
			device, strerror(errno));
		exit(1);
	}

	if (set_baudrate(fd, bps, hwflow) < 0) {
		fprintf(stderr, "can't set baudrate\n");
		exit(1);
	}

	if (gsmd_initialize(&g) < 0) {
		fprintf(stderr, "internal error\n");
		exit(1);
	}

	gsmd_timer_init();

	if (gsmd_machine_plugin_init(&g, machine_name, vendor_name) < 0) {
		fprintf(stderr, "no machine plugins found\n");
		exit(1);
	}

	/* select a machine plugin and load possible vendor plugins */
	gsmd_machine_plugin_find(&g);

	/* initialize the machine plugin */
	if (g.machinepl->init &&
	    (g.machinepl->init(&g, fd) < 0)) {
		fprintf(stderr, "couldn't initialize machine plugin\n");
		exit(1);
	}

	if (wait >= 0)
		g.interpreter_ready = !wait;

	if (atcmd_init(&g, fd) < 0) {
		fprintf(stderr, "can't initialize UART device\n");
		exit(1);
	}

        write(fd,"\r",1);
	atcmd_drain(fd);

	if (usock_init(&g) < 0) {
		fprintf(stderr, "can't open unix socket\n");
		exit(1);
	}

	/* select a vendor plugin */
	gsmd_vendor_plugin_find(&g);

	unsolicited_init(&g);

	if (g.interpreter_ready) {
		gsmd_initsettings(&g);
	
		gsmd_alive_start(&g);
	}

	gsmd_opname_init(&g);

	while (1) {
		int ret = gsmd_select_main();
		if (ret == 0)
			continue;

		if (ret < 0) {
			if (errno == EINTR)
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
