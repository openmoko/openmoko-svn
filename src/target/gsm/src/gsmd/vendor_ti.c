/* TI [Calypso] compatible gsmd plugin
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
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/usock.h>
#include <gsmd/event.h>
#include <gsmd/talloc.h>
#include <gsmd/extrsp.h>
#include <gsmd/atcmd.h>
#include <gsmd/vendorplugin.h>
#include <gsmd/unsolicited.h>

#if 0
#include "vendorplugin.h"

static int 
ti_getopt(struct gsmd *gh, int optname, void *optval, int *optlen)
{
	switch (optname) {
	case GSMD_OPT_CIPHER_IND:
		/* FIXME: send AT%CPRI=? */
		break;
	default:
		return -EINVAL;
	}
}

static int 
ti_setopt(struct gsmd *gh, int optname, const void *optval, int optlen)
{
	switch (optname) {
	case GSMD_OPT_CIPHER_IND:
		/* FIXME: send AT%CPRI= */
		break;
	default:
		return -EINVAL;
	}
}

#endif


static int csq_parse(char *buf, int len, const char *param,
		     struct gsmd *gsmd)
{
	char *tok;
	struct gsmd_evt_auxdata *aux;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_SIGNAL,
					     sizeof(*aux));

	DEBUGP("entering csq_parse param=`%s'\n", param);
	if (!ucmd)
		return -EINVAL;
	
	
	aux = (struct gsmd_evt_auxdata *) ucmd->buf;
	tok = strtok(param, ",");
	if (!tok)
		goto out_free_io;
	
	aux->u.signal.sigq.rssi = atoi(tok);

	tok = strtok(NULL, ",");
	if (!tok)
		goto out_free_io;

	aux->u.signal.sigq.ber = atoi(tok);

	usock_evt_send(gsmd, ucmd, GSMD_EVT_SIGNAL);

	return 0;

out_free_io:
	free(ucmd);
	return -EIO;
}

static int cpri_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{
	char *tok1, *tok2;

	tok1 = strtok(buf, ",");
	if (!tok1)
		return -EIO;
	
	tok2 = strtok(NULL, ",");
	if (!tok2) {
		switch (atoi(tok1)) {
		case 0:
			gsmd->dev_state.ciph_ind.flags &= ~GSMD_CIPHIND_ACTIVE;
			break;
		case 1:
			gsmd->dev_state.ciph_ind.flags |= GSMD_CIPHIND_ACTIVE;
			break;
		case 2:
			gsmd->dev_state.ciph_ind.flags |= GSMD_CIPHIND_DISABLED_SIM;
			break;
		}
	} else {
		struct gsmd_evt_auxdata *aux;
		struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT,
							   GSMD_EVT_CIPHER,
							   sizeof(*aux));
		if (!ucmd)
			return -ENOMEM;

		aux = (struct gsmd_evt_auxdata *) ucmd->buf;

		aux->u.cipher.net_state_gsm = atoi(tok1);
		aux->u.cipher.net_state_gsm = atoi(tok2);

		usock_evt_send(gsmd, ucmd, GSMD_EVT_CIPHER);
	}

	return 0;
}

static int ctzv_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{
	/* FIXME: decide what to do with it.  send as event to clients? or keep
	 * locally? Offer option to sync system RTC? */
	return 0;
}

/* Call Progress Information */
static int cpi_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{
	char *tok;
	struct gsmd_evt_auxdata *aux;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT,
						   GSMD_EVT_OUT_STATUS,
						   sizeof(*aux));
	
	DEBUGP("entering cpi_parse param=`%s'\n", param);
	if (!ucmd)
		return -EINVAL;
	
	aux = (struct gsmd_evt_auxdata *) ucmd->buf;

	/* Format: cId, msgType, ibt, tch, dir,[mode],[number],[type],[alpha],[cause],line */

	/* call ID */
	tok = strtok(buf, ",");
	if (!tok)
		goto out_free_io;

	/* message type (layer 3) */
	tok = strtok(NULL, ",");
	if (!tok)
		goto out_free_io;
	aux->u.call_status.prog = atoi(tok);

	/* in-band tones */
	tok = strtok(NULL, ",");
	if (!tok)
		goto out_free_io;

	if (*tok == '1')
		aux->u.call_status.ibt = 1;
	else
		aux->u.call_status.ibt = 0;
	
	/* TCH allocated */
	tok = strtok(NULL, ",");
	if (!tok)
		goto out_free_io;

	if (*tok == '1')
		aux->u.call_status.tch = 1;
	else
		aux->u.call_status.tch = 0;
	
	/* direction */
	tok = strtok(NULL, ",");
	if (!tok)
		goto out_send;
	
	switch (*tok) {
	case '0':
	case '1':
	case '2':
	case '3':
		aux->u.call_status.dir = (*tok - '0');
		break;
	default:
		break;
	}

	/* mode */
	tok = strtok(NULL, ",");
	if (!tok)
		goto out_send;
	
