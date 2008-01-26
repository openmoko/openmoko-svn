/* libgsmd core
 *
 * (C) 2006-2007 by OpenMoko, Inc.
 * Written by Harald Welte <laforge@openmoko.org>
 * All Rights Reserved
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <gsmd/usock.h>
#include <libgsmd/libgsmd.h>
#include <libgsmd/event.h>

#include "lgsm_internals.h"


static int lgsm_get_packet(struct lgsm_handle *lh)
{
	static char buf[GSMD_MSGSIZE_MAX];
	struct gsmd_msg_hdr *hdr = (struct gsmd_msg_hdr *) buf;
	int rc = read(lh->fd, buf, sizeof(buf));
	if (rc <= 0)
		return rc;

	if (hdr->version != GSMD_PROTO_VERSION)
		return -EINVAL;
	
	switch (hdr->msg_type) {
	case GSMD_MSG_PASSTHROUGH:
		
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


static int lgsm_open_backend(struct lgsm_handle *lh, const char *device)
{
	int rc;

	if (!strcmp(device, LGSMD_DEVICE_GSMD)) {
		struct sockaddr_un sun;
		
		/* use unix domain socket to gsm daemon */
		lh->fd = socket(PF_UNIX, GSMD_UNIX_SOCKET_TYPE, 0);
		if (lh->fd < 0)
			return lh->fd;
		
		memset(&sun, 0, sizeof(sun));
		sun.sun_family = AF_UNIX;
		memcpy(sun.sun_path, GSMD_UNIX_SOCKET, sizeof(GSMD_UNIX_SOCKET));

		rc = connect(lh->fd, (struct sockaddr *)&sun, sizeof(sun));
		if (rc < 0) {
			close(lh->fd);
			lh->fd = -1;
			return rc;
		}
	} else 	
		return -EINVAL;

	return 0;
}

/* handle a packet that was received on the gsmd socket */
int lgsm_handle_packet(struct lgsm_handle *lh, char *buf, int len)
{
	struct gsmd_msg_hdr *gmh;
	lgsm_msg_handler *handler; 
	int rc = 0;

	while (len) {
		if (len < sizeof(*gmh))
			return -EINVAL;
		gmh = (struct gsmd_msg_hdr *) buf;

		if (len - sizeof(*gmh) < gmh->len)
			return -EINVAL;
		len -= sizeof(*gmh) + gmh->len;
		buf += sizeof(*gmh) + gmh->len;

		if (gmh->msg_type >= __NUM_GSMD_MSGS)
			return -EINVAL;

		handler = lh->handler[gmh->msg_type];

		if (handler)
			rc |= handler(lh, gmh);
		else
			fprintf(stderr, "unable to handle packet type=%u\n",
					gmh->msg_type);
	}
	return rc;
}

int lgsm_register_handler(struct lgsm_handle *lh, int type, lgsm_msg_handler *handler)
{
	if (type >= __NUM_GSMD_MSGS)
		return -EINVAL;

	lh->handler[type] = handler;

	return 0;
}

void lgsm_unregister_handler(struct lgsm_handle *lh, int type)
{
	if (type < __NUM_GSMD_MSGS)
		lh->handler[type] = NULL;
}

/* blocking read and processing of packets until packet matching 'id' is found */
int lgsm_blocking_wait_packet(struct lgsm_handle *lh, u_int16_t id,
			      struct gsmd_msg_hdr *gmh, int rlen)
{
	int rc;
	fd_set readset;

	FD_ZERO(&readset);

	while (1) {
		FD_SET(lh->fd, &readset);
		rc = select(lh->fd+1, &readset, NULL, NULL, NULL);
		if (rc <= 0)
			return rc;

		rc = read(lh->fd, (char *)gmh, rlen);
		if (rc <= 0)
			return rc;

		if (gmh->id == id) {
			/* we've found the matching packet, return to calling function */
			return rc;
		} else
			rc = lgsm_handle_packet(lh, (char *)gmh, rc);
	}
}

int lgsm_fd(struct lgsm_handle *lh)
{
	return lh->fd;
}

struct lgsm_handle *lgsm_init(const char *device)
{
	struct lgsm_handle *lh = malloc(sizeof(*lh));

	memset(lh, 0, sizeof(*lh));
	lh->fd = -1;

	if (lgsm_open_backend(lh, device) < 0) {
		free(lh);
		return NULL;
	}

	lgsm_evt_init(lh);

	return lh;
}

int lgsm_exit(struct lgsm_handle *lh)
{
	free(lh);

	return 0;
}


static u_int16_t next_msg_id;

int lgsm_send(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	gmh->id = next_msg_id++;
	return send(lh->fd, (char *) gmh, sizeof(*gmh) + gmh->len, 0);
}

struct gsmd_msg_hdr *lgsm_gmh_fill(int type, int subtype, int payload_len)
{
	struct gsmd_msg_hdr *gmh = malloc(sizeof(*gmh)+payload_len);
	if (!gmh)
		return NULL;

	memset(gmh, 0, sizeof(*gmh)+payload_len);

	gmh->version = GSMD_PROTO_VERSION;
	gmh->msg_type = type;
	gmh->msg_subtype = subtype;
	gmh->len = payload_len;

	return gmh;
}

int lgsm_send_simple(struct lgsm_handle *lh, int type, int sub_type)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(type, sub_type, 0);
	if (!gmh)
		return -ENOMEM;
	rc = lgsm_send(lh, gmh);

	lgsm_gmh_free(gmh);
	return rc;
}
