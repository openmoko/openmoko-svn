/* Convert raw PDUs to and from gsmd format.
 *
 * Copyright (C) 2007 OpenMoko, Inc.
 * Written by Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <string.h>

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/usock.h>
#include <gsmd/sms.h>

static int sms_number_bytelen(u_int8_t type, u_int8_t len)
{
	switch (type & __GSMD_TOA_TON_MASK) {
	case GSMD_TOA_TON_ALPHANUMERIC:
		return (len + 1) >> 1;
	default:
		return (len + 1) >> 1;
	}
}

static int sms_data_bytelen(u_int8_t data_coding_scheme, u_int8_t len)
{
	switch (data_coding_scheme) {	/* Get length in bytes */
	case ALPHABET_DEFAULT:
		return (len * 7 + 7) >> 3;
	case ALPHABET_8BIT:
		return len;
	case ALPHABET_UCS2:
		return len * 2;
	}
	return 0;
}

static int sms_address2ascii(struct gsmd_addr *dst, const u_int8_t *src)
{
	int i;

	if (src[0] > GSMD_ADDR_MAXLEN)
		return 1;

	/* The Type-of-address field */
	dst->type = src[1];

	switch (dst->type & __GSMD_TOA_TON_MASK) {
	case GSMD_TOA_TON_ALPHANUMERIC:
		for (i = 0; ((i * 7 + 3) >> 2) < src[0]; i ++)
			dst->number[i] =
				((src[2 + ((i * 7 + 7) >> 3)] <<
				  (7 - ((i * 7 + 7) & 7))) |
				 (src[2 + ((i * 7) >> 3)] >>
				  ((i * 7) & 7))) & 0x7f;
		break;
	default:
		for (i = 0; i < src[0]; i ++)
			dst->number[i] = '0' +
				((src[2 + (i >> 1)] >> ((i << 2) & 4)) & 0xf);
	}
	dst->number[i] = 0;

	return 0;
}

int sms_pdu_to_msg(struct gsmd_sms_list *dst,
		const u_int8_t *src, int pdulen, int len)
{
	int i, vpf;
	if (len < 1 || len < 1 + src[0] + pdulen || pdulen < 1)
		return 1;

	/* Skip SMSC number and its Type-of-address */
	len -= 1 + src[0];
	src += 1 + src[0];

	/* TP-UDHI */
	dst->payload.has_header = !!(src[0] & GSMD_SMS_TP_UDHI_WITH_HEADER);

	/* TP-VPF */
	vpf = (src[0] >> 3) & 3;

	/* TP-MTI */
	switch (src[0] & 3) {
	case GSMD_SMS_TP_MTI_DELIVER:
		if (len < 3)
			return 1;
		i = sms_number_bytelen(src[2], src[1]);
		if (len < 13 + i)
			return 1;

		if (sms_address2ascii(&dst->addr, src + 1))
			return 1;

		len -= 3 + i;
		src += 3 + i;

		/* TP-DCS */
		switch (src[1] >> 4) {
		case 0x0 ... 3:	/* General Data Coding indication */
		case 0xc:	/* Message Waiting indication: Discard */
			/* FIXME: support compressed data */
			dst->payload.coding_scheme = src[1] & 0xc;
			break;
		case 0xd:	/* Message Waiting indication: Store */
			dst->payload.coding_scheme = ALPHABET_DEFAULT;
			break;
		case 0xe:	/* Message Waiting indication: Store */
			dst->payload.coding_scheme = ALPHABET_UCS2;
			break;
		case 0xf:	/* Data coding/message class */
			dst->payload.coding_scheme = (src[1] & 4) ?
				ALPHABET_8BIT : ALPHABET_DEFAULT;
			break;
		default:
			return 1;
		}

		/* TP-SCTS */
		memcpy(dst->time_stamp, src + 2, 7);

		/* Skip TP-PID */
		len -= 9;
		src += 9;
		break;
	case GSMD_SMS_TP_MTI_SUBMIT:
		if (len < 4)
			return 1;
		i = sms_number_bytelen(src[3], src[2]);
		if (len < 8 + i)
			return 1;

		if (sms_address2ascii(&dst->addr, src + 2))
			return 1;

		len -= 4 + i;
		src += 4 + i;

		/* TP-DCS */
		switch (src[1] >> 4) {
		case 0x0 ... 3:	/* General Data Coding indication */
		case 0xc:	/* Message Waiting indication: Discard */
			/* FIXME: compressed data */
			dst->payload.coding_scheme = src[1] & 0xc;
			break;
		case 0xd:	/* Message Waiting indication: Store */
			dst->payload.coding_scheme = ALPHABET_DEFAULT;
			break;
		case 0xe:	/* Message Waiting indication: Store */
			dst->payload.coding_scheme = ALPHABET_UCS2;
			break;
		case 0xf:	/* Data coding/message class */
			dst->payload.coding_scheme = (src[1] & 4) ?
				ALPHABET_8BIT : ALPHABET_DEFAULT;
			break;
		default:
			return 1;
		}

		/* Skip TP-PID and TP-Validity-Period */
		len -= vpf ? 3 : 2;
		src += vpf ? 3 : 2;

		memset(dst->time_stamp, 0, 7);
		break;
	case GSMD_SMS_TP_MTI_STATUS_REPORT:
		/* TODO */
	default:
		/* Unknown PDU type */
		return 1;
	}

	/* TP-UDL */
	dst->payload.length = src[0];
	i = sms_data_bytelen(dst->payload.coding_scheme, src[0]);

	/* TP-UD */
	if (len < 1 + i || i > GSMD_SMS_DATA_MAXLEN)
		return 1;
	memcpy(dst->payload.data, src + 1, i);
	dst->payload.data[i] = 0;

	return 0;
}

