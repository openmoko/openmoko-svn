/* gsmd SMS functions
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

#include <sys/socket.h>
#include <sys/un.h>

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/usock.h>
#include <gsmd/select.h>
#include <gsmd/atcmd.h>
#include <gsmd/usock.h>
#include <gsmd/unsolicited.h>
#include <gsmd/sms.h>

static const char *ts0705_memtype_name[] = {
	[GSM0705_MEMTYPE_NONE]		= "NONE",
	[GSM0705_MEMTYPE_BROADCAST]	= "BM",
	[GSM0705_MEMTYPE_ME_MESSAGE]	= "ME",
	[GSM0705_MEMTYPE_MT]		= "MT",
	[GSM0705_MEMTYPE_SIM]		= "SM",
	[GSM0705_MEMTYPE_TA]		= "TA",
	[GSM0705_MEMTYPE_SR]		= "SR",
};

static inline int parse_memtype(char *memtype)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ts0705_memtype_name); i++) {
		if (!strcmp(ts0705_memtype_name[i], memtype))
			return i;
	}

	return GSM0705_MEMTYPE_NONE;
}

static int sms_list_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd_user *gu = ctx;
	struct gsmd_ucmd *ucmd;
	struct gsmd_sms_list msg;
	int i, idx, stat, len, cr;
	u_int8_t pdu[SMS_MAX_PDU_SIZE];

	if (cmd->ret && cmd->ret != -255)
		return 0;

	/* FIXME: TEXT mode */
	if (
			sscanf(resp, "+CMGL: %i,%i,,%i\n%n",
				&idx, &stat, &len, &cr) < 3 &&
			sscanf(resp, "+CMGL: %i,%i,\"%*[^\"]\",%i\n%n",
				&idx, &stat, &len, &cr) < 3)
		return -EINVAL;
	if (len > 164)
		return -EINVAL;

	msg.index = idx;
	msg.stat = stat;
	msg.is_last = (cmd->ret == 0);
	for (i = 0; resp[cr] >= '0' && resp[cr + 1] >= '0' &&
			i < SMS_MAX_PDU_SIZE; i ++) {
		if (sscanf(resp + cr, "%2hhX", &pdu[i]) < 1) {
			gsmd_log(GSMD_DEBUG, "malformed input (%i)\n", i);
			return -EINVAL;
		}
		cr += 2;
	}
	if (sms_pdu_to_msg(&msg, pdu, len, i)) {
		gsmd_log(GSMD_DEBUG, "malformed PDU\n");
		return -EINVAL;
	}

	ucmd = gsmd_ucmd_fill(sizeof(msg), GSMD_MSG_SMS,
			GSMD_SMS_LIST, cmd->id);
	if (!ucmd)
		return -ENOMEM;
	memcpy(ucmd->buf, &msg, sizeof(msg));

	usock_cmd_enqueue(ucmd, gu);

	return 0;
}

static int sms_read_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd_user *gu = ctx;
	struct gsmd_ucmd *ucmd;
	struct gsmd_sms_list msg;
	int i, stat, len, cr;
	u_int8_t pdu[SMS_MAX_PDU_SIZE];
	const char *colon;

	if (cmd->ret)
		return 0;

	/* FIXME: TEXT mode */
	if (
			sscanf(resp, "+CMGR: %i,,%i\n%n",
				&stat, &len, &cr) < 2 &&
			sscanf(resp, "+CMGR: %i,%*i,%i\n%n",
				&stat, &len, &cr) < 2)
		return -EINVAL;
	if (len > 164)
		return -EINVAL;

	msg.index = 0;
	colon = strchr(cmd->buf, '=');

        /* get a correct message index value on reading a SMS */
	if (!strncmp(cmd->buf, "AT+CMGR", 7) && colon) 
		msg.index = atoi(colon+1);
	msg.stat = stat;
	msg.is_last = 1;
	for (i = 0; resp[cr] >= '0' && resp[cr + 1] >= '0' &&
			i < SMS_MAX_PDU_SIZE; i ++) {
		if (sscanf(resp + cr, "%2hhX", &pdu[i]) < 1) {
			gsmd_log(GSMD_DEBUG, "malformed input (%i)\n", i);
			return -EINVAL;
		}
		cr += 2;
	}
	if (sms_pdu_to_msg(&msg, pdu, len, i)) {
		gsmd_log(GSMD_DEBUG, "malformed PDU\n");
		return -EINVAL;
	}

	ucmd = gsmd_ucmd_fill(sizeof(msg), GSMD_MSG_SMS,
			GSMD_SMS_READ, cmd->id);
	if (!ucmd)
		return -ENOMEM;
	memcpy(ucmd->buf, &msg, sizeof(msg));

	usock_cmd_enqueue(ucmd, gu);

	return 0;
}

