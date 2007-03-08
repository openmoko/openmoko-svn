/* gsmd unix domain socket handling
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/usock.h>
#include <gsmd/select.h>
#include <gsmd/atcmd.h>
#include <gsmd/usock.h>
#include <gsmd/talloc.h>

static void *__ucmd_ctx, *__gu_ctx;

struct gsmd_ucmd *ucmd_alloc(int extra_size)
{
	return talloc_size(__ucmd_ctx, 
			   sizeof(struct gsmd_ucmd) + extra_size);
}

void usock_cmd_enqueue(struct gsmd_ucmd *ucmd, struct gsmd_user *gu)
{
	DEBUGP("enqueueing usock cmd %p for user %p\n", ucmd, gu);

	/* add to per-user list of finished cmds */
	llist_add_tail(&ucmd->list, &gu->finished_ucmds);

	/* mark socket of user as we-want-to-write */
	gu->gfd.when |= GSMD_FD_WRITE;
}

/* callback for completed passthrough gsmd_atcmd's */
static int usock_cmd_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd_user *gu = ctx;
	int rlen = strlen(resp)+1;
	struct gsmd_ucmd *ucmd = ucmd_alloc(rlen);

	DEBUGP("entering(cmd=%p, gu=%p)\n", cmd, gu);

	if (!ucmd)
		return -ENOMEM;
	
	/* FIXME: pass error values back somehow */
	ucmd->hdr.version = GSMD_PROTO_VERSION;
	ucmd->hdr.msg_type = GSMD_MSG_PASSTHROUGH;
	ucmd->hdr.msg_subtype = GSMD_PASSTHROUGH_RESP;
	ucmd->hdr.len = strlen(resp)+1;
	ucmd->hdr.id = cmd->id;
	memcpy(ucmd->buf, resp, ucmd->hdr.len);

	usock_cmd_enqueue(ucmd, gu);

	return 0;
}

typedef int usock_msg_handler(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, int len);

static int usock_rcv_passthrough(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, int len)
{
	struct gsmd_atcmd *cmd;
	cmd = atcmd_fill((char *)gph+sizeof(*gph), gph->len, &usock_cmd_cb, gu, gph->id);
	if (!cmd)
		return -ENOMEM;

	DEBUGP("submitting cmd=%p, gu=%p\n", cmd, gu);

	return atcmd_submit(gu->gsmd, cmd);
}

static int usock_rcv_event(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, int len)
{
	u_int32_t *evtmask = (u_int32_t *) ((char *)gph + sizeof(*gph), gph->id);

	if (len < sizeof(*gph) + sizeof(u_int32_t))
		return -EINVAL;

	if (gph->msg_subtype != GSMD_EVT_SUBSCRIPTIONS)
		return -EINVAL;

	gu->subscriptions = *evtmask;
}

static int usock_rcv_voicecall(struct gsmd_user *gu, struct gsmd_msg_hdr *gph,
				int len)
{
	struct gsmd_atcmd *cmd = NULL;
	struct gsmd_addr *ga;
	struct gsmd_dtmf *gd;
	int atcmd_len;

	switch (gph->msg_subtype) {
	case GSMD_VOICECALL_DIAL:
		if (len < sizeof(*gph) + sizeof(*ga))
			return -EINVAL;
		ga = (struct gsmd_addr *) ((void *)gph + sizeof(*gph));
		ga->number[GSMD_ADDR_MAXLEN] = '\0';
		cmd = atcmd_fill("ATD", 5 + strlen(ga->number),
				 &usock_cmd_cb, gu, gph->id);
		if (!cmd)
			return -ENOMEM;
		sprintf(cmd->buf, "ATD%s;", ga->number);
		/* FIXME: number type! */
		break;
	case GSMD_VOICECALL_HANGUP:
		cmd = atcmd_fill("ATH0", 5, &usock_cmd_cb, gu, gph->id);
		break;
	case GSMD_VOICECALL_ANSWER:
		cmd = atcmd_fill("ATA", 4, &usock_cmd_cb, gu, gph->id);
		break;
	case GSMD_VOICECALL_DTMF:
		if (len < sizeof(*gph) + sizeof(*gd))
			return -EINVAL;

		gd = (struct gsmd_dtmf *) ((void *)gph + sizeof(*gph));
		if (len < sizeof(*gph) + sizeof(*gd) + gd->len)
			return -EINVAL;

		/* FIXME: we don't yet support DTMF of multiple digits */
		if (gd->len != 1)
			return -EINVAL;

		atcmd_len = 1 + strlen("AT+VTS=") + (gd->len * 2);
		cmd = atcmd_fill("AT+VTS=", atcmd_len, &usock_cmd_cb,
				 gu, gph->id);
		if (!cmd)
			return -ENOMEM;

		sprintf(cmd->buf, "AT+VTS=%c;", gd->dtmf[0]);
		break;
	default:
		return -EINVAL;
	}

	if (cmd)
		return atcmd_submit(gu->gsmd, cmd);
	else
		return 0;
}