/* Refer to GSM 03.40 subclause 9.2.3.3, for SMS-SUBMIT */
int sms_pdu_make_smssubmit(char *dest, const struct gsmd_sms_submit *src)
{
	/* FIXME: ALPHANUMERIC encoded addresses can be longer than 13B */
	u_int8_t header[15 + GSMD_ADDR_MAXLEN];
	int pos = 0, i, len;

	/* SMSC Length octet.  If omitted or zero, use SMSC stored in the
	 * phone.  One some phones this can/has to be omitted.  */
	header[pos ++] = 0x00;

	header[pos ++] =
		GSMD_SMS_TP_MTI_SUBMIT |
		(0 << 2) |		/* Reject Duplicates: 0 */
		GSMD_SMS_TP_VPF_NOT_PRESENT |
		GSMD_SMS_TP_SRR_NOT_REQUEST |
		(src->payload.has_header ? GSMD_SMS_TP_UDHI_WITH_HEADER :
		 GSMD_SMS_TP_UDHI_NO_HEADER) |
		GSMD_SMS_TP_RP_NOT_SET;

	/* TP-Message-Reference - 00 lets the phone set the number itself */
	header[pos ++] = 0x00;

	header[pos ++] = strlen(src->addr.number);
	header[pos ++] = src->addr.type;
	for (i = 0; src->addr.number[i]; i ++) {
		header[pos] = src->addr.number[i ++] - '0';
		if (src->addr.number[i])
			header[pos ++] |= (src->addr.number[i] - '0') << 4;
		else {
			header[pos ++] |= 0xf0;
			break;
		}
	}

	/* TP-Protocol-Identifier - 00 means implicit */
	header[pos ++] = 0x00;

	/* TP-Data-Coding-Scheme */
	header[pos ++] = src->payload.coding_scheme;

	/* TP-Validity-Period, if present, would go here */

	header[pos ++] = src->payload.length;
	len = sms_data_bytelen(src->payload.coding_scheme,
			src->payload.length);

	if (dest) {
		for (i = 0; i < pos; i ++) {
			sprintf(dest, "%02X", header[i]);
			dest += 2;
		}
		for (i = 0; i < len; i ++) {
			sprintf(dest, "%02X", src->payload.data[i]);
			dest += 2;
		}
	}

	return pos + len;
}

/* Refer to GSM 03.41 subclause 9.3 */
int cbs_pdu_to_msg(struct gsmd_cbm *dst, u_int8_t *src, int pdulen, int len)
{
	if (len != pdulen || len != CBM_MAX_PDU_SIZE)
		return 1;

	dst->serial.scope = (src[0] >> 6) & 3;
	dst->serial.msg_code = ((src[0] << 4) | (src[1] >> 4)) & 0x3ff;
	dst->serial.update_num = src[1] & 0xf;

	dst->msg_id = (src[2] << 8) | src[3];

	dst->language = src[4] & 0xf;
	dst->coding_scheme = ((src[4] >> 4) & 3) << 2;

	dst->pages = src[5] & 0xf;
	dst->page = src[5] >> 4;

	memcpy(dst->data, src + 6, len - 6);
	return 0;
}