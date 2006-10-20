#ifndef _GSMD_EVENT_H
#define _GSMD_EVENT_H

enum gsmd_events {
	GSMD_EVT_NONE		= 0,
	GSMD_EVT_IN_CALL	= 1,	/* Incoming call */
	GSMD_EVT_IN_SMS		= 2,	/* Incoming SMS */
	GSMD_EVT_IN_GPRS	= 3,	/* Network initiated GPRS */
	GSMD_EVT_IN_CLIP	= 4,	/* Incoming CLIP */
	GSMD_EVT_NETREG		= 5,	/* Network (un)registration event */
	GSMD_EVT_SIGNAL		= 6,	/* Signal quality event */
	GSMD_EVT_PIN		= 7, 	/* Modem is waiting for some PIN/PUK */
	GSMD_EVT_OUT_STATUS	= 8,	/* Outgoing call status */
	GSMD_EVT_OUT_COLP	= 9,	/* Outgoing COLP */
	GSMD_EVT_CALL_WAIT	= 10,	/* Call Waiting */
	__NUM_GSMD_EVT
};

/* Chapter 8.3 */
enum gsmd_pin_type {			/* waiting for ... */
	GSMD_PIN_NONE		= 0,	/* not for any PIN */
	GSMD_PIN_SIM_PIN	= 1,	/* SIM PIN */
	GSMD_PIN_SIM_PUK	= 2,	/* SIM PUK */
	GSMD_PIN_PH_SIM_PIN	= 3,	/* phone-to-SIM passowrd */
	GSMD_PIN_PH_FSIM_PIN	= 4,	/* phone-to-very-first SIM password */
	GSMD_PIN_PH_FSIM_PUK	= 5,	/* phone-to-very-first SIM PUK password */
	GSMD_PIN_SIM_PIN2	= 6,	/* SIM PIN2 */
	GSMD_PIN_SIM_PUK2	= 7,	/* SIM PUK2 */
	GSMD_PIN_PH_NET_PIN	= 8,	/* netwokr personalisation password */
	GSMD_PIN_PH_NET_PUK	= 9,	/* network personalisation PUK */
	GSMD_PIN_PH_NETSUB_PIN	= 10, 	/* network subset personalisation PIN */
	GSMD_PIN_PH_NETSUB_PUK	= 11,	/* network subset personalisation PUK */
	GSMD_PIN_PH_SP_PIN	= 12,	/* service provider personalisation PIN */
	GSMD_PIN_PH_SP_PUK	= 13,	/* service provider personalisation PUK */
	GSMD_PIN_PH_CORP_PIN	= 14,	/* corporate personalisation PIN */
	GSMD_PIN_PH_CORP_PUK	= 15,	/* corporate personalisation PUK */
	__NUM_GSMD_PIN
};

enum gsmd_call_type {
	GSMD_CALL_NONE		= 0,
	GSMD_CALL_UNSPEC	= 1,
	GSMD_CALL_VOICE		= 2,
	GSMD_CALL_FAX		= 4,
	GSMD_CALL_DATA_SYNC	= 5,
	GSMD_CALL_DATA_REL_ASYNC= 6,
	GSMD_CALL_DATA_REL_SYNC	= 7,
	__NUM_GSMD_CALL
};

enum gsmd_netreg_state {
	GSMD_NETREG_NONE	= 0,
	__NUM_GSMD_NETREG
};

#endif