static int sms_send_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd_user *gu = ctx;
	struct gsmd_ucmd *ucmd;
	int msgref;

	if (cmd->ret == 0 || cmd->ret == -255) {
		if (sscanf(resp, "+CMGS: %i", &msgref) < 1)
			return -EINVAL;
	} else
		msgref = -cmd->ret;

	ucmd = gsmd_ucmd_fill(sizeof(int), GSMD_MSG_SMS,
			GSMD_SMS_SEND, cmd->id);
	if (!ucmd)
		return -ENOMEM;
	*(int *) ucmd->buf = msgref;

	usock_cmd_enqueue(ucmd, gu);

	return 0;
}

static int sms_write_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd_user *gu = ctx;
	struct gsmd_ucmd *ucmd;
	int result;

	if (cmd->ret == 0) {
		if (sscanf(resp, "+CMGW: %i", &result) < 1)
			return -EINVAL;
	} else
		result = -cmd->ret;

	ucmd = gsmd_ucmd_fill(sizeof(int), GSMD_MSG_SMS,
			GSMD_SMS_WRITE, cmd->id);
	if (!ucmd)
		return -ENOMEM;
	*(int *) ucmd->buf = result;

	usock_cmd_enqueue(ucmd, gu);

	return 0;
}

static int sms_delete_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd_user *gu = ctx;
	struct gsmd_ucmd *ucmd;
	int *result;

	ucmd = gsmd_ucmd_fill(sizeof(int), GSMD_MSG_SMS,
			GSMD_SMS_DELETE, cmd->id);
	if (!ucmd)
		return -ENOMEM;

	result = (int *) ucmd->buf;
	*result = cmd->ret;

	usock_cmd_enqueue(ucmd, gu);

	return 0;
}

static int usock_cpms_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd_user *gu = ctx;
	struct gsmd_ucmd *ucmd = ucmd_alloc(sizeof(struct gsmd_sms_storage));
	struct gsmd_sms_storage *gss = (typeof(gss)) ucmd->buf;
	char buf[3][3];

	DEBUGP("entering(cmd=%p, gu=%p)\n", cmd, gu);

	if (!ucmd)
		return -ENOMEM;

	ucmd->hdr.version = GSMD_PROTO_VERSION;
	ucmd->hdr.msg_type = GSMD_MSG_SMS;
	ucmd->hdr.msg_subtype = GSMD_SMS_GET_MSG_STORAGE;
	ucmd->hdr.len = sizeof(struct gsmd_sms_storage);
	ucmd->hdr.id = cmd->id;

	if (sscanf(resp, "+CPMS: \"%2[A-Z]\",%hi,%hi,"
				"\"%2[A-Z]\",%hi,%hi,\"%2[A-Z]\",%hi,%hi",
				buf[0], &gss->mem[0].used, &gss->mem[0].total,
				buf[1], &gss->mem[1].used, &gss->mem[1].total,
				buf[2], &gss->mem[2].used, &gss->mem[2].total)
			< 9) {
		talloc_free(ucmd);
		return -EINVAL;
	}

	gss->mem[0].memtype = parse_memtype(buf[0]);
	gss->mem[1].memtype = parse_memtype(buf[1]);
	gss->mem[2].memtype = parse_memtype(buf[2]);

	usock_cmd_enqueue(ucmd, gu);

	return 0;
}

