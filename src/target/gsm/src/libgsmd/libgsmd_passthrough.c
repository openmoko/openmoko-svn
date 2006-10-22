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

static u_int16_t next_msg_id;

static int lgsm_send(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	gmh->id = next_msg_id++;
	return send(lh->fd, (char *) gmh, sizeof(*gmh) + gmh->len, 0);
}

#define PT_BUF_SIZE	1024
static char passthrough_buf[sizeof(struct gsmd_msg_hdr)+PT_BUF_SIZE];
static char passthrough_rbuf[sizeof(struct gsmd_msg_hdr)+PT_BUF_SIZE];

int lgsm_passthrough(struct lgsm_handle *lh, const char *tx, char *rx, unsigned int *rx_len)
{
	struct gsmd_msg_hdr *gmh = (struct gsmd_msg_hdr *)passthrough_buf;
	struct gsmd_msg_hdr *rgmh = (struct gsmd_msg_hdr *)passthrough_rbuf;
	char *tx_buf = (char *)gmh + sizeof(*gmh);
	char *rx_buf = (char *)rgmh + sizeof(*rgmh);
	int len = strlen(tx);
	int rc;

	if (len > PT_BUF_SIZE)
		return -EINVAL;

	gmh->version = GSMD_PROTO_VERSION;
	gmh->msg_type = GSMD_MSG_PASSTHROUGH;
	gmh->msg_subtype = GSMD_PASSTHROUGH_REQ;
	gmh->len = len+1;
	strcpy(tx_buf, tx);

	rc = lgsm_send(lh, gmh);
	if (rc < len+sizeof(*gmh))
		return rc;

	/* since we synchronously want to wait for a response, we need to _internally_ loop over
	 * incoming packets and call the callbacks for intermediate messages (if applicable) */
	rc = lgsm_blocking_wait_packet(lh, gmh->id, passthrough_rbuf, sizeof(passthrough_rbuf));
	if (rc <= 0)
		return rc;

	if (rc < sizeof(*rgmh))
		return -EINVAL;

	if (rc < sizeof(*rgmh) + rgmh->len)
		return -EINVAL;
	
	/* FIXME: make sure rx_buf is zero-terminated */
	strcpy(rx, rx_buf);
	*rx_len = rgmh->len;

	return rx_len;
}
