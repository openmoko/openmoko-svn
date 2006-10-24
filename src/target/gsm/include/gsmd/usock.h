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

enum gsmd_msg_voicecall_type {
	GSMD_VOICECALL_DIAL	= 1,
	GSMD_VOICECALL_HANGUP	= 2,
	GSMD_VOICECALL_ANSWER	= 3,
};

/* Handset / MT related commands */
enum gsmd_msg_phone_type {
	GSMD_PHONE_VOLUME	= 1,
	GSMD_PHONE_VIBRATOR	= 2,
};

enum gsmd_msg_pin_type {
	GSMD_PIN_INPUT		= 1,
};

enum gsmd_msg_phone {
	GSMD_PHONE_POWERUP	= 1,
	GSMD_PHONE_POWERDOWN	= 2,
};

enum gsmd_msg_network {
	GSMD_NETWORK_REGISTER	= 1,
	GSMD_NETWORK_SIGQ_GET	= 2,
	GSMD_NETWORK_VMAIL_GET	= 3,
	GSMD_NETWORK_VMAIL_SET	= 4,
};

/* Length from 3GPP TS 04.08, Clause 10.5.4.7 */

#define GSMD_ADDR_MAXLEN	32
struct gsmd_addr {
	u_int8_t type;
	char number[GSMD_ADDR_MAXLEN+1];
} __attribute__ ((packed));

struct gsmd_signal_quality {
	u_int8_t rssi;
	u_int8_t ber;
} __attribute__ ((packed));

struct gsmd_voicemail {
	u_int8_t enable;
	struct gsmd_addr addr;
} __attribute__ ((packed));

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
		struct {
			struct gsmd_signal_quality sigq;
		} signal;
	} u;
} __attribute__((packed));

struct gsmd_msg_hdr {
	u_int8_t version;
	u_int8_t msg_type;
	u_int8_t msg_subtype;
	u_int8_t _pad;
	u_int16_t id;
	u_int16_t len;
	u_int8_t data[];
} __attribute__((packed));


#ifdef __GSMD__

#include <common/linux_list.h>

#include <gsmd/usock.h>
#include <gsmd/gsmd.h>

struct gsmd_user;

struct gsmd_ucmd {
	struct llist_head list;
	struct gsmd_msg_hdr hdr;
	char buf[];
} __attribute__ ((packed));

extern int usock_init(struct gsmd *g);
extern void usock_cmd_enqueue(struct gsmd_ucmd *ucmd, struct gsmd_user *gu);
extern struct gsmd_ucmd *usock_build_event(u_int8_t type, u_int8_t subtype, u_int8_t len);
extern int usock_evt_send(struct gsmd *gsmd, struct gsmd_ucmd *ucmd, u_int32_t evt);

#endif /* __GSMD__ */

#endif

