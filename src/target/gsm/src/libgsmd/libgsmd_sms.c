/* libgsmd sms support
 *
 * (C) 2007 by OpenMoko, Inc.
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
#include <stdio.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/misc.h>
#include <libgsmd/sms.h>

#include <gsmd/usock.h>
#include <gsmd/event.h>

#include "lgsm_internals.h"

int lgsm_sms_list(struct lgsm_handle *lh, enum gsmd_msg_sms_type stat)
{
	/* FIXME: only support PDU mode now */
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_SMS,
			GSMD_SMS_LIST, sizeof(int));
	if (!gmh)
		return -ENOMEM;
	*(int *) gmh->data = stat;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_sms_read(struct lgsm_handle *lh, int index)
{
	struct gsmd_msg_hdr *gmh;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_SMS,
			GSMD_SMS_READ, sizeof(int));
	if (!gmh)
		return -ENOMEM;
	*(int *) gmh->data = index;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);;
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_sms_delete(struct lgsm_handle *lh,
		const struct lgsm_sms_delete *sms_del)
{
	struct gsmd_msg_hdr *gmh;
	struct gsmd_sms_delete *gsd;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_SMS,
			GSMD_SMS_DELETE, sizeof(*gsd));
	if (!gmh)
		return -ENOMEM;
	gsd = (struct gsmd_sms_delete *) gmh->data;
	gsd->index = sms_del->index;
	gsd->delflg = sms_del->delflg;

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_number2addr(struct gsmd_addr *dst, const char *src, int skipplus)
{
	char *ch;

	if (strlen(src) + 1 > sizeof(dst->number))
		return 1;
	if (src[0] == '+') {
		dst->type =
			GSMD_TOA_NPI_ISDN |
			GSMD_TOA_TON_INTERNATIONAL |
			GSMD_TOA_RESERVED;
		strcpy(dst->number, src + skipplus);
	} else {
		dst->type =
			GSMD_TOA_NPI_ISDN |
			GSMD_TOA_TON_UNKNOWN |
			GSMD_TOA_RESERVED;
		strcpy(dst->number, src);
	}

	for (ch = dst->number; *ch; ch ++)
		if (*ch < '0' || *ch > '9')
			return 1;
	return 0;
}

int lgsm_sms_send(struct lgsm_handle *lh,
		const struct lgsm_sms *sms)
{
	/* FIXME: only support PDU mode */
	struct gsmd_msg_hdr *gmh;
	struct gsmd_sms_submit *gss;
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_SMS,
			GSMD_SMS_SEND, sizeof(*gss));
	if (!gmh)
		return -ENOMEM;
	gss = (struct gsmd_sms_submit *) gmh->data;

	if (lgsm_number2addr(&gss->addr, sms->addr, 1))
		return -EINVAL;

	gss->ask_ds = sms->ask_ds;
	gss->payload.has_header = 0;
	gss->payload.length = sms->length;
	gss->payload.coding_scheme = sms->alpha;
	memcpy(gss->payload.data, sms->data, LGSM_SMS_DATA_MAXLEN);

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_sms_write(struct lgsm_handle *lh,
		const struct lgsm_sms_write *sms_write)
{
	/* FIXME: only support PDU mode */
	struct gsmd_msg_hdr *gmh;
	struct gsmd_sms_write *gsw; 
	int rc;

	gmh = lgsm_gmh_fill(GSMD_MSG_SMS,
			GSMD_SMS_WRITE, sizeof(*gsw));
	if (!gmh)
		return -ENOMEM;
	gsw = (struct gsmd_sms_write *) gmh->data;

	gsw->stat = sms_write->stat;

	if (lgsm_number2addr(&gsw->sms.addr, sms_write->sms.addr, 1))
		return -EINVAL;

	gsw->sms.ask_ds = sms_write->sms.ask_ds;
	gsw->sms.payload.has_header = 0;
	gsw->sms.payload.length = sms_write->sms.length;
	gsw->sms.payload.coding_scheme = sms_write->sms.alpha;
	memcpy(gsw->sms.payload.data, sms_write->sms.data,
			LGSM_SMS_DATA_MAXLEN);

	rc = lgsm_send(lh, gmh);
	if (rc < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);

	return 0;
}

int lgsm_sms_get_storage(struct lgsm_handle *lh)
{
	return lgsm_send_simple(lh, GSMD_MSG_SMS, GSMD_SMS_GET_MSG_STORAGE);
}

