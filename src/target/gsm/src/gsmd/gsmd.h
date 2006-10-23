#ifndef __GSMD_H
#define __GSMD_H

#include <sys/types.h>

#include <common/linux_list.h>

#include "select.h"

/* Refer to 3GPP TS 07.07 v 7.8.0, Chapter 4.1 */
#define LGSM_ATCMD_F_EXTENDED	0x01	/* as opposed to basic */
#define LGSM_ATCMD_F_PARAM	0x02	/* as opposed to action */

struct gsmd_atcmd {
	struct llist_head list;
	void *ctx;
	int (*cb)(struct gsmd_atcmd *cmd, void *ctx, char *resp);
	char *resp;
	int32_t ret;
	u_int32_t buflen;
	u_int16_t id;
	u_int8_t flags;
	char buf[];
};

enum llparse_state {
	LLPARSE_STATE_IDLE,		/* idle, not parsing a response */
	LLPARSE_STATE_IDLE_CR,		/* CR before response (V1) */
	LLPARSE_STATE_IDLE_LF,		/* LF before response (V1) */
	LLPARSE_STATE_RESULT,		/* within result payload */
	LLPARSE_STATE_RESULT_CR,	/* CR after result */
	LLPARSE_STATE_ERROR,		/* something went wrong */
					/* ... idle again */
};

/* we can't take any _single_ response bigger than this: */
#define LLPARSE_BUF_SIZE	256

struct llparser {
	enum llparse_state state;
	unsigned int len;
	unsigned int flags;
	void *ctx;
	int (*cb)(const char *buf, int len, void *ctx);
	char *cur;
	char buf[LLPARSE_BUF_SIZE];
};

#define GSMD_FLAG_V0		0x0001	/* V0 responses to be expected from TA */

struct gsmd {
	unsigned int flags;
	struct gsmd_fd gfd_uart;
	struct gsmd_fd gfd_sock;
	struct llparser llp;
	struct llist_head users;
	struct llist_head pending_atcmds;	/* our busy gsmd_atcmds */
	struct llist_head busy_atcmds;	/* our busy gsmd_atcmds */
};

struct gsmd_user {
	struct llist_head list;		/* our entry in the global list */
	struct llist_head finished_ucmds;	/* our busy gsmd_ucmds */
	struct gsmd *gsmd;
	struct gsmd_fd gfd;				/* the socket */
	u_int32_t subscriptions;		/* bitmaks of subscribed event groups */
};

#define GSMD_DEBUG	1	/* debugging information */
#define GSMD_INFO	3
#define GSMD_NOTICE	5	/* abnormal/unexpected condition */
#define GSMD_ERROR	7	/* error condition, requires user action */
#define GSMD_FATAL	8	/* fatal, program aborted */

extern int gsmdlog_init(const char *path);
/* write a message to the daemons' logfile */
void __gsmd_log(int level, const char *file, int line, const char *function, const char *message, ...);
/* macro for logging including filename and line number */
#define gsmd_log(level, format, args ...) \
	__gsmd_log(level, __FILE__, __LINE__, __FUNCTION__, format, ## args)

#define DEBUGP(x, args ...)	gsmd_log(GSMD_DEBUG, x, ## args)

#endif /* __GSMD_H */
