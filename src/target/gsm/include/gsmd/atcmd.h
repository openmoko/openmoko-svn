#ifndef __GSMD_ATCMD_H
#define __GSMD_ATCMD_H

#ifdef __GSMD__

#include <gsmd/gsmd.h>

typedef int atcmd_cb_t(struct gsmd_atcmd *cmd, void *ctx, char *resp);

extern struct gsmd_atcmd *atcmd_fill(const char *cmd, int rlen, atcmd_cb_t *cb, void *ctx, u_int16_t id);
extern int atcmd_submit(struct gsmd *g, struct gsmd_atcmd *cmd);
extern int cancel_atcmd(struct gsmd *g, struct gsmd_atcmd *cmd);
extern int atcmd_init(struct gsmd *g, int sockfd);
extern void atcmd_drain(int fd);

#endif /* __GSMD__ */

#endif