int lgsm_sms_set_storage(struct lgsm_handle *lh, enum ts0705_mem_type mem1,
		enum ts0705_mem_type mem2, enum ts0705_mem_type mem3)
{
	struct gsmd_msg_hdr *gmh =
		lgsm_gmh_fill(GSMD_MSG_SMS, GSMD_SMS_SET_MSG_STORAGE,
				3 * sizeof(enum ts0705_mem_type));
	if (!gmh)
		return -ENOMEM;

	((enum ts0705_mem_type *) gmh->data)[0] = mem1;
	((enum ts0705_mem_type *) gmh->data)[1] = mem2;
	((enum ts0705_mem_type *) gmh->data)[2] = mem3;

	if (lgsm_send(lh, gmh) < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);
	return 0;
}

int lgsm_sms_get_smsc(struct lgsm_handle *lh)
{
	return lgsm_send_simple(lh, GSMD_MSG_SMS, GSMD_SMS_GET_SERVICE_CENTRE);
}

int lgsm_sms_set_smsc(struct lgsm_handle *lh, const char *number)
{
	struct gsmd_msg_hdr *gmh =
		lgsm_gmh_fill(GSMD_MSG_SMS, GSMD_SMS_SET_SERVICE_CENTRE,
				sizeof(struct gsmd_addr));
	if (!gmh)
		return -ENOMEM;

	if (lgsm_number2addr((struct gsmd_addr *) gmh->data, number, 0)) {
		lgsm_gmh_free(gmh);
		return -EINVAL;
	}

	if (lgsm_send(lh, gmh) < gmh->len + sizeof(*gmh)) {
		lgsm_gmh_free(gmh);
		return -EIO;
	}

	lgsm_gmh_free(gmh);
	return 0;
}

int packing_7bit_character(const char *src, struct lgsm_sms *dest)
{
	int i,j = 0;
	unsigned char ch1, ch2;
	int shift = 0;

	dest->alpha = ALPHABET_DEFAULT;

	for ( i=0; i<strlen(src); i++ ) {
		
		ch1 = src[i] & 0x7F;
		ch1 = ch1 >> shift;
		ch2 = src[(i+1)] & 0x7F;
		ch2 = ch2 << (7-shift);

		ch1 = ch1 | ch2;

		if (j > sizeof(dest->data))
			break;
		dest->data[j++] = ch1;

		shift++;

		if ( 7 == shift ) {
			shift = 0;
			i++;
		}
	}

	dest->length = i;
	return j;
}

int unpacking_7bit_character(const struct gsmd_sms *src, char *dest)
{
	int i = 0;
	int l = 0;

	if (src->has_header)
		l += ((src->data[0] << 3) + 14) / 7;
	i += l;
	for (; i < src->length; i ++)
		*(dest ++) =
			((src->data[(i * 7 + 7) >> 3] <<
			  (7 - ((i * 7 + 7) & 7))) |
			 (src->data[(i * 7) >> 3] >>
			  ((i * 7) & 7))) & 0x7f;
	*dest = '\0';

	return i - l;
}

int cbm_unpacking_7bit_character(const char *src, char *dest)
{
	int i;
	u_int8_t ch = 1;

	for (i = 0; i < 93 && ch; i ++)
		*(dest ++) = ch =
			((src[(i * 7 + 7) >> 3] << (7 - ((i * 7 + 7) & 7))) |
			 (src[(i * 7) >> 3] >> ((i * 7) & 7))) & 0x7f;
	*dest = '\0';

	return i;
}

/* Refer to 3GPP TS 11.11 Annex B */
int packing_UCS2_80(char *src, char *dest)
{
	return 0;
}

/* Refer to 3GPP TS 11.11 Annex B */
int unpacking_UCS2_80(char *src, char *dest)
{
	return 0;
}

/* Refer to 3GPP TS 11.11 Annex B */
int packing_UCS2_81(char *src, char *dest)
{
	return 0;
}

/* Refer to 3GPP TS 11.11 Annex B */
int unpacking_UCS2_81(char *src, char *dest)
{
	return 0;
}

/* Refer to 3GPP TS 11.11 Annex B */
int packing_UCS2_82(char *src, char *dest)
{
	return 0;
}

/* Refer to 3GPP TS 11.11 Annex B */
int unpacking_UCS2_82(char *src, char *dest)
{
	return 0;
}

int lgsm_cb_subscribe(struct lgsm_handle *lh)
{
	return lgsm_send_simple(lh, GSMD_MSG_CB, GSMD_CB_SUBSCRIBE);
}

int lgsm_cb_unsubscribe(struct lgsm_handle *lh)
{
	return lgsm_send_simple(lh, GSMD_MSG_CB, GSMD_CB_UNSUBSCRIBE);
}
