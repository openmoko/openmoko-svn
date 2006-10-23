#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/misc.h>

#include <gsmd/usock.h>
#include <gsmd/event.h>

#include "lgsm_internals.h"

int lgsm_netreg_register(struct lgsm_handle *lh, int oper)
{
	/* FIXME: implement oper selection */
	return lgsm_send_simple(lh, GSMD_MSG_NETWORK, GSMD_NETWORK_REGISTER);
}

