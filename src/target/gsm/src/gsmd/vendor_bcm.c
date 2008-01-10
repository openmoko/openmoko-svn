/* Broadcom [BCM2132] gsmd plugin
 *
 * Written by Alex Osborne <bobofdoom@gmail.com>
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

static int mrdy_parse(const char *buf, int len, const char *param,
		     struct gsmd *g)
{
	int status = atoi(param);
	DEBUGP("entering mrdy_parse param=`%s'\n", param);

	switch (status) {
	case 1:
		DEBUGP("Module is ready.\n");
		break;
	case 2:
		DEBUGP("Emergency call is ready.\n");
		break;
	case 3:
		DEBUGP("All AT commands are ready.\n");
		break;
	case 4:
		DEBUGP("SIM card inserted.\n");
		break;
	case 5:
		DEBUGP("SIM card removed.\n");
		break;
	case 6:
		DEBUGP("No access - limited service.\n");
		break;
	case 7:
		DEBUGP("SOS - limited service.\n");
		break;
	default:
		DEBUGP("Unknown module ready status %d\n", status);
	}

	return 0;
}

static int mtsmenu_parse(const char *buf, int len, const char *param,
		     struct gsmd *g)
{

	DEBUGP("mtsmenu_parse param=`%s'\n", param);
	return 0;
}

static const struct gsmd_unsolicit bcm_unsolicit[] = {
	{ "*MRDY",	&mrdy_parse },		/* Module Ready Status */
	{ "*MTSMENU",	&mtsmenu_parse },	/* Set Up Menu (SAT) */

	/* FIXME: determine other unsolicited responses */
};

static int bcm_detect(struct gsmd *g)
{
	/* FIXME: do actual detection of vendor if we have multiple vendors */
	return 1;
}

static int bcm_initsettings(struct gsmd *g)
{
	int rc = 0;

	/* bcm sometimes sends LFCR instead of CRLF (eg *MTSMENU message) */
	g->llp.flags |= LGSM_ATCMD_F_LFCR;

	/* TODO */
	return rc;
}

struct gsmd_vendor_plugin gsmd_vendor_plugin = {
	.name = "Broadcom BCM2132",
	.ext_chars = "*",
	.num_unsolicit = ARRAY_SIZE(bcm_unsolicit),
	.unsolicit = bcm_unsolicit,
	.detect = &bcm_detect,
	.initsettings = &bcm_initsettings,
};
