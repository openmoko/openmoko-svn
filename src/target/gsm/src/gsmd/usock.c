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

typedef int usock_msg_handler(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, int len);

static int usock_rcv_passthrough(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, int len)
{
	struct gsmd_atcmd *cmd;
	cmd = atcmd_fill((char *)gph+sizeof(*gph), 255, &usock_cmd_cb, gu);
	if (!cmd)
		return -ENOMEM;

	return atcmd_submit(gu->gsmd, cmd);
}

static int usock_rcv_event(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, int len)
{
	u_int32_t *evtmask = (u_int32_t *) ((char *)gph + sizeof(*gph));

	if (len < sizeof(*gph) + sizeof(u_int32_t))
		return -EINVAL;

	if (gph->msg_subtype != GSMD_EVENT_SUBSCRIPTIONS)
		return -EINVAL;

	gu->subscriptions = *evtmask;
}

static int usock_rcv_voicecall(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, int len)
{
	struct gsmd_atcmd *cmd;

	switch (gph->msg_subtype) {
	case GSMD_VOICECALL_DIAL:
		/* FIXME */
		break;
	case GSMD_VOICECALL_HANGUP:
		cmd = atcmd_fill("ATH0", 5, &usock_cmd_cb, gu);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

#define GSMD_PIN_MAXLEN 16

static int usock_rcv_pin(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, int len)
{
	u_int8_t *pin = (u_int8_t *)gph + sizeof(*gph);
	int pin_len = len - sizeof(*gph);
	char pinbuf[GSMD_PIN_MAXLEN + 11]; /* `AT+CPIN=""\0' */

	snprintf(pinbuf, sizeof(pinbuf), "AT+CPIN=\"%s\"", pin);

	switch (gph->msg_subtype) {
	case GSMD_PIN_INPUT:
		/* FIXME */
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static usock_msg_handler *pcmd_type_handlers[] = {
	[GSMD_MSG_PASSTHROUGH]	= &usock_rcv_passthrough,
	[GSMD_MSG_EVENT]	= &usock_rcv_event,
	[GSMD_MSG_VOICECALL]	= &usock_rcv_voicecall,
	[GSMD_MSG_PIN]		= &usock_rcv_pin,
};

static int usock_rcv_pcmd(struct gsmd_user *gu, char *buf, int len)
{
	struct gsmd_msg_hdr *gph = (struct gsmd_msg_hdr *)buf;
	usock_msg_handler *umh;

	if (gph->version != GSMD_PROTO_VERSION)
		return -EINVAL;

	if (gph->msg_type >= ARRAY_SIZE(pcmd_type_handlers))
		return -EINVAL;
	
	umh = pcmd_type_handlers[gph->msg_type];
	if (!umh)
		return -EINVAL;

	return umh(gu, gph, len);
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
		if (rcvlen == 0) {
			/* EOF, this client has just vanished */
			gsmd_unregister_fd(&gu->gfd);
			close(fd);
			/* destroy whole user structure */
			llist_del(&gu->list);
			/* FIXME: delete budy ucmds from finished_ucmds */
			return 0;
		} else if (rcvlen < 0)
			return rcvlen;
		else
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
		gsmd_register_fd(&newuser->gfd);
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
	
	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	memcpy(sun.sun_path, GSMD_UNIX_SOCKET, sizeof(GSMD_UNIX_SOCKET));

	rc = bind(fd, (struct sockaddr *)&sun, sizeof(sun));
	if (rc < 0) {
		close(fd);
		return rc;
	}

	rc = listen(fd, 10);
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
