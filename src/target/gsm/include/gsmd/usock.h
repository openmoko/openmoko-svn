#ifndef _GSMD_USOCK_H
#define _GSMD_USOCK_H

#include <gsmd/event.h>

#define GSMD_UNIX_SOCKET "\0gsmd"
#define GSMD_UNIX_SOCKET_TYPE SOCK_SEQPACKET

#define GSMD_PROTO_VERSION	1

enum gsmd_prot_cmd {
	GSMD_PCMD_NONE,
	GSMD_PCMD_EVT_SUBSCRIPTIONS,		/* alter event subscriptions */
	GSMD_PCMD_PASSTHROUGH,			/* transparent atcmd passthrough */
};

enum gsmd_pcmd_result {
	GSMD_PCMD_OK		= 0,
	GSMD_PCMD_ERR_UNSPEC	= 0xff,
};

struct gsmd_prot_hdr {
	u_int16_t cmd;
	u_int8_t result;
	u_int8_t version;
} __attribute__((packed));


enum gsmd_msg_type {
	GSMD_MSG_NONE		= 0,
	GSMD_MSG_EVENT		= 1,
	GSMD_MSG_PASSTHROUGH	= 2,
};

enum gsmd_passthrough_type {
	GSMD_PASSTHROUGH_NONE	= 0,
	GSMD_PASSTHROUGH_REQ	= 1,
	GSMD_PASSTHROUGH_RESP	= 2,
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
	} u;
};

struct gsmd_msg_hdr {
	u_int8_t version;
	u_int8_t msg_type;
	u_int8_t msg_subtype;
	u_int8_t len;
} __attribute__((packed));


#endif
