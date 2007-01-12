#ifndef _LIBGSMD_VCALL_H
#define _LIBGSMD_VCALL_H

#include <libgsmd/libgsmd.h>

/* Voice Calls */

/* Initiate an outgoing voice call */
extern int lgsm_voice_out_init(struct lgsm_handle *lh, 
			       const struct lgsm_addr *number);

/* Accept incoming voice call */
extern int lgsm_voice_in_accept(struct lgsm_handle *lh);

/* Terminate outgoing (or incoming) voice call */
extern int lgsm_voice_hangup(struct lgsm_handle *lh);

/* Send DTMF character during voice call */
extern int lgsm_voice_dtmf(struct lgsm_handle *lh, char dtmf_char);

#endif