static int usock_get_smsc_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd_user *gu = ctx;
	struct gsmd_ucmd *ucmd;
	struct gsmd_addr *ga;

	ucmd = gsmd_ucmd_fill(sizeof(struct gsmd_addr), GSMD_MSG_SMS,
			GSMD_SMS_GET_SERVICE_CENTRE, cmd->id);
	if (!ucmd)
		return -ENOMEM;

	ga = (struct gsmd_addr *) ucmd->buf;
	if (sscanf(resp, "+CSCA: \"%31[^\"]\",%hhi",
				ga->number, &ga->type) < 2) {
		talloc_free(ucmd);
		return -EINVAL;
	}

	usock_cmd_enqueue(ucmd, gu);
	return 0;
}

static const char *gsmd_cmgl_stat[] = {
	"REC UNREAD", "REC READ", "STO UNSENT", "STO SENT", "ALL",
};

/* main unix socket Short Message receiver */
int usock_rcv_sms(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, int len)
{
	/* FIXME: TEXT mode support!!  */
	struct gsmd_atcmd *cmd = NULL;
	struct gsmd_sms_delete *gsd;
	struct gsmd_sms_submit *gss;
	struct gsmd_sms_write *gsw;
	struct gsmd_addr *ga;
	enum ts0705_mem_type *storage;
	int *stat, *index;
	int atcmd_len;
	char buf[1024];

	switch (gph->msg_subtype) {
	case GSMD_SMS_LIST:
		if(len < sizeof(*gph) + sizeof(int))
			return -EINVAL;
		stat = (int *) ((void *)gph + sizeof(*gph));
		if (*stat < 0 || *stat > 4)
			return -EINVAL;

		if (gu->gsmd->flags & GSMD_FLAG_SMS_FMT_TEXT)
			atcmd_len = sprintf(buf, "AT+CMGL=\"%s\"",
					gsmd_cmgl_stat[*stat]);
		else
			atcmd_len = sprintf(buf, "AT+CMGL=%i", *stat);

		cmd = atcmd_fill(buf, atcmd_len + 1,
				&sms_list_cb, gu, gph->id);
		break;

	case GSMD_SMS_READ:
		if(len < sizeof(*gph) + sizeof(int))
			return -EINVAL;
		index = (int *) ((void *)gph + sizeof(*gph));

		atcmd_len = sprintf(buf, "AT+CMGR=%i", *index);

		cmd = atcmd_fill(buf, atcmd_len + 1,
				&sms_read_cb, gu, gph->id);
		break;

	case GSMD_SMS_SEND:
		if (len < sizeof(*gph) + sizeof(*gss))
			return -EINVAL;
		gss = (struct gsmd_sms_submit *) ((void *) gph + sizeof(*gph));

		if (gu->gsmd->flags & GSMD_FLAG_SMS_FMT_TEXT) {
			atcmd_len = sprintf(buf, "AT+CMGS=\"%s\"\n%.*s",
					gss->addr.number,
					gss->payload.length,
					gss->payload.data);	/* FIXME */
		} else {
			atcmd_len = sprintf(buf, "AT+CMGS=%i\n",
					sms_pdu_make_smssubmit(NULL, gss) - 1);
			atcmd_len += sms_pdu_make_smssubmit(buf + atcmd_len,
					gss) * 2;
		}
		buf[atcmd_len ++] = 26;	/* ^Z ends the message */
		buf[atcmd_len ++] = 0;

		cmd = atcmd_fill(buf, atcmd_len, &sms_send_cb, gu, gph->id);
		break;

	case GSMD_SMS_WRITE:
		if (len < sizeof(*gph) + sizeof(*gsw))
			return -EINVAL;
		gsw = (struct gsmd_sms_write *) ((void *) gph + sizeof(*gph));
		if (gsw->stat > 4)
			return -EINVAL;

		if (gu->gsmd->flags & GSMD_FLAG_SMS_FMT_TEXT) {
			atcmd_len = sprintf(buf, "AT+CMGW=\"%s\"\n%.*s",
					gsw->sms.addr.number,
					gsw->sms.payload.length,
					gsw->sms.payload.data);	/* FIXME */
		} else {
			atcmd_len = sprintf(buf, "AT+CMGW=%i,%i\n",
					sms_pdu_make_smssubmit(NULL,
						&gsw->sms) - 1, gsw->stat);
			atcmd_len += sms_pdu_make_smssubmit(buf + atcmd_len,
					&gsw->sms) * 2;
		}
		buf[atcmd_len ++] = 26;	/* ^Z ends the message */
		buf[atcmd_len ++] = 0;

		cmd = atcmd_fill(buf, atcmd_len, &sms_write_cb, gu, gph->id);
		break;

	case GSMD_SMS_DELETE:
		if(len < sizeof(*gph) + sizeof(*gsd))
			return -EINVAL;
		gsd = (struct gsmd_sms_delete *) ((void *)gph + sizeof(*gph));

		atcmd_len = sprintf(buf, "AT+CMGD=%d,%d",
				gsd->index, gsd->delflg);

		cmd = atcmd_fill(buf, atcmd_len + 1,
				&sms_delete_cb, gu, gph->id);
		break;

	case GSMD_SMS_GET_MSG_STORAGE:
		cmd = atcmd_fill("AT+CPMS?", 8 + 1, usock_cpms_cb, gu, 0);
		break;

	case GSMD_SMS_SET_MSG_STORAGE:
		if (len < sizeof(*gph) + 3 * sizeof(enum ts0705_mem_type))
			return -EINVAL;
		storage = (enum ts0705_mem_type *)
			((void *) gph + sizeof(*gph));
		atcmd_len = sprintf(buf, "AT+CPMS=\"%s\",\"%s\",\"%s\"",
				ts0705_memtype_name[storage[0]],
				ts0705_memtype_name[storage[1]],
				ts0705_memtype_name[storage[2]]);
		cmd = atcmd_fill(buf, atcmd_len + 1, NULL, gu, gph->id);
		break;

	case GSMD_SMS_GET_SERVICE_CENTRE:
		cmd = atcmd_fill("AT+CSCA?", 8 + 1, &usock_get_smsc_cb, gu, 0);
		break;

	case GSMD_SMS_SET_SERVICE_CENTRE:
		if (len < sizeof(*gph) + sizeof(struct gsmd_addr))
			return -EINVAL;
		ga = (struct gsmd_addr *) ((void *) gph + sizeof(*gph));
		atcmd_len = sprintf(buf, "AT+CSCA=\"%s\",%i",
				ga->number, ga->type);
		cmd = atcmd_fill(buf, atcmd_len + 1, NULL, gu, gph->id);
		break;

	default:
		return -ENOSYS;
	}

	if (!cmd)
		return -ENOMEM;

	gsmd_log(GSMD_DEBUG, "%s\n", cmd ? cmd->buf : 0);
	return atcmd_submit(gu->gsmd, cmd);
}

