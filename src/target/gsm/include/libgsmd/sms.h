#ifndef _LIBGSMD_SMS_H
#define _LIBGSMD_SMS_H

#include <libgsmd/libgsmd.h>

/* Short Message Service */

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

/* TP-Status from 3GPP TS 23.040 section 9.2.3.15 */
enum lgsm_tp_status {
  /* sms received sucessfully */
  TP_STATUS_RECEIVED_OK                   = 0x00,
  TP_STATUS_UNABLE_TO_CONFIRM_DELIVERY    = 0x01,
  TP_STATUS_REPLACED                      = 0x02,
  /* Reserved: 0x03 - 0x0f */
  /* Values specific to each SC: 0x10 - 0x1f */
  /* Temporary error, SC still trying to transfer SM: */
  TP_STATUS_TRY_CONGESTION             = 0x20,
  TP_STATUS_TRY_SME_BUSY               = 0x21,
  TP_STATUS_TRY_NO_RESPONSE_FROM_SME   = 0x22,
  TP_STATUS_TRY_SERVICE_REJECTED       = 0x23,
  TP_STATUS_TRY_QOS_NOT_AVAILABLE      = 0x24,
  TP_STATUS_TRY_SME_ERROR              = 0x25,
  /* Reserved: 0x26 - 0x2f */
  /* Values specific to each SC: 0x30 - 0x3f */
  /* Permanent error, SC is not making any more transfer attempts:  */
  TP_STATUS_PERM_REMOTE_PROCEDURE_ERROR   = 0x40,
  TP_STATUS_PERM_INCOMPATIBLE_DEST        = 0x41,
  TP_STATUS_PERM_REJECTED_BY_SME          = 0x42,
  TP_STATUS_PERM_NOT_OBTAINABLE           = 0x43,
  TP_STATUS_PERM_QOS_NOT_AVAILABLE        = 0x44,
  TP_STATUS_PERM_NO_INTERWORKING          = 0x45,
  TP_STATUS_PERM_VALID_PER_EXPIRED        = 0x46,
  TP_STATUS_PERM_DELETED_BY_ORIG_SME      = 0x47,
  TP_STATUS_PERM_DELETED_BY_SC_ADMIN      = 0x48,
  TP_STATUS_PERM_SM_NO_EXIST              = 0x49,
  /* Reserved: 0x4a - 0x4f */
  /* Values specific to each SC: 0x50 - 0x5f */
  /* Temporary error, SC is not making any more transfer attempts: */
  TP_STATUS_TMP_CONGESTION               = 0x60,
  TP_STATUS_TMP_SME_BUSY                 = 0x61,
  TP_STATUS_TMP_NO_RESPONSE_FROM_SME     = 0x62,
  TP_STATUS_TMP_SERVICE_REJECTED         = 0x63,
  TP_STATUS_TMP_QOS_NOT_AVAILABLE        = 0x64,
  TP_STATUS_TMP_SME_ERROR                = 0x65,
  /* Reserved: 0x66 - 0x6f */
  /* Values specific to each SC: 0x70 - 0x7f */
  /* Reserved: 0x80 - 0xff */
  TP_STATUS_NONE = 0xFF
};


/* Refer to GSM 03.40 subclause 9.2.2.2 */
#define LGSM_SMS_ADDR_MAXLEN	12
#define LGSM_SMS_DATA_MAXLEN	140
struct lgsm_sms {
	char addr[LGSM_SMS_ADDR_MAXLEN+1];
	/* FIXME: max length of data,
	 * 7 bit coding - 160(140*8/7); ucs2 coding - 70(140/2) */
	enum gsmd_sms_alphabet alpha;
	u_int8_t data[LGSM_SMS_DATA_MAXLEN+1];
	int length;
	int ask_ds;
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
extern int lgsm_sms_delete(struct lgsm_handle *lh,
		const struct lgsm_sms_delete *sms_del);

/* Send Message */
extern int lgsm_sms_send(struct lgsm_handle *lh, const struct lgsm_sms *sms);

/* Write Message to Memory */
extern int lgsm_sms_write(struct lgsm_handle *lh,
		const struct lgsm_sms_write *sms_write);

/* Retrieve SMS storage information */
extern int lgsm_sms_get_storage(struct lgsm_handle *lh);

/* Set preferred SMS storage */
extern int lgsm_sms_set_storage(struct lgsm_handle *lh,
		enum ts0705_mem_type mem1, enum ts0705_mem_type mem2,
		enum ts0705_mem_type mem3);
extern int lgsm_number2addr(struct gsmd_addr *dst, const char *src,
	int skipplus);

/* Retrieve current default service centre address */
extern int lgsm_sms_get_smsc(struct lgsm_handle *lh);

/* Set new default service centre address */
extern int lgsm_sms_set_smsc(struct lgsm_handle *lh, const char *number);

/* Packing of 7-bit characters, refer to GSM 03.38 subclause 6.1.2.1.1 */
extern int packing_7bit_character(const char *src, struct lgsm_sms *dest);

/* Packing of 7-bit characters, refer to GSM 03.38 subclause 6.1.2.1.1 */
extern int unpacking_7bit_character(const struct gsmd_sms *src, char *dest);
extern int cbm_unpacking_7bit_character(const char *src, char *dest);

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

/* This phone wants to receive Cell Broadcast Messages */
extern int lgsm_cb_subscribe(struct lgsm_handle *lh);

/* This phone wants no more Cell Broadcast Messages */
extern int lgsm_cb_unsubscribe(struct lgsm_handle *lh);

#endif
