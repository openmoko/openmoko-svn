/* gsmd unsolicited message handling
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
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "gsmd.h"

#include <gsmd/usock.h>
#include <gsmd/event.h>
#include <gsmd/extrsp.h>
#include <gsmd/ts0707.h>
#include <gsmd/unsolicited.h>
#include <gsmd/talloc.h>

struct gsmd_ucmd *usock_build_event(u_int8_t type, u_int8_t subtype, u_int16_t len)
{
	struct gsmd_ucmd *ucmd = ucmd_alloc(len);

	if (!ucmd)
		return NULL;

	ucmd->hdr.version = GSMD_PROTO_VERSION;
	ucmd->hdr.msg_type = type;
	ucmd->hdr.msg_subtype = subtype;
	ucmd->hdr.len = len;

	return ucmd;
}

static struct gsmd_ucmd *ucmd_copy(const struct gsmd_ucmd *orig)
{
	struct gsmd_ucmd *copy = ucmd_alloc(orig->hdr.len);

	if (copy)
		memcpy(copy, orig, orig->hdr.len);

	return copy;
}

int usock_evt_send(struct gsmd *gsmd, struct gsmd_ucmd *ucmd, u_int32_t evt)
{
	struct gsmd_user *gu;
	int num_sent = 0;

	DEBUGP("entering evt=%u\n", evt);

	llist_for_each_entry(gu, &gsmd->users, list) {
		if (gu->subscriptions & (1 << evt)) {
			if (num_sent == 0)
				usock_cmd_enqueue(ucmd, gu);
			else {
				struct gsmd_ucmd *cpy = ucmd_copy(ucmd);
				if (!cpy) {
					fprintf(stderr, 
						"can't allocate memory for "
						"copy of ucmd\n");
					return num_sent;
				}
				usock_cmd_enqueue(cpy, gu);
			}
			num_sent++;
		}
	}

	if (num_sent == 0)
		talloc_free(ucmd);

	return num_sent;
}

static int ring_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	/* FIXME: generate ring event */
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_IN_CALL,
					     sizeof(struct gsmd_evt_auxdata));	
	struct gsmd_evt_auxdata *aux;
	if (!ucmd)
		return -ENOMEM;

	aux = (struct gsmd_evt_auxdata *)ucmd->buf;

	aux->u.call.type = GSMD_CALL_UNSPEC;

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_IN_CALL);
}

static int cring_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_IN_CALL,
					     sizeof(struct gsmd_evt_auxdata));
	struct gsmd_evt_auxdata *aux;
	if (!ucmd)
		return -ENOMEM;

	aux = (struct gsmd_evt_auxdata *) ucmd->buf;

	if (!strcmp(param, "VOICE")) {
		/* incoming voice call */
		aux->u.call.type = GSMD_CALL_VOICE;
	} else if (!strcmp(param, "SYNC")) {
		aux->u.call.type = GSMD_CALL_DATA_SYNC;
	} else if (!strcmp(param, "REL ASYNC")) {
		aux->u.call.type = GSMD_CALL_DATA_REL_ASYNC;
	} else if (!strcmp(param, "REL SYNC")) {
		aux->u.call.type = GSMD_CALL_DATA_REL_SYNC;
	} else if (!strcmp(param, "FAX")) {
		aux->u.call.type = GSMD_CALL_FAX;
	} else if (!strncmp(param, "GPRS ", 5)) {
		/* FIXME: change event type to GPRS */
		talloc_free(ucmd);
		return 0;
	}
	/* FIXME: parse all the ALT* profiles, Chapter 6.11 */

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_IN_CALL);
}

/* Chapter 7.2, network registration */
static int creg_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	const char *comma = strchr(param, ',');
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_NETREG,
					     sizeof(struct gsmd_evt_auxdata));
	struct gsmd_evt_auxdata *aux;
	if (!ucmd)
		return -ENOMEM;
	aux = (struct gsmd_evt_auxdata *) ucmd->buf;

	aux->u.netreg.state = atoi(param);
	if (comma) {
		/* we also have location area code and cell id to parse (hex) */
		aux->u.netreg.lac = strtoul(comma+2, NULL, 16);
		comma = strchr(comma+1, ',');
		if (!comma)
			return -EINVAL;
		aux->u.netreg.ci = strtoul(comma+2, NULL, 16);
	} else
		aux->u.netreg.lac = aux->u.netreg.ci = 0;

	/* Intialise things that depend on network registration */
	if (aux->u.netreg.state == GSMD_NETREG_REG_HOME ||
			aux->u.netreg.state == GSMD_NETREG_REG_ROAMING) {
		sms_cb_network_init(gsmd);
	}

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_NETREG);
}

