#ifndef _GSMD_USOCK_H
#define _GSMD_USOCK_H

#include <gsmd/event.h>

#define GSMD_UNIX_SOCKET "\0gsmd"
//#define GSMD_UNIX_SOCKET_TYPE SOCK_SEQPACKET
#define GSMD_UNIX_SOCKET_TYPE SOCK_STREAM

#define GSMD_PROTO_VERSION	1

#define GSMD_MSGSIZE_MAX	4096

enum gsmd_msg_type {
	GSMD_MSG_NONE		= 0,
	GSMD_MSG_EVENT		= 1,
	GSMD_MSG_PASSTHROUGH	= 2,
	GSMD_MSG_VOICECALL	= 3,
	GSMD_MSG_DATACALL	= 4,
	GSMD_MSG_PHONEBOOK	= 5,
	GSMD_MSG_NETWORK	= 6,
	GSMD_MSG_PHONE		= 7,
	GSMD_MSG_PIN		= 8,
	__NUM_GSMD_MSGS
};

enum gsmd_passthrough_type {
	GSMD_PASSTHROUGH_NONE	= 0,
	GSMD_PASSTHROUGH_REQ	= 1,
	GSMD_PASSTHROUGH_RESP	= 2,
};

enum gsmd_event_type {
	GSMD_EVENT_NONE		= 0,
	GSMD_EVENT_SUBSCRIPTIONS= 1,
	GSMD_EVENT_HAPPENED	= 2,
};

enum gsmd_msg_voicecall_type {
	GSMD_VOICECALL_DIAL	= 1,
	GSMD_VOICECALL_HANGUP	= 2,
};

/* Handset / MT related commands */
enum gsmd_msg_phone_type {
	GSMD_PHONE_VOLUME	= 1,
	GSMD_PHONE_VIBRATOR	= 2,
};

enum gsmd_msg_pin_type {
	GSMD_PIN_INPUT		= 1,
};

/* Length from 3GPP TS 04.08, Clause 10.5.4.7 */

#define GSMD_ADDR_MAXLEN	13
struct gsmd_addr {
	u_int8_t type;
	char number[GSMD_ADDR_MAXLEN+1];
};

struct gsmd_evt_auxdata {
	union {
		struct {
			enum gsmd_call_type type;
		} call;
		struct {
			struct gsmd_addr addr;
		} clip;
		struct {
			struct gsmd_addr addr;
		} colp;
		struct {
			/* TBD */
			struct gsmd_addr addr;
		} sms;
		struct {
			enum gsmd_pin_type type;
		} pin;
		struct {
			enum gsmd_netreg_state state;
			u_int16_t lac;
			u_int16_t ci;
		} netreg;
		struct {
			u_int8_t tz;
		} timezone;
	} u;
} __attribute__((packed));

struct gsmd_msg_hdr {
	u_int8_t version;
	u_int8_t msg_type;
	u_int8_t msg_subtype;
	u_int8_t _pad;
	u_int16_t id;
	u_int16_t len;
} __attribute__((packed));


#endif
