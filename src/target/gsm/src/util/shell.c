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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/voicecall.h>
#include <libgsmd/misc.h>

#define STDIN_BUF_SIZE	1024

/* this is the handler for receiving passthrough responses */
static int pt_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	char *payload = (char *)gmh + sizeof(*gmh);
	printf("RSTR=`%s'\n", payload);
}

static int shell_help(void)
{
	printf( "\tA\tAnswer incoming call\n"
		"\tD\tDial outgoing number\n"
		"\tH\tHangup call\n"
		"\tO\tPower On\n"
		"\to\tPower Off\n"
		"\tR\tRegister Netowrk\n"
		"\tT\tSend DTMF Tone\n"
		"\tq\tQuit\n"
		);
}

int shell_main(struct lgsm_handle *lgsmh)
{
	int rc;
	char buf[STDIN_BUF_SIZE+1];
	char rbuf[STDIN_BUF_SIZE+1];
	int rlen = sizeof(rbuf);
	fd_set readset;

	lgsm_register_handler(lgsmh, GSMD_MSG_PASSTHROUGH, &pt_msghandler);

	fcntl(0, F_SETFD, O_NONBLOCK);
	fcntl(lgsm_fd(lgsmh), F_SETFD, O_NONBLOCK);

	FD_ZERO(&readset);

	printf("# ");

	while (1) {
		fd_set readset;
		int gsm_fd = lgsm_fd(lgsmh);
		FD_SET(0, &readset);
		FD_SET(gsm_fd, &readset);

		rc = select(gsm_fd+1, &readset, NULL, NULL, NULL);
		if (rc <= 0)	
			break;
		if (FD_ISSET(gsm_fd, &readset)) {
			/* we've received something on the gsmd socket, pass it
			 * on to the library */
			rc = read(gsm_fd, buf, sizeof(buf));
			if (rc <= 0) {
				printf("ERROR reding from gsm_fd\n");
				break;
			}
			rc = lgsm_handle_packet(lgsmh, buf, rc);
		}
		if (FD_ISSET(0, &readset)) {
			/* we've received something on stdin.  */
			printf("# ");
			rc = fscanf(stdin, "%s", buf);
			if (rc == EOF) {
				printf("EOF\n");
				return -1;
			}
			if (rc <= 0) {
				printf("NULL\n");
				continue;
			}
			if (!strcmp(buf, "h")) {
				shell_help();
			} else if (!strcmp(buf, "?")) {
				shell_help();
			} else if (!strcmp(buf, "H")) {
				printf("Hangup\n");
				lgsm_voice_hangup(lgsmh);
			} else if (buf[0] == 'D') {
				struct lgsm_addr addr;
				if (strlen(buf) < 2)
					continue;
				printf("Dial %s\n", buf+1);
				addr.type = 129;
				strncpy(addr.addr, buf+1, sizeof(addr.addr)-1);
				addr.addr[sizeof(addr.addr)-1] = '\0';
				lgsm_voice_out_init(lgsmh, &addr);
			} else if (!strcmp(buf, "A")) {
				printf("Answer\n");
				lgsm_voice_in_accept(lgsmh);
			} else if (!strcmp(buf, "O")) {
				printf("Power-On\n");
				lgsm_phone_power(lgsmh, 1);
			} else if (!strcmp(buf, "o")) {
				printf("Power-Off\n");
				lgsm_phone_power(lgsmh, 0);
			} else if (!strcmp(buf, "R")) {
				printf("Register\n");
				lgsm_netreg_register(lgsmh, 0);
			} else if (!strcmp(buf, "q")) {
				exit(0);
			} else if (buf[0] == 'T') {
				if (strlen(buf) < 2)
					continue;
				printf("DTMF: %c\n", buf[1]);
				lgsm_voice_dtmf(lgsmh, buf[1]);
			} else {
				printf("Unknown command `%s'\n", buf);
			}
		}
	}
}