/* Chapter 7.11, call waiting */
static int ccwa_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	const char *token;
	unsigned int type;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_CALL_WAIT,
					     sizeof(struct gsmd_addr));
	struct gsmd_addr *gaddr;

	if (!ucmd)
		return -ENOMEM;

	gaddr = (struct gsmd_addr *) ucmd->buf;
	memset(gaddr, 0, sizeof(*gaddr));

	/* parse address (phone number) */
	token = strtok(buf, ",");
	if (!token)
		return -EINVAL;
	strncpy(gaddr->number, token, GSMD_ADDR_MAXLEN);

	/* parse type */
	token = strtok(NULL, ",");
	if (!token)
		return -EINVAL;
	type = atoi(token) & 0xff;
	gaddr->type = type;

	/* FIXME: parse class */
	token = strtok(NULL, ",");

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_CALL_WAIT);
}

/* Chapter 7.14, unstructured supplementary service data */
static int cusd_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	/* FIXME: parse */
	return 0;
}

/* Chapter 7.15, advise of charge */
static int cccm_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	/* FIXME: parse */
	return 0;
}

/* Chapter 10.1.13, GPRS event reporting */
static int cgev_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	/* FIXME: parse */
	return 0;
}

/* Chapter 10.1.14, GPRS network registration status */
static int cgreg_parse(char *buf, int len, const char *param,
		       struct gsmd *gsmd)
{
	/* FIXME: parse */
	return 0;
}

/* Chapter 7.6, calling line identification presentation */
static int clip_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_IN_CLIP,
					     sizeof(struct gsmd_evt_auxdata));
	struct gsmd_evt_auxdata *aux;
	const char *comma = strchr(param, ',');

	if (!ucmd)
		return -ENOMEM;
	
	aux = (struct gsmd_evt_auxdata *) ucmd->buf;

	if (!comma)
		return -EINVAL;

	
	if (comma - param > GSMD_ADDR_MAXLEN)
		return -EINVAL;

	aux->u.clip.addr.number[0] = '\0';
	strncat(aux->u.clip.addr.number, param, comma-param);
	/* FIXME: parse of subaddr, etc. */

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_IN_CLIP);
}

/* Chapter 7.9, calling line identification presentation */
static int colp_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_OUT_COLP,
					     sizeof(struct gsmd_evt_auxdata));
	struct gsmd_evt_auxdata *aux;
	const char *comma = strchr(param, ',');

	if (!ucmd)
		return -ENOMEM;
	
	aux = (struct gsmd_evt_auxdata *) ucmd->buf;

	if (!comma)
		return -EINVAL;
	
	if (comma - param > GSMD_ADDR_MAXLEN)
		return -EINVAL;

	aux->u.colp.addr.number[0] = '\0';
	strncat(aux->u.colp.addr.number, param, comma-param);
	/* FIXME: parse of subaddr, etc. */

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_OUT_COLP);
}

static int ctzv_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_TIMEZONE,
					     sizeof(struct gsmd_evt_auxdata));
	struct gsmd_evt_auxdata *aux;
	int tz;

	if (!ucmd)
		return -ENOMEM;
	
	aux = (struct gsmd_evt_auxdata *) ucmd->buf;

	/* timezones are expressed in quarters of hours +/- GMT (-48...+48) */
	tz = atoi(param);

	if (tz < -48  || tz > 48)
		return -EINVAL;
	
	aux->u.timezone.tz = tz;

	return usock_evt_send(gsmd, ucmd, GSMD_EVT_TIMEZONE);
}

static int copn_parse(char *buf, int len, const char *param,
		      struct gsmd *gsmd)
{
	struct gsm_extrsp *er = extrsp_parse(gsmd_tallocs, param);
	int rc = 0;

	if (!er)
		return -ENOMEM;

	extrsp_dump(er);

	if (er->num_tokens == 2 &&
	    er->tokens[0].type == GSMD_ECMD_RTT_STRING &&
	    er->tokens[1].type == GSMD_ECMD_RTT_STRING)
		rc = gsmd_opname_add(gsmd, er->tokens[0].u.string,
				     er->tokens[1].u.string);

	talloc_free(er);

	return rc;
}

