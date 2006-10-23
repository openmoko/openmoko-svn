#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libgsmd/voicecall.h>

#include "lgsm_internals.h"

int lgsm_send_simple(struct lgsm_handle *lh, int type, int sub_type)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(type, sub_type, 0);
	if (!gmh)
		return -ENOMEM;
	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}
	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_voice_out_init(struct lgsm_handle *lh,
			const struct lgsm_addr *number)
{
	struct gsmd_msg_hdr *gmh;
	struct gsmd_addr *ga;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_VOICECALL,
			    GSMD_VOICECALL_DIAL, sizeof(*ga));
	if (!gmh)
		return -ENOMEM;
	ga = (struct gsmd_addr *) gmh->data;
	ga->type = number->type;	/* FIXME: is this correct? */
	memcpy(ga->number, number->addr, sizeof(ga->number));
	ga->number[sizeof(ga->number)-1] = '\0';

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_voice_in_accept(struct lgsm_handle *lh)
{
	return lgsm_send_simple(lh, GSMD_MSG_VOICECALL, GSMD_VOICECALL_ANSWER);
}

int lgsm_voice_hangup(struct lgsm_handle *lh)
{
	return lgsm_send_simple(lh, GSMD_MSG_VOICECALL, GSMD_VOICECALL_HANGUP);
}
