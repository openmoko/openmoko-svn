#ifndef _LIBGSMD_EVENT_H
#define _LIBGSMD_EVENT_H

#include <libgsmd/libgsmd.h>

enum lgsm_events {
	LGSM_EVT_NONE		= 0,
	LGSM_EVT_IN_CALL_VOICE	= 1,	/* Incoming VOICE call */
	LGSM_EVT_IN_CALL_DATA	= 2,	/* Incoming DATA call */
	LGSM_EVT_IN_CALL_FAX	= 3,	/* Incoming FAX call */
	LGSM_EVT_IN_SMS		= 4,	/* Incoming SMS */
	LGSM_EVT_IN_GPRS	= 5,	/* Network initiated GPRS */
	LGSM_EVT_NETREG		= 6,	/* Network (un)registration event */
	LGSM_EVT_SIGNAL		= 7,	/* Signal quality event */
	LGSM_EVT_PIN		= 8, 	/* Modem is waiting for some PIN/PUK */
	LGSM_EVT_OUT_STATUS	= 9,	/* Outgoing call status */
};

/* Chapter 8.3 */
enum lgsm_pin_type {			/* waiting for ... */
	LGSM_PIN_NONE		= 0,	/* not for any PIN */
	LGSM_PIN_SIM_PIN	= 1,	/* SIM PIN */
	LGSM_PIN_SIM_PUK	= 2,	/* SIM PUK */
	LGSM_PIN_PH_SIM_PIN	= 3,	/* phone-to-SIM passowrd */
	LGSM_PIN_PH_FSIM_PIN	= 4,	/* phone-to-very-first SIM password */
	LGSM_PIN_PH_FSIM_PUK	= 5,	/* phone-to-very-first SIM PUK password */
	LGSM_PIN_SIM_PIN2	= 6,	/* SIM PIN2 */
	LGSM_PIN_SIM_PUK2	= 7,	/* SIM PUK2 */
	LGSM_PIN_PH_NET_PIN	= 8,	/* netwokr personalisation password */
	LGSM_PIN_PH_NET_PUK	= 9,	/* network personalisation PUK */
	LGSM_PIN_PH_NETSUB_PIN	= 10, 	/* network subset personalisation PIN */
	LGSM_PIN_PH_NETSUB_PUK	= 11,	/* network subset personalisation PUK */
	LGSM_PIN_PH_SP_PIN	= 12,	/* service provider personalisation PIN */
	LGSM_PIN_PH_SP_PUK	= 13,	/* service provider personalisation PUK */
	LGSM_PIN_PH_CORP_PIN	= 14,	/* corporate personalisation PIN */
	LGSM_PIN_PH_CORP_PUK	= 15,	/* corporate personalisation PUK */
};

struct lgsm_evt_auxdata {
	union {
		struct {
			struct lgsm_addr addr;
		} call;
		struct {
			/* TBD */
		} sms;
		struct {
			enum lgsm_pin_type type;
		} pin;
	} u;
};

/* Prototype of libgsmd callback handler function */
typedef int evt_cb_func(struct lgsm_handle *lh, enum lgsm_events evt, 
			void *user);

/* Register an event callback handler with libgsmd */
extern int lgsm_register_evt_cb(struct lgsm_handle *lh, 
				evt_cb_func *cb, void *user);

#endif
