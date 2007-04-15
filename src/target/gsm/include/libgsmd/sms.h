#ifndef _LIBGSMD_SMS_H
#define _LIBGSMD_SMS_H

#include <libgsmd/libgsmd.h>

/* Short Message Service */

/* Data Coding Scheme, refer to GSM 03.38 Clause 4 */
#define B5_COMPRESSED	(1<<5)
#define B4_CLASSMEANING	(1<<4)
enum {
	MESSAGE_CLASS_CLASS0		= 0x00,
	MESSAGE_CLASS_CLASS1		= 0x01,
	MESSAGE_CLASS_CLASS2		= 0x10,
	MESSAGE_CLASS_CLASS3		= 0x11,
};

enum {
	ALPHABET_DEFAULT		= (0x00<<2),
	ALPHABET_8BIT			= (0x01<<2),
	ALPHABET_UCS2			= (0x10<<2),
	ALPHABET_RESERVED		= (0x11<<2),
};

/* Coding of Alpha fields in the SIM for UCS2, (3GPP TS 11.11 Annex B) */
//enum {	
		
//};


/* SMS delflg from 3GPP TS 07.05, Clause 3.5.4 */
enum lgsm_msg_sms_delflg {
	LGSM_SMS_DELFLG_INDEX		= 0,
	LGSM_SMS_DELFLG_READ		= 1,
	LGSM_SMS_DELFLG_READ_SENT	= 2,
	LGSM_SMS_DELFLG_LEAVING_UNREAD	= 3,
	LGSM_SMS_DELFLG_ALL		= 4,
};

/* SMS stat from 3GPP TS 07.05, Clause 3.1 */
/* FIXME: only support PDU mode */
enum lgsm_msg_sms_stat {
	LGSM_SMS_REC_UNREAD		= 0,
	LGSM_SMS_REC_READ		= 1,
	LGSM_SMS_STO_UNSENT		= 2,
	LGSM_SMS_STO_SENT		= 3,
	LGSM_SMS_ALL			= 4,
};

/* Refer to GSM 07.05 subclause 3.5.4 */
struct lgsm_sms_delete {
	int index;
	enum lgsm_msg_sms_delflg delflg;
};

/* Refer to GSM 03.40 subclause 9.2.2.2 */
#define LGSM_SMS_ADDR_MAXLEN	12
#define LGSM_SMS_DATA_MAXLEN	140
struct lgsm_sms {
	/* FIXME: max length of data, 
	 * 7 bit coding - 160(140*8/7); ucs2 coding - 70(140/2) */
	char addr[LGSM_SMS_ADDR_MAXLEN+1];
	char data[LGSM_SMS_DATA_MAXLEN+1];
};

/* GSM 03.40 subclause 9.2.2.2 and GSM 07.05 subclause 4.4 and subclause 3.1 */
struct lgsm_sms_write {
	enum lgsm_msg_sms_stat stat; 			
	struct lgsm_sms sms;
};

/* List Messages */
extern int lgsm_sms_list(struct lgsm_handle *lh, enum gsmd_msg_sms_type stat);

/* Read Message */
extern int lgsm_sms_read(struct lgsm_handle *lh, int index);

/* Delete Message */
extern int lgsmd_sms_delete(struct lgsm_handle *lh, 
		const struct lgsm_sms_delete *sms_del);

/* Send Message */
extern int lgsmd_sms_send(struct lgsm_handle *lh, const struct lgsm_sms *sms);

/* Write Message to Memory */
extern int lgsmd_sms_write(struct lgsm_handle *lh, 
		const struct lgsm_sms_write *sms_write);

/* Packing of 7-bit characters, refer to GSM 03.38 subclause 6.1.2.1.1 */
extern int packing_7bit_character(char *src, char *dest);

/* Packing of 7-bit characters, refer to GSM 03.38 subclause 6.1.2.1.1 */
extern int unpacking_7bit_character(char *src, char *dest);

/* Refer to 3GPP TS 11.11 Annex B */
extern int packing_UCS2_80(char *src, char *dest);

/* Refer to 3GPP TS 11.11 Annex B */
extern int unpacking_UCS2_80(char *src, char *dest);

/* Refer to 3GPP TS 11.11 Annex B */
extern int packing_UCS2_81(char *src, char *dest);

/* Refer to 3GPP TS 11.11 Annex B */
extern int unpacking_UCS2_81(char *src, char *dest);

/* Refer to 3GPP TS 11.11 Annex B */
extern int packing_UCS2_82(char *src, char *dest);

/* Refer to 3GPP TS 11.11 Annex B */
extern int unpacking_UCS2_82(char *src, char *dest);

#endif

