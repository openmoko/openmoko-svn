/* libgsmd event demultiplexer handler */

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


static int evt_demux_msghandler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh)
{
	struct gsmd_evt_auxdata *aux = gmh->data;

	if (gmh->len < sizeof(*aux))
		return -EIO;
	
	if (gmh->msg_type != GSMD_MSG_EVENT || 
	    gmh->msg_subtype >= __NUM_GSMD_EVT)
		return -EINVAL;

	return evt_handlers[gmh->msg_subtype](lh, gmh->msg_subtype, aux);
}

int lgsm_evt_init(struct lgsm_handle *lh)
{
	return lgsm_register_handler(lh, GSMD_MSG_EVENT, &evt_demux_msghandler);
}

void lgsm_evt_exit(struct lgsm_handle *lh)
{
	lgsm_unregister_handler(lh, GSMD_MSG_EVENT);
}
