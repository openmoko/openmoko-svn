#ifndef __GSMD_SMS_H
#define __GSMD_SMS_H

#ifdef __GSMD__

#include <gsmd/gsmd.h>

int sms_cb_init(struct gsmd *gsmd);
int sms_cb_network_init(struct gsmd *gsmd);

#define SMS_MAX_PDU_SIZE	180
#define CBM_MAX_PDU_SIZE	88
#define CBM_MAX_PDU_PAGES	15
int sms_pdu_make_smssubmit(char *dest, const struct gsmd_sms_submit *src);
int sms_pdu_to_msg(struct gsmd_sms_list *dst, const u_int8_t *src,
		int pdulen, int len);
int cbs_pdu_to_msg(struct gsmd_cbm *dst, u_int8_t *src, int pdulen, int len);

int usock_rcv_sms(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, int len);
int usock_rcv_cb(struct gsmd_user *gu, struct gsmd_msg_hdr *gph, int len);
int sms_pdu_decode_dcs(struct gsmd_sms_datacodingscheme *dcs,
		const u_int8_t *data);
#endif /* __GSMD__ */

#endif
