/* libgsmd event demultiplexer handler 
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
#include <unistd.h>
#include <string.h>

#include <libgsmd/voicecall.h>
#include <libgsmd/event.h>

#include <gsmd/usock.h>
#include <gsmd/event.h>

#include "lgsm_internals.h"

static lgsm_evt_handler *evt_handlers[__NUM_GSMD_EVT];

int lgsm_evt_handler_register(struct lgsm_handle *lh, int evt_type,
			      lgsm_evt_handler *handler)
{
	if (evt_type >= __NUM_GSMD_EVT)
		return -EINVAL;

	evt_handlers[evt_type] = handler;

	return 0;
}

void lgsm_evt_handler_unregister(struct lgsm_handle *lh, int evt_type)
{
	if (evt_type < __NUM_GSMD_EVT)
		evt_handlers[evt_type] = NULL;
}


static int evt_demux_msghandler(struct lgsm_handle *lh,
		struct gsmd_msg_hdr *gmh)
{
	struct gsmd_evt_auxdata *aux = (struct gsmd_evt_auxdata *) gmh->data;

	if (gmh->len < sizeof(*aux))
		return -EIO;
	
	if (gmh->msg_type != GSMD_MSG_EVENT || 
	    gmh->msg_subtype >= __NUM_GSMD_EVT)
		return -EINVAL;

	switch (gmh->msg_subtype) {
	case GSMD_EVT_NETREG:
		lh->netreg_state = aux->u.netreg.state;
		break;
	}

	if (evt_handlers[gmh->msg_subtype])
		return evt_handlers[gmh->msg_subtype](lh, gmh->msg_subtype, aux);
	else
		return 0;
}

int lgsm_evt_init(struct lgsm_handle *lh)
{
	lh->netreg_state = LGSM_NETREG_ST_NOTREG;
	return lgsm_register_handler(lh, GSMD_MSG_EVENT, &evt_demux_msghandler);
}

void lgsm_evt_exit(struct lgsm_handle *lh)
{
	lgsm_unregister_handler(lh, GSMD_MSG_EVENT);
}
