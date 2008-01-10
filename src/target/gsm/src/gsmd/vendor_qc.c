/* Qualcomm [msm6250] gsmd plugin
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

static int htccsq_parse(const char *buf, int len, const char *param,
		     struct gsmd *gsmd)
{
	struct gsmd_evt_auxdata *aux;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_SIGNAL,
					     sizeof(*aux));
	static int rssi_table[] = { 0,5,10,15,20,25,99 }; /* FIXME */
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

static int wcdma_parse(const char *buf, int len, const char *param,
		     struct gsmd *gsmd)
{
	return 0;
}

static const struct gsmd_unsolicit qc_unsolicit[] = {
	{ "@HTCCSQ",	&htccsq_parse },	/* Signal Quality */
	{ "[WCDMA]",	&wcdma_parse },		/* ignore [WCDMA] messages */

	/* FIXME: parse the below and generate the respective events */

	/* %CGREG: reports extended information about GPRS registration state */
};

static int qc_detect(struct gsmd *g)
{
	/* FIXME: do actual detection of vendor if we have multiple vendors */
	/* open /proc/cpuinfo and check for HTC Universal? */

	/* The Qualcomm chip switches to V0 mode in the strangest places */
	g->flags |= GSMD_FLAG_V0;
	return 1;
}

static int qc_initsettings(struct gsmd *g)
{
	int rc = 0;

	/* enable @HTCCSQ: signal quality reports */
	rc |= gsmd_simplecmd(g, "AT@HTCCSQ=1");

	return rc;
}

struct gsmd_vendor_plugin gsmd_vendor_plugin = {
	.name = "Qualcomm msm6250",
	.ext_chars = "@[",
	.num_unsolicit = ARRAY_SIZE(qc_unsolicit),
	.unsolicit = qc_unsolicit,
	.detect = &qc_detect,
	.initsettings = &qc_initsettings,
};