/* main unix socket Cell Broadcast receiver */
int usock_rcv_cb(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, int len)
{
	struct gsmd_atcmd *cmd;

	switch (gph->msg_subtype) {
	case GSMD_CB_SUBSCRIBE:
		cmd = atcmd_fill("AT+CSCB=1", 9 + 1, NULL, gu->gsmd, 0);
		break;
	case GSMD_CB_UNSUBSCRIBE:
		cmd = atcmd_fill("AT+CSCB=0", 9 + 1, NULL, gu->gsmd, 0);
		break;
	default:
		return -ENOSYS;
	}

	if (!cmd)
		return -ENOMEM;

	return atcmd_submit(gu->gsmd, cmd);
}

/* Unsolicited messages related to SMS / CB */
static int cmti_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{
	char memstr[3];
	struct gsmd_evt_auxdata *aux;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT,
			GSMD_EVT_IN_SMS, sizeof(struct gsmd_evt_auxdata));

	if (!ucmd)
		return -ENOMEM;

	aux = (struct gsmd_evt_auxdata *) ucmd->buf;
	if (sscanf(param, "\"%2[A-Z]\",%i", memstr, &aux->u.sms.index) < 2) {
		talloc_free(ucmd);
		return -EINVAL;
	}

	aux->u.sms.inlined = 0;
	aux->u.sms.memtype = parse_memtype(memstr);

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_IN_SMS);
}

