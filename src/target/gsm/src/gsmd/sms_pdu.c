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
		return len;
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

	/* init voicemail is false */
	dst->payload.is_voicemail = 0;
	
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
		dst->payload.tp_mti = GSMD_SMS_TP_MTI_DELIVER;
		if (len < 3)
			return 1;
		i = sms_number_bytelen(src[2], src[1]);
		if (len < 13 + i)
			return 1;

		if (sms_address2ascii(&dst->addr, src + 1))
			return 1;

		len -= 3 + i;
		src += 3 + i;
		
		/* check voicemail by TP-PID */
		if(src[0] == 0x5f)  /* return call message */
			dst->payload.is_voicemail = 1;

		/* decode TP-DCS */
		if(sms_pdu_decode_dcs(&dst->payload.dcs,src+1))
			return 1;
		/* check voicemail by MWI */
		if(dst->payload.dcs.mwi_kind == MESSAGE_WAITING_VOICEMAIL &&
			(dst->payload.dcs.mwi_group == MESSAGE_WAITING_DISCARD || 
			dst->payload.dcs.mwi_group == MESSAGE_WAITING_STORE))
			dst->payload.is_voicemail = 1;
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

		/* TP-UDL */
		dst->payload.length = src[0];
		i = sms_data_bytelen(dst->payload.coding_scheme, src[0]);

		/* TP-UD */
		if (len < 1 + i || i > GSMD_SMS_DATA_MAXLEN)
			return 1;
		memcpy(dst->payload.data, src + 1, i);
		dst->payload.data[i] = 0;

		break;
	case GSMD_SMS_TP_MTI_SUBMIT:
		dst->payload.tp_mti = GSMD_SMS_TP_MTI_SUBMIT;
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

		/* TP-UDL */
		dst->payload.length = src[0];
		i = sms_data_bytelen(dst->payload.coding_scheme, src[0]);

		/* TP-UD */
		if (len < 1 + i || i > GSMD_SMS_DATA_MAXLEN)
			return 1;
		memcpy(dst->payload.data, src + 1, i);
		dst->payload.data[i] = 0;
		break;
	case GSMD_SMS_TP_MTI_STATUS_REPORT:
		dst->payload.tp_mti = GSMD_SMS_TP_MTI_STATUS_REPORT;
		if (len < 3)
			return 1;

		/* TP-MR set it gsmd_sms_list.index*/
		dst->index = (u_int8_t) src[1];
		/* TP-STATUS set it to coding_scheme */
		dst->payload.coding_scheme = (u_int8_t) src[len-1];
		/* TP-RA */
		i = sms_number_bytelen(src[3], src[2]);
		if (len < 13 + i)
			return 1;
		if (sms_address2ascii(&dst->addr, src + 2))
			return 1;
		len -= 4 + i;
		src += 4 + i;
		/* TP-SCTS */
		memcpy(dst->time_stamp, src, 7);
		/* TP-UD  */
		dst->payload.length = 0;
		dst->payload.data[0] = 0;
		break;
	default:
		/* Unknown PDU type */
		return 1;
	}


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
		(src->ask_ds ? GSMD_SMS_TP_SRR_STATUS_REQUEST :
		 GSMD_SMS_TP_SRR_NOT_REQUEST) |
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

/* Refer to GSM 03.38 Clause 4, for TP-DCS */  
int sms_pdu_decode_dcs(struct gsmd_sms_datacodingscheme *dcs, 
	const u_int8_t *data)
{
	int pos = 0, i;

	/* init dcs value */
	dcs->mwi_active		= NOT_ACTIVE;
	dcs->mwi_kind		= MESSAGE_WAITING_OTHER;
	
	/* bits 7-6 */
	i = ( data[pos] & 0xC0 ) >> 6;
	switch( i )
	{
	case 0: /* pattern 00xx xxxx */
		dcs->is_compressed = data[pos] & 0x20;
		if( data[pos] & 0x10 )
			dcs->msg_class = data[pos] & 0x03;
		else
			/* no class information */
			dcs->msg_class = MSG_CLASS_NONE;
		dcs->alphabet  = ( data[pos] & 0x0C ) >> 2;     
		dcs->mwi_group 	= MESSAGE_WAITING_NONE;
		break;
	case 3: /* pattern 1111 xxxx */
		/* bits 5-4 */
		if( (data[pos] & 0x30) == 0x30 )
		{
			/* bit 3 is reserved */
			/* bit 2 */
			dcs->alphabet = (data[pos] & 0x04 ) ? SMS_ALPHABET_8_BIT:
					   SMS_ALPHABET_7_BIT_DEFAULT;
			/* bits 1-0 */
			dcs->msg_class = data[pos] & 0x03;
			/* set remaining fields */
			dcs->is_compressed  = NOT_COMPRESSED;
			dcs->mwi_group    = MESSAGE_WAITING_NONE_1111;
		}
		else
		{
			/* Message waiting groups */
			dcs->is_compressed  = NOT_COMPRESSED;
			dcs->msg_class      = MSG_CLASS_NONE;
			/* bits 5-4 */
			if( (data[pos] & 0x30) == 0x00 )
			{
				dcs->mwi_group  = MESSAGE_WAITING_DISCARD;
				dcs->alphabet   = SMS_ALPHABET_7_BIT_DEFAULT;
			}
			else if( (data[pos] & 0x30) == 0x10 )
			{
				dcs->mwi_group  = MESSAGE_WAITING_STORE;
				dcs->alphabet   = SMS_ALPHABET_7_BIT_DEFAULT;
			}
			else
			{
				dcs->mwi_group  = MESSAGE_WAITING_STORE;
				dcs->alphabet   = SMS_ALPHABET_UCS2;
			}
			/* bit 3 */
			dcs->mwi_active = ( data[pos] & 0x08 ) ? ACTIVE : 
				NOT_ACTIVE;
			/* bit 2 is reserved */
			/* bits 1-0 */
			dcs->mwi_kind = data[pos] & 0x03;
		}
		break;
	default:
		/* reserved values	*/
		dcs->msg_class      	= MSG_CLASS_NONE;
		dcs->alphabet       	= SMS_ALPHABET_7_BIT_DEFAULT;
		dcs->is_compressed  	= NOT_COMPRESSED;
		dcs->mwi_group    	= MESSAGE_WAITING_NONE;
		dcs->mwi_active		= NOT_ACTIVE;
		dcs->mwi_kind		= MESSAGE_WAITING_OTHER;
		break;
	}

	if ( dcs->alphabet > SMS_ALPHABET_UCS2 )
		dcs->alphabet = SMS_ALPHABET_7_BIT_DEFAULT;
	/* keep raw dcs data*/
	dcs->raw_dcs_data = data[pos];
	return 0;
}
