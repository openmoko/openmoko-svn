
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <libgsmd/libgsmd.h>

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

	if (!strcmp(device, "gsmd")) {
		struct sockaddr_un sun;
		
		/* use unix domain socket to gsm daemon */
		lh->fd = socket(PF_UNIX, GSMD_UNIX_SOCKET_TYPE, 0);
		if (lh->fd < 0)
			return lh->fd;
		
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

	/* send some initial commands, such as ATV1 (verbose response)
	 * and +CRC=1 (which we currently require!) */

	return lh;
}

int lgsm_exit(struct lgsm_handle *lh)
{
	free(lh);

	return 0;
}