static const struct gsmd_unsolicit gsm0707_unsolicit[] = {
	{ "RING",	&ring_parse },
	{ "+CRING", 	&cring_parse },
	{ "+CREG",	&creg_parse },	/* Network registration */
	{ "+CCWA",	&ccwa_parse },	/* Call waiting */
	{ "+CUSD",	&cusd_parse },	/* Unstructured supplementary data */
	{ "+CCCM",	&cccm_parse },	/* Advice of Charge */
	{ "+CGEV",	&cgev_parse },	/* GPRS Event */
	{ "+CGREG",	&cgreg_parse },	/* GPRS Registration */
	{ "+CLIP",	&clip_parse },
	{ "+COLP",	&colp_parse },
	{ "+CTZV",	&ctzv_parse },	/* Timezone */
	{ "+COPN",	&copn_parse },  /* operator names, treat as unsolicited */
	/*
	{ "+CKEV",	&ckev_parse },
	{ "+CDEV",	&cdev_parse },
	{ "+CIEV",	&ciev_parse },
	{ "+CLAV",	&clav_parse },
	{ "+CCWV",	&ccwv_parse },
	{ "+CLAV",	&clav_parse },
	{ "+CSSU",	&cssu_parse },
	*/
};

static struct gsmd_unsolicit unsolicit[256] = {{ 0, 0 }};

/* called by midlevel parser if a response seems unsolicited */
int unsolicited_parse(struct gsmd *g, char *buf, int len, const char *param)
{
	struct gsmd_unsolicit *i;
	int rc;
	struct gsmd_vendor_plugin *vpl = g->vendorpl;

	/* call unsolicited code parser */
	for (i = unsolicit; i->prefix; i ++) {
		const char *colon;
		if (strncmp(buf, i->prefix, strlen(i->prefix)))
			continue;

		colon = strchr(buf, ':') + 2;
		if (colon > buf+len)
			colon = NULL;

		rc = i->parse(buf, len, colon, g);
		if (rc == -EAGAIN)
			return rc;
		if (rc < 0) 
			gsmd_log(GSMD_ERROR, "error %d during parsing of "
				 "an unsolicied response `%s'\n",
				 rc, buf);
			return rc;
	}

	gsmd_log(GSMD_NOTICE, "no parser for unsolicited response `%s'\n", buf);

	return -ENOENT;
}

int unsolicited_register_array(const struct gsmd_unsolicit *arr, int len)
{
	int curlen = 0;

	while (unsolicit[curlen ++].prefix);
	if (len + curlen > ARRAY_SIZE(unsolicit))
		return -ENOMEM;

	/* Add at the beginning for overriding to be possible */
	memmove(&unsolicit[len], unsolicit,
			sizeof(struct gsmd_unsolicit) * curlen);
	memcpy(unsolicit, arr,
			sizeof(struct gsmd_unsolicit) * len);

	return 0;
}

void unsolicited_init(struct gsmd *g)
{
	struct gsmd_vendor_plugin *vpl = g->vendorpl;

	/* register generic unsolicited code parser */
	unsolicited_register_array(gsm0707_unsolicit,
			ARRAY_SIZE(gsm0707_unsolicit));

	/* register vendor-specific unsolicited code parser */
	if (vpl && vpl->num_unsolicit)
		if (unsolicited_register_array(vpl->unsolicit,
					vpl->num_unsolicit))
			gsmd_log(GSMD_ERROR, "registering vendor-specific "
					"unsolicited responses failed\n");
}

static unsigned int errors_creating_events[] = {
	GSM0707_CME_PHONE_FAILURE,
	GSM0707_CME_PHONE_NOCONNECT,
	GSM0707_CME_PHONE_ADAPT_RESERVED,
	GSM0707_CME_PH_SIM_PIN_REQUIRED,
	GSM0707_CME_PH_FSIM_PIN_REQUIRED,
	GSM0707_CME_PH_FSIM_PUK_REQUIRED,
	GSM0707_CME_SIM_NOT_INSERTED,
	GSM0707_CME_SIM_PIN_REQUIRED,
	GSM0707_CME_SIM_PUK_REQUIRED,
/*	GSM0707_CME_SIM_FAILURE,
	GSM0707_CME_SIM_BUSY,
	GSM0707_CME_SIM_WRONG,*/
	GSM0707_CME_SIM_PIN2_REQUIRED,
	GSM0707_CME_SIM_PUK2_REQUIRED,
/*	GSM0707_CME_MEMORY_FULL,
	GSM0707_CME_MEMORY_FAILURE,*/    
	GSM0707_CME_NETPERS_PIN_REQUIRED,
	GSM0707_CME_NETPERS_PUK_REQUIRED,
	GSM0707_CME_NETSUBSET_PIN_REQUIRED,
	GSM0707_CME_NETSUBSET_PUK_REQUIRED,
	GSM0707_CME_PROVIDER_PIN_REQUIRED,
	GSM0707_CME_PROVIDER_PUK_REQUIRED,
	GSM0707_CME_CORPORATE_PIN_REQUIRED,
	GSM0707_CME_CORPORATE_PUK_REQUIRED,
};

