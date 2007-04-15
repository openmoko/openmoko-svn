#ifndef _LIBGSMD_MISC_H
#define _LIBGSMD_MISC_H

/* libgsmd.h - Library API for gsmd, the GSM Daemon
 * (C) 2006 by Harald Welte <hwelte@hmw-consulting.de>
 * Development funded by First International Computers, Inc.
 */

#include <libgsmd/libgsmd.h>

extern int lgsm_phone_power(struct lgsm_handle *lh, int power);

enum lgsm_netreg_state {
	LGSM_NETREG_ST_NOTREG		= 0,
	LGSM_NETREG_ST_REG_HOME		= 1,
	LGSM_NETREG_ST_NOTREG_SEARCH	= 2,
	LGSM_NETREG_ST_DENIED		= 3,
	LGSM_NETREG_ST_UNKNOWN		= 4,
	LGSM_NETREG_ST_REG_ROAMING	= 5,
};

/* Get the current network registration status */
extern int lgsm_get_netreg_state(struct lgsm_handle *lh,
				 enum lgsm_netreg_state *state);

extern int lgsm_netreg_register(struct lgsm_handle *lh, int oper);

enum lgsm_info_type {
	LGSM_INFO_TYPE_NONE		= 0,
	LGSM_INFO_TYPE_MANUF		= 1,
	LGSM_INFO_TYPE_MODEL		= 2,
	LGSM_INFO_TYPE_REVISION		= 3,
	LGSM_INFO_TYPE_SERIAL		= 4,
	LGSM_INFO_TYPE_IMSI		= 5,
};

/* Get some information about the handset */
extern int lgsm_get_info(struct lgsm_handle *lh,
			 enum lgsm_info_type type,
			 char *ret_string, u_int16_t len);

/* Authenticate to SIM Card using specified null-terminated pin */
extern int lgsm_pin_auth(struct lgsm_handle *lh, const char *pin);


/* General Commands */

/* Get Signal Strehngth (Chapter 8.5) */
extern int lgsm_get_signal_quality(struct lgsm_handle *h,
				   unsigned int *rssi);

/* Set voice mail number */
extern int lgsm_voicemail_set(struct lgsm_handle *lh,
			      struct lgsm_addr *addr);

/* Get currently configured voice mail number */
extern int lgsm_voicemail_get(struct lgsm_handle *lh,
			      struct lgsm_addr *addr);

/* Operator Selection, Network Registration */
/* TBD */


/* CLIP, CLIR, COLP, Call Forwarding, Call Waiting, Call Deflecting */
/* TBD */


/* SMS related functions */
/* TBD */


/* GPRS related functions */
/* TBD */


#endif