static int null_cmd_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	gsmd_log(GSMD_DEBUG, "null cmd cb\n");
	return 0;
}

static int usock_rcv_pin(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, 
			 int len)
{
	u_int8_t *pin = (u_int8_t *)gph + sizeof(*gph);
	int pin_len = len - sizeof(*gph);
	struct gsmd_atcmd *cmd;

	switch (gph->msg_subtype) {
	case GSMD_PIN_INPUT:
		/* FIXME */
		break;
	default:
		gsmd_log(GSMD_ERROR, "unknown pin type %u\n",
			 gph->msg_subtype);
		return -EINVAL;
	}

	cmd = atcmd_fill("AT+CPIN=\"", 9+1+1+strlen(pin),
			 &null_cmd_cb, gu, 0);
	if (!cmd)
		return -ENOMEM;

	strcat(cmd->buf, pin);
	strcat(cmd->buf, "\"");

	return atcmd_submit(gu->gsmd, cmd);
}

static int usock_rcv_phone(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, 
			   int len)
{
	struct gsmd_atcmd *cmd;

	switch (gph->msg_subtype) {
	case GSMD_PHONE_POWERUP:
		cmd = atcmd_fill("AT+CFUN=1", 9+1,
				 &null_cmd_cb, gu, 0);
		gu->gsmd->dev_state.on = 1;
		break;

	case GSMD_PHONE_POWERDOWN:
		cmd = atcmd_fill("AT+CFUN=0", 9+1,
				 &null_cmd_cb, gu, 0);
		gu->gsmd->dev_state.on = 0;
		break;
	default:
		return -EINVAL;
	}
	if (!cmd)
		return -ENOMEM;

	return atcmd_submit(gu->gsmd, cmd);
}

static int network_vmail_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd_user *gu = ctx;
	struct gsmd_voicemail *vmail;
	struct gsmd_ucmd *ucmd;
	char *comma;

	DEBUGP("entering(cmd=%p, gu=%p)\n", cmd, gu);

	ucmd = ucmd_alloc(sizeof(*vmail));
	if (!ucmd)
		return -ENOMEM;
	
	/* FIXME: pass error values back somehow */
	ucmd->hdr.version = GSMD_PROTO_VERSION;
	ucmd->hdr.msg_type = GSMD_MSG_NETWORK;
	ucmd->hdr.len = sizeof(*vmail);
	ucmd->hdr.id = cmd->id;

	if (cmd->buf[7] == '=') {
		/* response to set command */
		ucmd->hdr.msg_subtype = GSMD_NETWORK_VMAIL_SET;
		/* FIXME: */
		return 0;
	} else {
		/* response to get command */
		char *tok = strtok(resp, ",");
		if (!tok)
			goto out_free_einval;
		ucmd->hdr.msg_subtype = GSMD_NETWORK_VMAIL_GET;
		vmail->enable = atoi(tok);

		tok = strtok(NULL, ",");
		if (!tok)
			goto out_free_einval;
		strncpy(vmail->addr.number, tok, GSMD_ADDR_MAXLEN);
		vmail->addr.number[GSMD_ADDR_MAXLEN] = '\0';

		tok = strtok(NULL, ",");
		if (!tok)
			goto out_free_einval;
		vmail->addr.type = atoi(tok);
	}

	usock_cmd_enqueue(ucmd, gu);

	return 0;

out_free_einval:
	gsmd_log(GSMD_ERROR, "can't understand voicemail response\n");
	talloc_free(ucmd);
	return -EINVAL;
}

static struct gsmd_ucmd *gsmd_ucmd_fill(int len, u_int8_t msg_type, u_int8_t msg_subtype,
					u_int16_t id)
{
	struct gsmd_ucmd *ucmd;

	ucmd = ucmd_alloc(len);
	if (!ucmd)
		return NULL;
	
	ucmd->hdr.version = GSMD_PROTO_VERSION;
	ucmd->hdr.msg_type = msg_type;
	ucmd->hdr.msg_subtype = msg_subtype;
	ucmd->hdr.len = len;
	ucmd->hdr.id = id;

	return ucmd;
}

