/* Telit GM862 gsmd plugin
 *
 * (c) 2008 Florian Boor <florian@kernelconcepts.de>
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

static int telit_detect(struct gsmd *g)
{
	return 1; /* not yet implemented */
}

static int telit_initsettings(struct gsmd *g)
{
	int rc = 0;

	/* Get network registration events with full information. */
	rc |= gsmd_simplecmd(g, "AT+CGREG=2");
	/* Turn on CLIP. */
	rc |= gsmd_simplecmd(g, "AT+CLIP=1");
	/* Enable powersaving. */
	rc |= gsmd_simplecmd(g, "AT+CFUN=5");
	/* enable signal quality reports */
//	rc |= gsmd_simplecmd(g, "AT%HTCCSQ=1");

	return rc;
}

static int csq_parse(const char *buf, int len, const char *param,
		     struct gsmd *gsmd)
{
	struct gsmd_evt_auxdata *aux;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT,
			GSMD_EVT_SIGNAL, sizeof(*aux));

	DEBUGP("entering csq_parse param=`%s'\n", param);
	if (!ucmd)
		return -EINVAL;

	aux = (struct gsmd_evt_auxdata *) ucmd->buf;
	if (sscanf(param, " %hhi, %hhi",
				&aux->u.signal.sigq.rssi,
				&aux->u.signal.sigq.ber) < 2)
		goto out_free_io;

	usock_evt_send(gsmd, ucmd, GSMD_EVT_SIGNAL);
	return 0;

out_free_io:
	talloc_free(ucmd);
	return -EIO;
}

static const struct gsmd_unsolicit telit_unsolicit[] = {
	{ "+CSQ",	&csq_parse },	/* Signal Quality */
};

struct gsmd_vendor_plugin gsmd_vendor_plugin = {
	.name = "Telit GM862",
	.ext_chars = "%#$",
	.num_unsolicit = ARRAY_SIZE(telit_unsolicit),
	.unsolicit = telit_unsolicit,
	.detect = &telit_detect,
	.initsettings = &telit_initsettings,
};
