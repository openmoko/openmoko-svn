#ifndef __GSMD_SMS_H
#define __GSMD_SMS_H

#ifdef __GSMD__

#include <gsmd/gsmd.h>

int sms_cb_init(struct gsmd *gsmd);

int sms_pdu_make_smssubmit(char *dest, const struct gsmd_sms_submit *src);
int sms_pdu_to_msg(struct gsmd_sms_list *dst, const u_int8_t *src,
		int pdulen, int len);

#endif /* __GSMD__ */

#endif
