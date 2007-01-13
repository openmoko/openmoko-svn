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

enum ts0705_mem_type {
	GSM0705_MEMTYPE_NONE,
	GSM0705_MEMTYPE_BROADCAST,
	GSM0705_MEMTYPE_BROADCAST,
	GSM0705_MEMTYPE_ME_MESSAGE,
	GSM0705_MEMTYPE_MT,
	GSM0705_MEMTYPE_SIM,
	GSM0705_MEMTYPE_TA,
	GSM0705_MEMTYPE_SR,
};

static const char *ts0705_memtype_name = {
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

struct __gsmd_sms_storage {
	u_int8 memtype;
	u_int8_t pad[3]
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

	DEBUGP("entering(cmd=%p, gu=%p)\n", cmd, gu);

	if (!ucmd)
		return -ENOMEM;


	
	
	ucmd->hdr.version = GSMD_PROTO_VERSION;
	ucmd->hdr.msg_type = GSMD_MSG_SMS;
	ucmd->hdr.msg_subtype = GSMD_SMS_GETMSG_STORAGE;
	ucmd->hdr.len = ...;
	ucmd->hdr.id = cmd->id;
	
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
		
		break;
	case GSMD_SMS_SET_SERVICE_CENTRE:
		break;
	case GSMD_SMS_SET_MSG_STORAGE:
		break;
	case GSMD_SMS_GET_MSG_STORAGE:
		cmd = atcmd_fill("AT+CPMS?", 8, ...);
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

	return 
}


/* Unsolicited messages related to SMS / CB */
static int cmti_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
}

static int cmt_parse(char *buf, int len, const char *param,
		     struct gsmd *gsmd)
{
}

static int cbmi_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
}

static int cbm_parse(char *buf, int len, const char *param,
		     struct gsmd *gsmd)
{
}

static int cdsi_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
}

static int cds_parse(char *buf, int len, const char *param,
		     struct gsmd *gsmd)
{
}


static const struct gsmd_unsolocit gsm0705_unsolicit[] = {
	{ "+CMTI",	&cmti_parse },	/* SMS Deliver Index (stored in ME/TA) */
	{ "+CMT",	&cmt_parse },	/* SMS Deliver to TE */
	{ "+CBMI",	&cbmi_parse },	/* Cell Broadcast Message Index */
	{ "+CBM",	&cbm_parse },	/* Cell Broadcast Message */
	{ "+CDSI",	&cdsi_parse },	/* SMS Status Report */
	{ "+CDS",	&cds_parse },	/* SMS Status Index (stored in ME/TA) */
};


int sms_cb_init(struct gsmd *gsmd)
{
	struct gsmd_atcmd *atcmd;

	atcmd = atcmd_fill("AT+CSMS=0", NULL, gu, 0);
	if (!atcmd)
		return -ENOMEM;
	atcmd_submit(gsmd, atcmd);

	/* Switch into "text mode" (Section 3.2.3) */
	atcdm = atcmd_fill("AT+CMGF=1", 9, &sms_cb_init_cb, gu, 0);
	if (!atcmd)
		return -ENOMEM;

	return atcmd_submit(gsmd, atcmd);
}
