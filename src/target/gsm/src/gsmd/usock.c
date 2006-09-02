#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <gsmd/gsmd.h>
#include <gsmd/usock.h>

#include "gsmd.h"
#include "select.h"
#include "atcmd.h"
#include "usock.h"

/* callback for completed passthrough gsmd_atcmd's */
static int usock_cmd_cb(struct gsmd_atcmd *cmd, void *ctx)
{
	struct gsmd_user *gu = ctx;
	struct gsmd_ucmd *ucmd = malloc(sizeof(*ucmd)+cmd->buflen);

	if (!ucmd)
		return -ENOMEM;
	
	ucmd->hdr.version = GSMD_PROTO_VERSION;
	ucmd->hdr.msg_type = GSMD_MSG_PASSTHROUGH;
	ucmd->hdr.msg_subtype = GSMD_PASSTHROUGH_RESP;
	ucmd->hdr.len = cmd->buflen;
	memcpy(ucmd->buf, cmd->buf, ucmd->hdr.len);

	/* add to per-user list of finished cmds */
	llist_add_tail(&ucmd->list, &gu->finished_ucmds);

	/* mark socket of user as we-want-to-write */
	gu->gfd.when |= GSMD_FD_WRITE;

	return 0;
}

static int usock_rcv_pcmd(struct gsmd_user *gu, char *buf, int len)
{
	struct gsmd_msg_hdr *gph = (struct gsmd_msg_hdr *)buf;

	if (gph->version != GSMD_PROTO_VERSION)
		return -EINVAL;

	switch (gph->msg_type) {
	case GSMD_MSG_PASSTHROUGH: 
		{
			struct gsmd_atcmd *cmd;
			cmd = atcmd_fill((char *)gph+sizeof(*gph),
					 255, &usock_cmd_cb, gu);
			if (!cmd)
				return -ENOMEM;
			return atcmd_submit(gu->gsmd, cmd);
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/* callback for read/write on client (libgsmd) socket */
static int gsmd_usock_user_cb(int fd, unsigned int what, void *data)
{
	struct gsmd_user *gu = data;

	/* FIXME: check some kind of backlog and limit it */

	if (what & GSMD_FD_READ) {
		char buf[1024];
		int rcvlen;
		/* read data from socket, determine what he wants */
		rcvlen = read(fd, buf, sizeof(buf));
		return usock_rcv_pcmd(gu, buf, rcvlen);
	}

	if (what & GSMD_FD_WRITE) {
		/* write data from pending replies to socket */
		struct gsmd_ucmd *ucmd, *uctmp;
		llist_for_each_entry_safe(ucmd, uctmp, &gu->finished_ucmds,
					  list) {
			int rc;

			rc = write(fd, &ucmd->hdr, sizeof(ucmd->hdr) + ucmd->hdr.len);
			if (rc < 0) {
				DEBUGP("write return %d\n", rc);
				return rc;
			}
			if (rc == 0) {
				DEBUGP("write returns zero!!\n");
				break;
			}
			if (rc != sizeof(ucmd->hdr) + ucmd->hdr.len) {
				DEBUGP("short write\n");
				break;
			}

			llist_del(&ucmd->list);
			free(ucmd);
		}
	}

	return 0;
}

/* callback for read on master-listen-socket */
static int gsmd_usock_cb(int fd, unsigned int what, void *data)
{
	struct gsmd *g = data;
	struct gsmd_user *newuser;

	/* FIXME: implement this */
	if (what & GSMD_FD_READ) {
		/* new incoming connection */
		newuser = malloc(sizeof(*newuser));
		if (!newuser)
			return -ENOMEM;
		
		newuser->gfd.fd = accept(fd, NULL, 0);
		if (newuser->gfd.fd < 0) {
			DEBUGP("error accepting incoming conn: `%s'\n",
				strerror(errno));
			free(newuser);
		}
		newuser->gfd.when = GSMD_FD_READ;
		newuser->gfd.data = newuser;
		newuser->gfd.cb = &gsmd_usock_user_cb;
		newuser->gsmd = g;

		llist_add(&newuser->list, &g->users);
	}

	return 0;
}

/* handling of socket with incoming client connections */
int usock_init(struct gsmd *g)
{
	struct sockaddr_un sun;
	int fd, rc;

	fd = socket(PF_UNIX, GSMD_UNIX_SOCKET_TYPE, 0);
	if (fd < 0)
		return fd;
	
	sun.sun_family = AF_UNIX;
	memcpy(sun.sun_path, GSMD_UNIX_SOCKET, sizeof(GSMD_UNIX_SOCKET));

	rc = bind(fd, (struct sockaddr *)&sun, sizeof(sun));
	if (rc < 0) {
		close(fd);
		return rc;
	}

	g->gfd_sock.fd = fd;
	g->gfd_sock.when = GSMD_FD_READ | GSMD_FD_EXCEPT;
	g->gfd_sock.data = g;
	g->gfd_sock.cb = &gsmd_usock_cb;

	return gsmd_register_fd(&g->gfd_sock);
}
