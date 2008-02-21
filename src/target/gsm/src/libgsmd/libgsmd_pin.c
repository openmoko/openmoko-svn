/* libgsmd pin support
 *
 * (C) 2006-2007 by OpenMoko, Inc.
 * Written by Harald Welte <laforge@openmoko.org>
 * All Rights Reserved
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <gsmd/event.h>
#include <libgsmd/libgsmd.h>

#include "lgsm_internals.h"

static const char *pin_type_names[__NUM_GSMD_PIN] = {
	[GSMD_PIN_READY]	= "READY",
	[GSMD_PIN_SIM_PIN]	= "SIM PIN",
	[GSMD_PIN_SIM_PUK]	= "SIM PUK",
	[GSMD_PIN_PH_SIM_PIN]	= "Phone-to-SIM PIN",
	[GSMD_PIN_PH_FSIM_PIN]	= "Phone-to-very-first SIM PIN",
	[GSMD_PIN_PH_FSIM_PUK]	= "Phone-to-very-first SIM PUK",
	[GSMD_PIN_SIM_PIN2]	= "SIM PIN2",
	[GSMD_PIN_SIM_PUK2]	= "SIM PUK2",
	[GSMD_PIN_PH_NET_PIN]	= "Network personalization PIN",
	[GSMD_PIN_PH_NET_PUK]	= "Network personalizaiton PUK",
	[GSMD_PIN_PH_NETSUB_PIN]= "Network subset personalisation PIN",
	[GSMD_PIN_PH_NETSUB_PUK]= "Network subset personalisation PUK",
	[GSMD_PIN_PH_SP_PIN]	= "Service provider personalisation PIN",
	[GSMD_PIN_PH_SP_PUK]	= "Service provider personalisation PUK",
	[GSMD_PIN_PH_CORP_PIN]	= "Corporate personalisation PIN",
	[GSMD_PIN_PH_CORP_PUK]	= "Corporate personalisation PUK",
};

const char *lgsm_pin_name(enum gsmd_pin_type ptype)
{
	if (ptype >= __NUM_GSMD_PIN)
		return "unknown";

	return pin_type_names[ptype];
}

int lgsm_pin_status(struct lgsm_handle *lh)
{
	return lgsm_send_simple(lh, GSMD_MSG_PIN, GSMD_PIN_GET_STATUS);
}

int lgsm_pin(struct lgsm_handle *lh, unsigned int type,
		const char *pin, const char *newpin)
{
	int rc;
	struct {
		struct gsmd_msg_hdr gmh;
		struct gsmd_pin gp;
	} __attribute__ ((packed)) *gm;

	if (strlen(pin) > GSMD_PIN_MAXLEN || 
	    (newpin && strlen(newpin) > GSMD_PIN_MAXLEN) ||
	    type >= __NUM_GSMD_PIN)
		return -EINVAL;

	gm = (void *) lgsm_gmh_fill(GSMD_MSG_PIN, GSMD_PIN_INPUT,
				    sizeof(struct gsmd_pin));
	if (!gm)
		return -ENOMEM;

	gm->gp.type = type;
	strncpy(gm->gp.pin, pin, sizeof(gm->gp.pin));

	switch (type) {
	case GSMD_PIN_SIM_PUK:
	case GSMD_PIN_SIM_PUK2:
		/* GSM 07.07 explicitly states that only those two PUK types
		 * require a new pin to be specified! Don't know if this is a
		 * bug or a feature. */
		if (!newpin) {
			free(gm);
			return -EINVAL;
		}
		strncpy(gm->gp.newpin, newpin, sizeof(gm->gp.newpin));
		break;
	default:
		break;
	}
	printf("sending pin='%s', newpin='%s'\n", gm->gp.pin, gm->gp.newpin);
	rc = lgsm_send(lh, &gm->gmh);
	free(gm);

	return rc;
}