static int network_sigq_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd_user *gu = ctx;
	struct gsmd_signal_quality *gsq;
	struct gsmd_ucmd *ucmd;
	char *comma;
	
	ucmd = gsmd_ucmd_fill(sizeof(*gsq), GSMD_MSG_NETWORK,
			      GSMD_NETWORK_SIGQ_GET, 0);
	if (!ucmd)
		return -ENOMEM;
	
	gsq = (struct gsmd_signal_quality *) ucmd->buf;
	gsq->rssi = atoi(resp);
	comma = strchr(resp, ',');
	if (!comma) {
		talloc_free(ucmd);
		return -EIO;
	}
	gsq->ber = atoi(comma+1);

	usock_cmd_enqueue(ucmd, gu);

	return 0;
}

static int network_oper_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd_user *gu = ctx;
	struct gsmd_signal_quality *gsq;
	struct gsmd_ucmd *ucmd;
	char *comma;
	
	ucmd = gsmd_ucmd_fill(sizeof(*gsq), GSMD_MSG_NETWORK,
			      GSMD_NETWORK_OPER_GET, 0);
	if (!ucmd)
		return -ENOMEM;

	/* FIXME: implementation */

	usock_cmd_enqueue(ucmd, gu);

	return 0;
}

static int usock_rcv_network(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, 
			     int len)
{
	struct gsmd_atcmd *cmd;
	struct gsmd_voicemail *vmail = (struct gsmd_voicemail *) gph->data;

	switch (gph->msg_subtype) {
	case GSMD_NETWORK_REGISTER:
		cmd = atcmd_fill("AT+COPS", 9+1,
				 &null_cmd_cb, gu, 0);
		break;
	case GSMD_NETWORK_VMAIL_GET:
		cmd = atcmd_fill("AT+CSVM?", 8+1, &network_vmail_cb, gu, 0);
		break;
	case GSMD_NETWORK_VMAIL_SET:
		cmd = atcmd_fill("AT+CSVM=", 8+1, &network_vmail_cb, gu, 0);
		break;
	case GSMD_NETWORK_SIGQ_GET:
		cmd = atcmd_fill("AT+CSQ", 6+1, &network_sigq_cb, gu, 0);
		break;
	case GSMD_NETWORK_OPER_GET:
		cmd = atcmd_fill("AT+COPS?", 8+1, &network_oper_cb, gu, 0);
		break;
	default:
		return -EINVAL;
	}
	if (!cmd)
		return -ENOMEM;

	return atcmd_submit(gu->gsmd, cmd);
}


static usock_msg_handler *pcmd_type_handlers[__NUM_GSMD_MSGS] = {
	[GSMD_MSG_PASSTHROUGH]	= &usock_rcv_passthrough,
	[GSMD_MSG_EVENT]	= &usock_rcv_event,
	[GSMD_MSG_VOICECALL]	= &usock_rcv_voicecall,
	[GSMD_MSG_PIN]		= &usock_rcv_pin,
	[GSMD_MSG_PHONE]	= &usock_rcv_phone,
	[GSMD_MSG_NETWORK]	= &usock_rcv_network,
};

static int usock_rcv_pcmd(struct gsmd_user *gu, char *buf, int len)
{
	struct gsmd_msg_hdr *gph = (struct gsmd_msg_hdr *)buf;
	usock_msg_handler *umh;

	if (gph->version != GSMD_PROTO_VERSION)
		return -EINVAL;

	if (gph->msg_type >= __NUM_GSMD_MSGS)
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
			DEBUGP("EOF, this client has just vanished\n");
			/* EOF, this client has just vanished */
			gsmd_unregister_fd(&gu->gfd);
			close(fd);
			/* destroy whole user structure */
			llist_del(&gu->list);
			/* FIXME: delete busy ucmds from finished_ucmds */
			talloc_free(gu);
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

			DEBUGP("successfully sent cmd %p to user %p, freeing\n", ucmd, gu);
			llist_del(&ucmd->list);
			talloc_free(ucmd);
		}
		if (llist_empty(&gu->finished_ucmds))
			gu->gfd.when &= ~GSMD_FD_WRITE;
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
		newuser = talloc(__gu_ctx, struct gsmd_user);
		if (!newuser)
			return -ENOMEM;
		
		newuser->gfd.fd = accept(fd, NULL, 0);
		if (newuser->gfd.fd < 0) {
			DEBUGP("error accepting incoming conn: `%s'\n",
				strerror(errno));
			talloc_free(newuser);
		}
		newuser->gfd.when = GSMD_FD_READ;
		newuser->gfd.data = newuser;
		newuser->gfd.cb = &gsmd_usock_user_cb;
		newuser->gsmd = g;
		newuser->subscriptions = 0xffffffff;
		INIT_LLIST_HEAD(&newuser->finished_ucmds);

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

	__ucmd_ctx = talloc_named_const(gsmd_tallocs, 1, "ucmd");
	__gu_ctx = talloc_named_const(gsmd_tallocs, 1, "gsmd_user");

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
