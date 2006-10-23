#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/misc.h>

#include <gsmd/usock.h>
#include <gsmd/event.h>

#include "lgsm_internals.h"

int lgsm_phone_power(struct lgsm_handle *lh, int power)
{
	int type;

	if (power)
		type = GSMD_PHONE_POWERUP;
	else
		type = GSMD_PHONE_POWERDOWN;
		
	return lgsm_send_simple(lh, GSMD_MSG_PHONE, type);
}