static int is_in_array(unsigned int val, unsigned int *arr, unsigned int arr_len)
{
	unsigned int i;

	for (i = 0; i < arr_len; i++) {
		if (arr[i] == val)
			return 1;
	}

	return 0;
}

int generate_event_from_cme(struct gsmd *g, unsigned int cme_error)
{
	struct gsmd_ucmd *gu;
	struct gsmd_evt_auxdata *eaux;

 	if (!is_in_array(cme_error, errors_creating_events,
		ARRAY_SIZE(errors_creating_events))) {

		gu = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_IN_ERROR, sizeof(*eaux));
		if (!gu)
			return -1;
		eaux = ((void *)gu) + sizeof(*gu);
		eaux->u.cme_err.number = cme_error;
		return usock_evt_send(g, gu, GSMD_EVT_IN_ERROR);
 	}
	else {
		gu = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_PIN, sizeof(*eaux));
		if (!gu)
			return -1;
		eaux = ((void *)gu) + sizeof(*gu);
	
		switch (cme_error) {
		case GSM0707_CME_PH_SIM_PIN_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_PH_SIM_PIN;
			break;
		case GSM0707_CME_PH_FSIM_PIN_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_PH_FSIM_PIN;
			break;
		case GSM0707_CME_PH_FSIM_PUK_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_PH_FSIM_PUK;
			break;
		case GSM0707_CME_SIM_PIN_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_SIM_PIN;
			break;
		case GSM0707_CME_SIM_PUK_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_SIM_PUK;
			break;
		case GSM0707_CME_SIM_PIN2_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_SIM_PIN2;
			break;
		case GSM0707_CME_SIM_PUK2_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_SIM_PUK2;
			break;
		case GSM0707_CME_NETPERS_PIN_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_PH_NET_PIN;
			break;
		case GSM0707_CME_NETPERS_PUK_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_PH_NET_PUK;
			break;
		case GSM0707_CME_NETSUBSET_PIN_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_PH_NETSUB_PIN;
			break;
		case GSM0707_CME_NETSUBSET_PUK_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_PH_NETSUB_PUK;
			break;
		case GSM0707_CME_PROVIDER_PIN_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_PH_SP_PIN;
			break;
		case GSM0707_CME_PROVIDER_PUK_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_PH_SP_PUK;
			break;
		case GSM0707_CME_CORPORATE_PIN_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_PH_CORP_PIN;
			break;
		case GSM0707_CME_CORPORATE_PUK_REQUIRED:
			eaux->u.pin.type = GSMD_PIN_PH_CORP_PUK;
			break;
	
		case GSM0707_CME_SIM_FAILURE:
		case GSM0707_CME_SIM_BUSY:
		case GSM0707_CME_SIM_WRONG:
		case GSM0707_CME_MEMORY_FULL:
		case GSM0707_CME_MEMORY_FAILURE:
		case GSM0707_CME_PHONE_FAILURE:
		case GSM0707_CME_PHONE_NOCONNECT:
		case GSM0707_CME_PHONE_ADAPT_RESERVED:
		case GSM0707_CME_SIM_NOT_INSERTED:
			/* FIXME */
			talloc_free(gu);
			return 0;
			break;
		default:
			talloc_free(gu);
			return 0;
			break;
		}
		return usock_evt_send(g, gu, GSMD_EVT_PIN);
	}
}

int generate_event_from_cms(struct gsmd *g, unsigned int cms_error)
{
	struct gsmd_ucmd *gu;
	struct gsmd_evt_auxdata *eaux;
	
	gu = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_IN_ERROR, sizeof(*eaux));
	if (!gu)
		return -1;
	eaux = ((void *)gu) + sizeof(*gu);
	eaux->u.cms_err.number = cms_error;
	return usock_evt_send(g, gu, GSMD_EVT_IN_ERROR);
}