out_send:
	usock_evt_send(gsmd, ucmd, GSMD_EVT_OUT_STATUS);

	return 0;

out_free_io:
	talloc_free(ucmd);
	return -EIO;
}

static const struct gsmd_unsolicit ticalypso_unsolicit[] = {
	{ "%CSQ",	&csq_parse },	/* Signal Quality */
	{ "%CPRI",	&cpri_parse },	/* Ciphering Indication */
	{ "%CPI",	&cpi_parse },	/* Call Progress Information */
	{ "%CTZV",	&ctzv_parse },	/* network time and data */

	/* FIXME: parse all the below and generate the respective events */

	/* %CPROAM: CPHS Home Country Roaming Indicator */
	/* %CPVWI: CPHS Voice Message Waiting */
	/* %CGREG: reports extended information about GPRS registration state */
	/* %CNIV: reports network name information */
	/* %CPKY: Press Key */
	/* %CMGRS: Message Retransmission Service */
	/* %CGEV: reports GPRS network events */
};

static int cpmb_detect_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd *g = ctx;
	struct gsm_extrsp *er;
	int rc;
	char atcmd_buf[20];

	if (strncmp(resp, "%CPMB: ", 7))
		return -EINVAL;
	resp += 7;
	
	er = extrsp_parse(cmd, resp);
	if (!er)
		return -ENOMEM;

	extrsp_dump(er);

	if (er->num_tokens == 5 &&
	    er->tokens[2].type == GSMD_ECMD_RTT_STRING &&
		er->tokens[3].type == GSMD_ECMD_RTT_NUMERIC &&
		er->tokens[4].type == GSMD_ECMD_RTT_STRING)
		rc = sprintf(atcmd_buf, "AT+CSVM=1,\"%s\",%d", 
			er->tokens[2].u.string, er->tokens[3].u.numeric);

	if(rc)
		return gsmd_simplecmd(g, atcmd_buf);

	talloc_free(er);

	return rc;
}

static int cpi_detect_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	struct gsmd *g = ctx;
	struct gsm_extrsp *er;

	if (strncmp(resp, "%CPI: ", 6))
		return -EINVAL;
	resp += 6;
	
	er = extrsp_parse(cmd, resp);
	if (!er)
		return -EINVAL;
	
	/* retrieve voicemail number */
	cmd = atcmd_fill("AT%CPMB=1", 10, &cpmb_detect_cb, g, 0);
	if (cmd)
		atcmd_submit(g, cmd);
	
	if (extrsp_supports(er, 0, 3))
		return gsmd_simplecmd(g, "AT%CPI=3");
	else if (extrsp_supports(er, 0, 2))
		return gsmd_simplecmd(g, "AT%CPI=2");
	else
		DEBUGP("Call Progress Indication mode 2 or 3 not supported!!\n");

	talloc_free(er);
	return 0;
}

static int ticalypso_detect(struct gsmd *g)
{
	/* FIXME: do actual detection of vendor if we have multiple vendors */
	return 1;
}

static int ticalypso_initsettings(struct gsmd *g)
{
	int rc = 0;
	struct gsmd_atcmd *cmd;

	/* use +CTZR: to report time zone changes */
	rc |= gsmd_simplecmd(g, "AT+CTZR=1");
	/* use %CTZV: Report time and date */
	rc |= gsmd_simplecmd(g, "AT%CTZV=1");
	/* use %CGREG */
	//rc |= gsmd_simplecmd(g, "AT%CGREG=3");
	/* enable %CPRI: ciphering indications */
	rc |= gsmd_simplecmd(g, "AT%CPRI=1");
	/* enable %CSQ: signal quality reports */
	rc |= gsmd_simplecmd(g, "AT%CSQ=1");
	/* send unsolicited commands at any time */
	rc |= gsmd_simplecmd(g, "AT%CUNS=0");
	/* enable %CPHS: Initialise CPHS Functionalities */
	rc |= gsmd_simplecmd(g, "AT%CPHS=1");
	
	/* enable %CPI: call progress indication */
	cmd = atcmd_fill("AT%CPI=?", 9, &cpi_detect_cb, g, 0);
	if (cmd)
		atcmd_submit(g, cmd);

	return rc;
}

struct gsmd_vendor_plugin gsmd_vendor_plugin = {
	.name = "TI Calypso",
	.ext_chars = "%@",
	.num_unsolicit = ARRAY_SIZE(ticalypso_unsolicit),
	.unsolicit = ticalypso_unsolicit,
	.detect = &ticalypso_detect,
	.initsettings = &ticalypso_initsettings,
};
