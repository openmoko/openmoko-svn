

#include <libgsmd/voicecall.h>

#include "lgsm_internals.h"


int lgsm_voice_out_init(struct lgsm_handle *lh,
			const struct lgsm_addr *number)
{
	/* send ATD command */
	return -EINVAL;
}

int lgsm_voice_in_accept(struct lgsm_handle *lh)
{
	return -EINVAL;
}

int lgsm_voice_hangup(struct lgsm_handle *lh)
{
	/* Send ATH0 */
	return -EINVAL;
}
