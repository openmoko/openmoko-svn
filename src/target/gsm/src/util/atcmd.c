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

#define STDIN_BUF_SIZE	1024

/* this is the handler for receiving passthrough responses */
static int pt_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	char *payload = (char *)gmh + sizeof(*gmh);
	printf("RSTR=`%s'\n", payload);
	return 0;
}

int atcmd_main(struct lgsm_handle *lgsmh)
{
	int rc;
	char buf[STDIN_BUF_SIZE+1];
	char rbuf[STDIN_BUF_SIZE+1];
	unsigned int rlen = sizeof(rbuf);
	fd_set readset;

	lgsm_register_handler(lgsmh, GSMD_MSG_PASSTHROUGH, &pt_msghandler);

	fcntl(0, F_SETFD, O_NONBLOCK);
	fcntl(1, F_SETFD, O_NONBLOCK);
	fcntl(2, F_SETFD, O_NONBLOCK);
	fcntl(lgsm_fd(lgsmh), F_SETFD, O_NONBLOCK);

	FD_ZERO(&readset);

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
			if (rc < 0)
				printf("ERROR processing packet: %d(%s)\n", rc, strerror(-rc));
		}
		if (FD_ISSET(0, &readset)) {
			/* we've received something on stdin.  send it as passthrough
			 * to gsmd */
			rc = fscanf(stdin, "%s", buf);
			if (rc == EOF) {
				printf("EOF\n");
				return -1;
			}
			if (rc <= 0) {
				printf("NULL\n");
				continue;
			}
			printf("STR=`%s'\n", buf);

			/* this is a synchronous call for a passthrough
			 * command */
			rlen = STDIN_BUF_SIZE + 1;
			lgsm_passthrough(lgsmh, buf, rbuf, &rlen);
			printf("RSTR=`%s'\n", rbuf);

			fflush(stdout);
		}
	}
	return 0;
}
