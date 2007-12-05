/* libgsmd passthrough
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

#include <gsmd/usock.h>
#include <libgsmd/libgsmd.h>

#include "lgsm_internals.h"

#define PT_BUF_SIZE	1024
static char passthrough_buf[sizeof(struct gsmd_msg_hdr)+PT_BUF_SIZE];
static char passthrough_rbuf[sizeof(struct gsmd_msg_hdr)+PT_BUF_SIZE];

int lgsm_passthrough_send(struct lgsm_handle *lh, const char *tx)
{
	struct gsmd_msg_hdr *gmh = (struct gsmd_msg_hdr *)passthrough_buf;
	char *tx_buf = (char *)gmh + sizeof(*gmh);
	int len = strlen(tx);

	if (len > PT_BUF_SIZE)
		return -EINVAL;

	gmh->version = GSMD_PROTO_VERSION;
	gmh->msg_type = GSMD_MSG_PASSTHROUGH;
	gmh->msg_subtype = GSMD_PASSTHROUGH_REQ;
	gmh->len = len+1;
	strcpy(tx_buf, tx);

	if (lgsm_send(lh, gmh) < len+sizeof(*gmh))
		return -EIO;

	return gmh->id;
}

int lgsm_passthrough(struct lgsm_handle *lh, const char *tx,
		char *rx, unsigned int *rx_len)
{
	struct gsmd_msg_hdr *rgmh = (struct gsmd_msg_hdr *)passthrough_rbuf;
	char *rx_buf = (char *)rgmh + sizeof(*rgmh);
	int rc;

	rc = lgsm_passthrough_send(lh, tx);
	if (rc < 0)
		return rc;

	/* since we synchronously want to wait for a response, we need to
	 * _internally_ loop over incoming packets and call the callbacks for
	 * intermediate messages (if applicable) */
	rc = lgsm_blocking_wait_packet(lh, rc, rgmh, sizeof(passthrough_rbuf));
	if (rc <= 0)
		return rc;

	if (rc < sizeof(*rgmh))
		return -EINVAL;

	if (rc < sizeof(*rgmh) + rgmh->len)
		return -EINVAL;

	rx[--*rx_len] = 0;
	if (rgmh->len < *rx_len)
		*rx_len = rgmh->len;
	memcpy(rx, rx_buf, *rx_len);

	return *rx_len;
}
