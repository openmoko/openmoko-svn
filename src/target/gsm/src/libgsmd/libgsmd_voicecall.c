/* libgsmd voice call support
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

#include "lgsm_internals.h"

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

int lgsm_voice_dtmf(struct lgsm_handle *lh, char dtmf_char)
{
	struct gsmd_msg_hdr *gmh;
	struct gsmd_dtmf *gd;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_VOICECALL,
			    GSMD_VOICECALL_DTMF, sizeof(*gd)+1);
	if (!gmh)
		return -ENOMEM;
	gd = (struct gsmd_dtmf *) gmh->data;
	gd->len = 1;
	gd->dtmf[0] = dtmf_char;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gd)+1) {
		lgsm_gmh_free(gmh);;
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

int lgsm_voice_volume_set(struct lgsm_handle *lh, int volume)
{
	/* FIXME: we need to pass along the parameter */
	return lgsm_send_simple(lh, GSMD_MSG_VOICECALL, GSMD_VOICECALL_VOL_SET);
}

int lgsm_voice_get_status(struct lgsm_handle *lh)
{
	return lgsm_send_simple(lh, GSMD_MSG_VOICECALL, GSMD_VOICECALL_GET_STAT);
}

int lgsm_voice_ctrl(struct lgsm_handle *lh, const struct lgsm_voicecall_ctrl *ctrl)
{
	struct gsmd_msg_hdr *gmh;
	struct gsmd_call_ctrl *gcc;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_VOICECALL,
			    GSMD_VOICECALL_CTRL, sizeof(*gcc));
	if (!gmh)
		return -ENOMEM;
	gcc = (struct gsmd_call_ctrl *) gmh->data;
	gcc->proc = ctrl->proc;
	gcc->idx = ctrl->idx;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_voice_fwd_disable(struct lgsm_handle *lh, 
		           enum lgsmd_voicecall_fwd_reason reason)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_VOICECALL,
			GSMD_VOICECALL_FWD_DIS, sizeof(int));
	if (!gmh)
		return -ENOMEM;

	*(int *) gmh->data = reason;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_voice_fwd_enable(struct lgsm_handle *lh, 
                          enum lgsmd_voicecall_fwd_reason reason)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_VOICECALL,
			GSMD_VOICECALL_FWD_EN, sizeof(int));
	if (!gmh)
		return -ENOMEM;

	*(int *) gmh->data = reason;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_voice_fwd_stat(struct lgsm_handle *lh, 
                        enum lgsmd_voicecall_fwd_reason reason)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_VOICECALL,
			GSMD_VOICECALL_FWD_STAT, sizeof(int));
	if (!gmh)
		return -ENOMEM;

	*(int *) gmh->data = reason;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_voice_fwd_reg(struct lgsm_handle *lh, 
		       struct lgsm_voicecall_fwd_reg *fwd_reg)
{
	struct gsmd_msg_hdr *gmh;
	struct gsmd_call_fwd_reg *gcfr;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_VOICECALL,
			GSMD_VOICECALL_FWD_REG, sizeof(struct gsmd_call_fwd_reg));
	if (!gmh)
		return -ENOMEM;

	gcfr = (struct gsmd_call_fwd_reg *)gmh->data;
	gcfr->reason = fwd_reg->reason;
	strcpy(gcfr->addr.number, fwd_reg->number.addr);

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_voice_fwd_erase(struct lgsm_handle *lh, 
                         enum lgsmd_voicecall_fwd_reason reason)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_VOICECALL,
			GSMD_VOICECALL_FWD_ERAS, sizeof(int));
	if (!gmh)
		return -ENOMEM;

	*(int *) gmh->data = reason;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}
