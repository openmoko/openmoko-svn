#ifndef _LIBGSMD_VCALL_H
#define _LIBGSMD_VCALL_H

#include <libgsmd/libgsmd.h>

/* Voice Calls */

/* 
 * call related supplementary services from 3GPP TS 02.30 4.5.5.1 
 * R - Release
 * A - Accept
 * H - Hold
 * M - Multiparty
 */
enum lgsm_voicecall_ctrl_proc {
	LGSM_VOICECALL_CTRL_R_HLDS			= 0,	// 0
	LGSM_VOICECALL_CTRL_UDUB			= 1,	// 0
	LGSM_VOICECALL_CTRL_R_ACTS_A_HLD_WAIT		= 2,	// 1	
	LGSM_VOICECALL_CTRL_R_ACT_X			= 3,	// 1x
	LGSM_VOICECALL_CTRL_H_ACTS_A_HLD_WAIT		= 4,	// 2
	LGSM_VOICECALL_CTRL_H_ACTS_EXCEPT_X		= 5,	// 2x
	LGSM_VOICECALL_CTRL_M_HELD			= 6,	// 3
};

/* Refer to GSM 07.07 subclause 7.12 and 02.30 subclause 4.5.5.1 */
struct lgsm_voicecall_ctrl {
	enum lgsm_voicecall_ctrl_proc proc;	
	int idx;
};

/* Initiate an outgoing voice call */
extern int lgsm_voice_out_init(struct lgsm_handle *lh, 
			       const struct lgsm_addr *number);

/* Accept incoming voice call */
extern int lgsm_voice_in_accept(struct lgsm_handle *lh);

/* Terminate outgoing (or incoming) voice call */
extern int lgsm_voice_hangup(struct lgsm_handle *lh);

/* Send DTMF character during voice call */
extern int lgsm_voice_dtmf(struct lgsm_handle *lh, char dtmf_char);

/* Get call status */
extern int lgsm_voice_get_status(struct lgsm_handle *lh); 

/* Call control */
extern int lgsm_voice_ctrl(struct lgsm_handle *lh, 
			       const struct lgsm_voicecall_ctrl *ctrl);

#endif
