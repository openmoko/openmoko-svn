#ifndef _GSMD_H
#define _GSMD_H

#ifdef __GSMD__

#include <sys/types.h>
#include <sys/time.h>

#include <common/linux_list.h>

#include <gsmd/machineplugin.h>
#include <gsmd/vendorplugin.h>
#include <gsmd/select.h>
#include <gsmd/state.h>

void *gsmd_tallocs;

/* Refer to 3GPP TS 07.07 v 7.8.0, Chapter 4.1 */
#define LGSM_ATCMD_F_EXTENDED	0x01	/* as opposed to basic */
#define LGSM_ATCMD_F_PARAM	0x02	/* as opposed to action */
#define LGSM_ATCMD_F_LFCR	0x04	/* accept LFCR as a line terminator */

typedef struct gsmd_timer * (create_timer_t)(struct gsmd *data);
struct gsmd_atcmd {
	struct llist_head list;
	void *ctx;
	int (*cb)(struct gsmd_atcmd *cmd, void *ctx, char *resp);
	char *resp;
	int32_t ret;
	u_int32_t buflen;
	u_int16_t id;
	u_int8_t flags;
        struct gsmd_timer *timeout;
	create_timer_t * create_timer_func;  
	char *cur;
	char buf[];
};

enum llparse_state {
	LLPARSE_STATE_IDLE,		/* idle, not parsing a response */
	LLPARSE_STATE_IDLE_CR,		/* CR before response (V1) */
	LLPARSE_STATE_IDLE_LF,		/* LF before response (V1) */
	LLPARSE_STATE_RESULT,		/* within result payload */
	LLPARSE_STATE_RESULT_CR,	/* CR after result */
	LLPARSE_STATE_RESULT_LF,	/* LF after result */
	LLPARSE_STATE_PROMPT,		/* within a "> " prompt */
	LLPARSE_STATE_PROMPT_SPC,	/* a complete "> " prompt */
	LLPARSE_STATE_ERROR,		/* something went wrong */
					/* ... idle again */
};

/* we can't take any _single_ response bigger than this: */
#define LLPARSE_BUF_SIZE	1024

/* we can't pare a mutiline response biger than this: */
#define MLPARSE_BUF_SIZE	65535

struct llparser {
	enum llparse_state state;
	unsigned int len;
	unsigned int flags;
	void *ctx;
	int (*cb)(const char *buf, int len, void *ctx);
	int (*prompt_cb)(void *ctx);
	char *cur;
	char buf[LLPARSE_BUF_SIZE];
};

struct gsmd;

#define GSMD_FLAG_V0		0x0001	/* V0 responses to be expected from TA */
#define GSMD_FLAG_SMS_FMT_TEXT	0x0002	/* TODO Use TEXT rather than PDU mode */

#define GSMD_ATCMD_TIMEOUT	60	/* If doesn get respond within 60 secs, discard */

struct gsmd {
	unsigned int flags;
	int interpreter_ready;
	struct gsmd_fd gfd_uart;
	struct gsmd_fd gfd_sock;
	struct llparser llp;
	struct llist_head users;
	struct llist_head pending_atcmds;	/* our busy gsmd_atcmds */
	struct llist_head busy_atcmds;	/* our busy gsmd_atcmds */
	struct gsmd_machine_plugin *machinepl;
	struct gsmd_vendor_plugin *vendorpl;
	struct gsmd_device_state dev_state;

	struct llist_head operators;		/* cached list of operator names */
	unsigned char *mlbuf;		/* ml_parse buffer */
	unsigned int mlbuf_len;
	int mlunsolicited;
	int alive_responded;
	char imsi[16];			/* imsi mem space */
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
void __gsmd_log(int level, const char *file, int line, const char *function, const char *message, ...)
	__attribute__ ((__format__ (__printf__, 5, 6)));
/* macro for logging including filename and line number */
#define gsmd_log(level, format, args ...) \
	__gsmd_log(level, __FILE__, __LINE__, __FUNCTION__, format, ## args)

#define DEBUGP(x, args ...)	gsmd_log(GSMD_DEBUG, x, ## args)

extern int gsmd_simplecmd(struct gsmd *gsmd, char *cmdtxt);
extern int gsmd_initsettings(struct gsmd *gsmd);
extern int gsmd_alive_start(struct gsmd *gsmd);

/***********************************************************************
 * timer handling
 ***********************************************************************/

struct gsmd_timer {
	struct llist_head list;
	struct timeval expires;
	void (*cb)(struct gsmd_timer *tmr, void *data);
	void *data;
};

int gsmd_timer_init(void);
void gsmd_timer_check_n_run(void);
struct gsmd_timer *gsmd_timer_alloc(void);
int gsmd_timer_register(struct gsmd_timer *timer);
void gsmd_timer_unregister(struct gsmd_timer *timer);

struct gsmd_timer *gsmd_timer_create(struct timeval *expires,
				     void (*cb)(struct gsmd_timer *tmr, void *data), void *data);
#define gsmd_timer_free(x) talloc_free(x)
#endif /* __GSMD__ */

#endif /* _GSMD_H */
