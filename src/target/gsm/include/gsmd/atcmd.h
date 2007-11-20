#ifndef __GSMD_ATCMD_H
#define __GSMD_ATCMD_H

#ifdef __GSMD__

#include <gsmd/gsmd.h>

typedef int atcmd_cb_t(struct gsmd_atcmd *cmd, void *ctx, char *resp);

extern struct gsmd_atcmd *atcmd_fill(const char *cmd, int rlen, atcmd_cb_t *cb, void *ctx, u_int16_t id, 
											   create_timer_t ct);
extern int atcmd_submit(struct gsmd *g, struct gsmd_atcmd *cmd);
extern int cancel_atcmd(struct gsmd *g, struct gsmd_atcmd *cmd);
extern int atcmd_init(struct gsmd *g, int sockfd);
extern void atcmd_drain(int fd);
extern int atcmd_terminate_matching(struct gsmd *g, void *ctx);
extern void atcmd_wake_pending_queue (struct gsmd *g);
extern void atcmd_wait_pending_queue (struct gsmd *g);

#endif /* __GSMD__ */

#endif
