#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/misc.h>

#include <gsmd/usock.h>
#include <gsmd/event.h>

#include "lgsm_internals.h"

int lgsm_modem_power(struct lgsm_handle *lh, int power)
{
	int type;

	if (power)
		type = GSMD_MODEM_POWERUP;
	else
		type = GSMD_MODEM_POWERDOWN;
		
	return lgsm_send_simple(lh, GSMD_MSG_MODEM, type);
}
