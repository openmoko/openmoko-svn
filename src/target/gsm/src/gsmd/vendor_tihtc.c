/* TI [Calypso] with HTC firmware gsmd plugin
 *
 * Written by Philipp Zabel <philipp.zabel@gmail.com>
 * based on vendor_ti.c
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

static int gsmd_test_atcb(struct gsmd_atcmd *cmd, void *ctx, char *resp)
{
	printf("`%s' returned `%s'\n", cmd->buf, resp);
	return 0;
}

int gsmd_simplecmd(struct gsmd *gsmd, char *cmdtxt)
{
	struct gsmd_atcmd *cmd;
	cmd = atcmd_fill(cmdtxt, strlen(cmdtxt)+1, &gsmd_test_atcb, NULL, 0, NULL);
	if (!cmd)
		return -ENOMEM;

	return atcmd_submit(gsmd, cmd);
}

static int htccsq_parse(const char *buf, int len, const char *param,
		     struct gsmd *gsmd)
{
	struct gsmd_evt_auxdata *aux;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_SIGNAL,
					     sizeof(*aux));
	static int rssi_table[] = { 0,5,10,15,20,25,99 };
	unsigned int i;

	DEBUGP("entering htccsq_parse param=`%s'\n", param);
	if (!ucmd)
		return -EINVAL;


	aux = (struct gsmd_evt_auxdata *) ucmd->buf;

	i = atoi(buf);
	if (i > 6)
		i = 6;
	aux->u.signal.sigq.rssi = rssi_table[atoi(buf)];
	aux->u.signal.sigq.ber = 99;

	DEBUGP("sending EVT_SIGNAL\n");
	usock_evt_send(gsmd, ucmd, GSMD_EVT_SIGNAL);

	return 0;
}

static int cpri_parse(const char *buf, int len, const char *param, struct gsmd *gsmd)
{
	char *tok1, *tok2;
	char tx_buf[20];
	
	strlcpy(tx_buf, buf, sizeof(tx_buf));
	tok1 = strtok(tx_buf, ",");
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

/* Call Progress Information */
static int cpi_parse(const char *buf, int len, const char *param, struct gsmd *gsmd)
{
	char *tok;
	struct gsmd_evt_auxdata *aux;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT,
						   GSMD_EVT_OUT_STATUS,
						   sizeof(*aux));
	char tx_buf[64];

	strlcpy(tx_buf, buf, sizeof(tx_buf));
	DEBUGP("entering cpi_parse param=`%s'\n", param);
	if (!ucmd)
		return -EINVAL;

	aux = (struct gsmd_evt_auxdata *) ucmd->buf;

	/* Format: cId, msgType, ibt, tch, dir,[mode],[number],[type],[alpha],[cause],line */

	/* call ID */
	tok = strtok(tx_buf, ",");
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

static const struct gsmd_unsolicit tihtc_unsolicit[] = {
	{ "%HTCCSQ",	&htccsq_parse },	/* Signal Quality */
	{ "%CPRI",	&cpri_parse },	/* Ciphering Indication */
	{ "%CPI",	&cpi_parse },	/* Call Progress Information */

	/* FIXME: parse the below and generate the respective events */

	/* %CGREG: reports extended information about GPRS registration state */
};

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

	if (extrsp_supports(er, 0, 3))
		return gsmd_simplecmd(g, "AT%CPI=3");
	else if (extrsp_supports(er, 0, 2))
		return gsmd_simplecmd(g, "AT%CPI=2");
	else
		DEBUGP("Call Progress Indication mode 2 or 3 not supported!!\n");

	talloc_free(er);
	return 0;
}

static int tihtc_detect(struct gsmd *g)
{
	/* FIXME: do actual detection of vendor if we have multiple vendors */
	/* open /proc/cpuinfo and check for HTC Magician or HTC Blueangel? */
	/* check for N_TIHTC ldisc? or set it ourselves? */
	return 1;
}

static int tihtc_initsettings(struct gsmd *g)
{
	int rc = 0;
	struct gsmd_atcmd *cmd;

	/* use %CGREG */
	//rc |= gsmd_simplecmd(g, "AT%CGREG=3");
	/* enable %CPRI: ciphering indications */
	rc |= gsmd_simplecmd(g, "AT%CPRI=1");
	/* enable %HTCCSQ: signal quality reports */
	rc |= gsmd_simplecmd(g, "AT%HTCCSQ=1");
	/* send unsolicited commands at any time */
	rc |= gsmd_simplecmd(g, "AT%CUNS=0");

	/* enable %CPI: call progress indication */
	cmd = atcmd_fill("AT%CPI=?", 9, &cpi_detect_cb, g, 0, NULL);
	if (cmd)
		atcmd_submit(g, cmd);

	return rc;
}

struct gsmd_vendor_plugin gsmd_vendor_plugin = {
	.name = "TI Calypso / HTC firmware",
	.ext_chars = "%",
	.num_unsolicit = ARRAY_SIZE(tihtc_unsolicit),
	.unsolicit = tihtc_unsolicit,
	.detect = &tihtc_detect,
	.initsettings = &tihtc_initsettings,
};
