#ifndef __GSMD_EVENT_H
#define __GSMD_EVENT_H

/* event handling */

enum gsmd_event_type {
	GSMD_EVTTYPE_NONE,
	GSMD_EVTTYPE_VOICECALL,
	GSMD_EVTTYPE_DATACALL,
	GSMD_EVTTYPE_SMS,
	GSMD_EVTTYPE_GPRS,
	GSMD_EVTTYPE_CIPHER_IND,
};

enum gsmd_event_call {
	GSMD_EVT_CALL_NONE,
	GSMD_EVT_CALL_HANGUP,		/* any call: hanged up */
	GSMD_EVT_CALL_RING,		/* incoming call: we're ringing */ 
	GSMD_EVT_CALL_BUSY,		/* outgoing call: busy */
	GSMD_EVT_CALL_RINGING,		/* outgoing call: other end ringing */
	GSMD_EVT_CALL_ESTABLISHED,	/* any call: now established */
};

enum gsmd_event_voice {
	/* all of event_call */
};

enum gsmd_event_data {
	/* all of event_call */
};

enum gsmd_event_sms {
	GSMD_EVT_SMS_NONE,
	GSMD_EVT_SMS_RCVD,		/* incoming SMS received */
	GSMD_EVT_SMS_OVERFLOW,		/* sms memory full, can't receive */
};

enum gsmd_event_gprs {
};

enum gsmd_event_cipher {
	GSMD_EVT_CI_NONE,
	GSMD_EVT_CI_ENABLED,		/* cipher enabled */
	GSMD_EVT_CI_DISABLED,		/* cipher disabled */
};

enum gsmd_event_network {
	GSMD_EVT_NW_NONE,
	GSMD_EVT_NW_SIGNAL,		/* signal strength */
	GSMD_EVT_NW_REG,		/* network registration */
};

#endif
