#ifndef _GSMD_STATE_H
#define _GSMD_STATE_H

#ifdef __GSMD__

#define GSMD_CIPHIND_CAPABLE		0x01
#define GSMD_CIPHIND_DISABLED_SIM	0x02
#define GSMD_CIPHIND_ACTIVE		0x04

struct gsmd_device_state {
	struct {
		unsigned int flags;
		unsigned int network_state_gsm;
		unsigned int network_state_gprs;
	} ciph_ind;
	unsigned int vibrator;
	unsigned int on;
	unsigned int registered;
        unsigned int ringing;
        struct gsmd_timer *ring_check;
};

#endif /* __GSMD__ */

#endif /* _GSMD_STATE_H */
