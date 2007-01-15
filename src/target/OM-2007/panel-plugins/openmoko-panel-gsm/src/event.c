/* libgsmd tool
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

#include <stdio.h>
#include <string.h>

#include <common/linux_list.h>
#include <libgsmd/libgsmd.h>
#include <libgsmd/event.h>
static int incall_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Incoming call type = %u\n", aux->u.call.type);

	return 0;
}

static int clip_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Incoming call clip = %s\n", aux->u.clip.addr.number);

	return 0;
}

static int colp_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Outgoing call colp = %s\n", aux->u.colp.addr.number);

	return 0;
}

static int netreg_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Netreg ");

	switch (aux->u.netreg.state) {
	case 0:
		printf("not searching for network ");
		break;
	case 1:
		printf("registered (home network) ");
		break;
	case 2:
		printf("searching for network ");
		break;
	case 3:
		printf("registration denied ");
		break;
	case 5:
		printf("registered (roaming) ");
		break;
	}

	if (aux->u.netreg.lac)
		printf("LocationAreaCode = 0x%04X ", aux->u.netreg.lac);
	if (aux->u.netreg.ci)
		printf("CellID = 0x%04X ", aux->u.netreg.ci);
	
	printf("\n");

	return 0;
}


static int sigq_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Signal Quality: %u\n", aux->u.signal.sigq.rssi);
	return 0;
}

int event_init(struct lgsm_handle *lh)
{
	int rc;

	rc  = lgsm_evt_handler_register(lh, GSMD_EVT_IN_CALL, &incall_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_NETREG, &netreg_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_SIGNAL, &sigq_handler);

	return rc;
}

