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

static int insms_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	printf("EVENT: Incoming SMS stored at location %i\n", aux->u.sms.index);

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
	case GSMD_NETREG_UNREG:
		printf("not searching for network ");
		break;
	case GSMD_NETREG_REG_HOME:
		printf("registered (home network) ");
		break;
	case GSMD_NETREG_UNREG_BUSY:
		printf("searching for network ");
		break;
	case GSMD_NETREG_DENIED:
		printf("registration denied ");
		break;
	case GSMD_NETREG_REG_ROAMING:
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

static const char *cprog_names[] = {
	[GSMD_CALLPROG_SETUP]		= "SETUP",
	[GSMD_CALLPROG_DISCONNECT]	= "DISCONNECT",
	[GSMD_CALLPROG_ALERT]		= "ALERT",
	[GSMD_CALLPROG_CALL_PROCEED]	= "PROCEED",
	[GSMD_CALLPROG_SYNC]		= "SYNC",
	[GSMD_CALLPROG_PROGRESS]	= "PROGRESS",
	[GSMD_CALLPROG_CONNECTED]	= "CONNECTED",
	[GSMD_CALLPROG_RELEASE]		= "RELEASE",
	[GSMD_CALLPROG_REJECT]		= "REJECT",
	[GSMD_CALLPROG_UNKNOWN]		= "UNKNOWN",
};

static const char *cdir_names[] = {
	[GSMD_CALL_DIR_MO]		= "Outgoing",
	[GSMD_CALL_DIR_MT]		= "Incoming",
	[GSMD_CALL_DIR_CCBS]		= "CCBS",
	[GSMD_CALL_DIR_MO_REDIAL]	= "Outgoing Redial",
};

static int cprog_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	const char *name, *dir;

	if (aux->u.call_status.prog >= ARRAY_SIZE(cprog_names))
		name = "UNDEFINED";
	else
		name = cprog_names[aux->u.call_status.prog];

	if (aux->u.call_status.dir >= ARRAY_SIZE(cdir_names))
		dir = "";
	else
		dir = cdir_names[aux->u.call_status.dir];

	printf("EVENT: %s Call Progress: %s\n", dir, name);

	return 0;
}

int event_init(struct lgsm_handle *lh)
{
	int rc;

	rc  = lgsm_evt_handler_register(lh, GSMD_EVT_IN_CALL, &incall_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_IN_CLIP, &clip_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_IN_SMS, &insms_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_OUT_COLP, &colp_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_NETREG, &netreg_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_SIGNAL, &sigq_handler);
	rc |= lgsm_evt_handler_register(lh, GSMD_EVT_OUT_STATUS, &cprog_handler);

	return rc;
}

