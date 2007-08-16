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

enum ts0705_mem_type {
	GSM0705_MEMTYPE_NONE,
	GSM0705_MEMTYPE_BROADCAST,
	GSM0705_MEMTYPE_ME_MESSAGE,
	GSM0705_MEMTYPE_MT,
	GSM0705_MEMTYPE_SIM,
	GSM0705_MEMTYPE_TA,
	GSM0705_MEMTYPE_SR,
};

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

/* TODO: move to headers */
struct __gsmd_sms_storage {
	u_int8_t memtype;
	u_int8_t pad[3];
	u_int16_t used;
	u_int16_t total;
} __attribute__ ((packed));

struct gsmd_sms_storage {
	struct __gsmd_sms_storage mem[3];
} __attribute__ ((packed));

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

/* main unix socket SMS receiver */
static int usock_rcv_sms(struct gsmd_user *gu, struct gsmd_msg_hdr *gph,
			 int len)
{
	struct gsmd_atcmd *cmd;

	switch (gph->msg_subtype) {
	case GSMD_SMS_GET_SERVICE_CENTRE:
		return;
	case GSMD_SMS_SET_SERVICE_CENTRE:
		return;
	case GSMD_SMS_SET_MSG_STORAGE:
		return;
	case GSMD_SMS_GET_MSG_STORAGE:
		cmd = atcmd_fill("AT+CPMS?", 8 + 1, usock_cpms_cb, gu, 0);
		break;
	}

	return atcmd_submit(gu->gsmd, cmd);
}

/* main unix socket Cell Broadcast receiver */
static int usock_rcv_cb(struct gsmd_user *gu, struct gsmd_msg_hdr *gph,
			int len)
{
	switch (gph->msg_subtype) {
	case GSMD_CB_SUBSCRIBE:
		break;
	case GSMD_CB_UNSUBSCRIBE:
		break;
	}

	return -ENOSYS;
}

/* Unsolicited messages related to SMS / CB */
static int cmti_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	char memstr[3];
	struct gsmd_ucmd *ucmd = ucmd_alloc(sizeof(struct gsmd_evt_auxdata));
	struct gsmd_evt_auxdata *aux = (struct gsmd_evt_auxdata *) ucmd->buf;

	if (!ucmd)
		return -ENOMEM;

	ucmd->hdr.version = GSMD_PROTO_VERSION;
	ucmd->hdr.msg_type = GSMD_MSG_EVENT;
	ucmd->hdr.msg_subtype = GSMD_EVT_IN_SMS;
	ucmd->hdr.len = sizeof(*aux);

	if (sscanf(param, "\"%2[A-Z]\",%i", memstr, &aux->u.sms.index) < 2) {
		talloc_free(ucmd);
		return -EINVAL;
	}

	aux->u.sms.memtype = parse_memtype(memstr);

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_IN_SMS);
}

static int cmt_parse(char *buf, int len, const char *param,
		     struct gsmd *gsmd)
{
	/* TODO: TEXT mode */
	u_int8_t pdu[180];
	const char *comma = strchr(param, ',');
	char *cr;
	int i;
	struct gsmd_sms_list msg;

	if (!comma)
		return -EINVAL;
	len = strtoul(comma + 1, &cr, 10);
	if (cr[0] != '\n')
		return -EINVAL;

	cr ++;
	for (i = 0; cr[0] >= '0' && cr[1] >= '0' && i < 180; i ++) {
		if (sscanf(cr, "%2hhX", &pdu[i]) < 1) {
			gsmd_log(GSMD_DEBUG, "malformed input (%i)\n", i);
			return -EINVAL;
		}
		cr += 2;
	}
	if (sms_pdu_to_msg(&msg, pdu, len, i)) {
		gsmd_log(GSMD_DEBUG, "malformed PDU\n");
		return -EINVAL;
	}

	/* FIXME: generate some kind of event */
	return -ENOSYS;
}

static int cbmi_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	char memstr[3];
	int memtype, index;

	if (sscanf(param, "\"%2[A-Z]\",%i", memstr, &index) < 2)
		return -EINVAL;

	memtype = parse_memtype(memstr);
	/* FIXME: generate some kind of event */
	return -ENOSYS;
}

static int cbm_parse(char *buf, int len, const char *param,
		     struct gsmd *gsmd)
{
	/* TODO: TEXT mode */
	u_int8_t pdu[180];
	char *cr;
	int i;
	struct gsmd_sms_list msg;

	len = strtoul(param, &cr, 10);
	if (cr[0] != '\n')
		return -EINVAL;

	cr ++;
	for (i = 0; cr[0] >= '0' && cr[1] >= '0' && i < 180; i ++) {
		if (sscanf(cr, "%2hhX", &pdu[i]) < 1) {
			gsmd_log(GSMD_DEBUG, "malformed input (%i)\n", i);
			return -EINVAL;
		}
		cr += 2;
	}
	if (sms_pdu_to_msg(&msg, pdu, len, i)) {
		gsmd_log(GSMD_DEBUG, "malformed PDU\n");
		return -EINVAL;
	}

	/* FIXME: generate some kind of event */
	return -ENOSYS;
}

static int cdsi_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	char memstr[3];
	int memtype, index;

	if (sscanf(param, "\"%2[A-Z]\",%i", memstr, &index) < 2)
		return -EINVAL;

	memtype = parse_memtype(memstr);
	/* FIXME: generate some kind of event */
	return -ENOSYS;
}

static int cds_parse(char *buf, int len, const char *param,
		     struct gsmd *gsmd)
{
	/* TODO: TEXT mode */
	u_int8_t pdu[180];
	char *cr;
	int i;
	struct gsmd_sms_list msg;

	len = strtoul(param, &cr, 10);
	if (cr[0] != '\n')
		return -EINVAL;

	cr ++;
	for (i = 0; cr[0] >= '0' && cr[1] >= '0' && i < 180; i ++) {
		if (sscanf(cr, "%2hhX", &pdu[i]) < 1) {
			gsmd_log(GSMD_DEBUG, "malformed input (%i)\n", i);
			return -EINVAL;
		}
		cr += 2;
	}
	if (sms_pdu_to_msg(&msg, pdu, len, i)) {
		gsmd_log(GSMD_DEBUG, "malformed PDU\n");
		return -EINVAL;
	}

	/* FIXME: generate some kind of event */
	return -ENOSYS;
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

	atcmd = atcmd_fill("AT+CSMS=0", 9 + 1, NULL, gsmd, 0);
	if (!atcmd)
		return -ENOMEM;
	atcmd_submit(gsmd, atcmd);

	/* Store and notify */
	atcmd = atcmd_fill("AT+CNMI=1,1,1", 13 + 1, NULL, gsmd, 0);
	if (!atcmd)
		return -ENOMEM;
	atcmd_submit(gsmd, atcmd);

	/* Store into ME/TA and notify */
	atcmd = atcmd_fill("AT+CSBS=1", 9 + 1, NULL, gsmd, 0);
	if (!atcmd)
		return -ENOMEM;
	atcmd_submit(gsmd, atcmd);

	/* Store into ME/TA and notify */
	atcmd = atcmd_fill("AT+CSDS=2", 9 + 1, NULL, gsmd, 0);
	if (!atcmd)
		return -ENOMEM;
	atcmd_submit(gsmd, atcmd);

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