static int cmt_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{
	/* TODO: TEXT mode */
	u_int8_t pdu[SMS_MAX_PDU_SIZE];
	const char *comma = strchr(param, ',');
	char *cr;
	int i;
	struct gsmd_evt_auxdata *aux;
	struct gsmd_sms_list *msg;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT,
			GSMD_EVT_IN_SMS, sizeof(struct gsmd_evt_auxdata) +
			sizeof(struct gsmd_sms_list));

	if (!ucmd)
		return -ENOMEM;

	aux = (struct gsmd_evt_auxdata *) ucmd->buf;
	msg = (struct gsmd_sms_list *) aux->data;

	if (!comma) {
		talloc_free(ucmd);
		return -EINVAL;
	}
	len = strtoul(comma + 1, &cr, 10);
	if (cr[0] != '\n') {
		talloc_free(ucmd);
		return -EAGAIN;
	}

	cr ++;
	for (i = 0; cr[0] >= '0' && cr[1] >= '0' && i < SMS_MAX_PDU_SIZE;
			i ++) {
		if (sscanf(cr, "%2hhX", &pdu[i]) < 1) {
			gsmd_log(GSMD_DEBUG, "malformed input (%i)\n", i);
			talloc_free(ucmd);
			return -EINVAL;
		}
		cr += 2;
	}

	aux->u.sms.inlined = 1;
	if (sms_pdu_to_msg(msg, pdu, len, i)) {
		gsmd_log(GSMD_DEBUG, "malformed PDU\n");
		talloc_free(ucmd);
		return -EINVAL;
	}

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_IN_SMS);
}

static int cbmi_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{
	char memstr[3];
	struct gsmd_evt_auxdata *aux;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT,
			GSMD_EVT_IN_CBM, sizeof(struct gsmd_evt_auxdata));

	if (!ucmd)
		return -ENOMEM;

	aux = (struct gsmd_evt_auxdata *) ucmd->buf;
	if (sscanf(param, "\"%2[A-Z]\",%i", memstr, &aux->u.cbm.index) < 2) {
		talloc_free(ucmd);
		return -EINVAL;
	}

	aux->u.cbm.inlined = 0;
	aux->u.cbm.memtype = parse_memtype(memstr);

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_IN_CBM);
}

static int cbm_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{
	/* TODO: TEXT mode */
	u_int8_t pdu[CBM_MAX_PDU_SIZE];
	char *cr;
	int i;
	struct gsmd_evt_auxdata *aux;
	struct gsmd_cbm *msg;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT,
			GSMD_EVT_IN_CBM, sizeof(struct gsmd_evt_auxdata) +
			sizeof(struct gsmd_cbm));

	if (!ucmd)
		return -ENOMEM;

	aux = (struct gsmd_evt_auxdata *) ucmd->buf;
	msg = (struct gsmd_cbm *) aux->data;

	len = strtoul(param, &cr, 10);
	if (cr[0] != '\n') {
		talloc_free(ucmd);
		return -EAGAIN;
	}

	cr ++;
	for (i = 0; cr[0] >= '0' && cr[1] >= '0' && i < CBM_MAX_PDU_SIZE;
			i ++) {
		if (sscanf(cr, "%2hhX", &pdu[i]) < 1) {
			gsmd_log(GSMD_DEBUG, "malformed input (%i)\n", i);
			talloc_free(ucmd);
			return -EINVAL;
		}
		cr += 2;
	}

	aux->u.cbm.inlined = 1;
	if (cbs_pdu_to_msg(msg, pdu, len, i)) {
		gsmd_log(GSMD_DEBUG, "malformed PDU\n");
		talloc_free(ucmd);
		return -EINVAL;
	}

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_IN_CBM);
}

static int cdsi_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{
	char memstr[3];
	struct gsmd_evt_auxdata *aux;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT,
			GSMD_EVT_IN_DS, sizeof(struct gsmd_evt_auxdata));

	if (!ucmd)
		return -ENOMEM;

	aux = (struct gsmd_evt_auxdata *) ucmd->buf;
	if (sscanf(param, "\"%2[A-Z]\",%i", memstr, &aux->u.ds.index) < 2) {
		talloc_free(ucmd);
		return -EINVAL;
	}

	aux->u.ds.inlined = 0;
	aux->u.ds.memtype = parse_memtype(memstr);

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_IN_DS);
}

static int cds_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{
	/* TODO: TEXT mode */
	u_int8_t pdu[SMS_MAX_PDU_SIZE];
	char *cr;
	int i;
	struct gsmd_evt_auxdata *aux;
	struct gsmd_sms_list *msg;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT,
			GSMD_EVT_IN_DS, sizeof(struct gsmd_evt_auxdata) +
			sizeof(struct gsmd_sms_list));

	if (!ucmd)
		return -ENOMEM;

	aux = (struct gsmd_evt_auxdata *) ucmd->buf;
	msg = (struct gsmd_sms_list *) aux->data;

	len = strtoul(param, &cr, 10);
	if (cr[0] != '\n') {
		talloc_free(ucmd);
		return -EAGAIN;
	}

	cr ++;
	for (i = 0; cr[0] >= '0' && cr[1] >= '0' && i < SMS_MAX_PDU_SIZE;
			i ++) {
		if (sscanf(cr, "%2hhX", &pdu[i]) < 1) {
			gsmd_log(GSMD_DEBUG, "malformed input (%i)\n", i);
			talloc_free(ucmd);
			return -EINVAL;
		}
		cr += 2;
	}

	aux->u.ds.inlined = 1;
	if (sms_pdu_to_msg(msg, pdu, len, i)) {
		gsmd_log(GSMD_DEBUG, "malformed PDU\n");
		talloc_free(ucmd);
		return -EINVAL;
	}

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_IN_DS);
}

static const struct gsmd_unsolicit gsm0705_unsolicit[] = {
	{ "+CMTI",	&cmti_parse },	/* SMS Deliver Index (stored in ME/TA)*/
	{ "+CMT",	&cmt_parse },	/* SMS Deliver to TE */
	{ "+CBMI",	&cbmi_parse },	/* Cell Broadcast Message Index */
	{ "+CBM",	&cbm_parse },	/* Cell Broadcast Message */
	{ "+CDSI",	&cdsi_parse },	/* SMS Status Report */
	{ "+CDS",	&cds_parse },	/* SMS Status Index (stored in ME/TA) */
};

int sms_cb_init(struct gsmd *gsmd)
{
	struct gsmd_atcmd *atcmd;
	char buffer[10];

	unsolicited_register_array(gsm0705_unsolicit,
			ARRAY_SIZE(gsm0705_unsolicit));

	/* If text mode, set the encoding */
	if (gsmd->flags & GSMD_FLAG_SMS_FMT_TEXT) {
		atcmd = atcmd_fill("AT+CSCS=\"IRA\"", 13 + 1, NULL, gsmd, 0);
		if (!atcmd)
			return -ENOMEM;
		atcmd_submit(gsmd, atcmd);
	}

	/* Switch into desired mode (Section 3.2.3) */
	atcmd = atcmd_fill(buffer, snprintf(buffer, sizeof(buffer),
				"AT+CMGF=%i",
				(gsmd->flags & GSMD_FLAG_SMS_FMT_TEXT) ?
				GSMD_SMS_FMT_TEXT : GSMD_SMS_FMT_PDU) + 1,
			NULL, gsmd, 0);
	if (!atcmd)
		return -ENOMEM;

	return atcmd_submit(gsmd, atcmd);
}

/* Called everytime the phone registers to the network and we want to start
 * receiving messages.  */
int sms_cb_network_init(struct gsmd *gsmd)
{
	int ret = 0;

	ret |= gsmd_simplecmd(gsmd, "AT+CSMS=0");

	/*
	 * Set the New Message Indications properties to values that are
	 * likely supported.  We will get a:
	 * +CMTI on a new incoming SMS,
	 * +CBM on a new incoming CB,
	 * +CDS on an SMS status report.
	 *
	 * FIXME: ask for supported +CNMI values first.
	 */
	ret |= gsmd_simplecmd(gsmd, "AT+CNMI=2,1,2,1,0");

	return ret;
}
