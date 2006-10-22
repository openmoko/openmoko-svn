#ifndef __GSMD_ATCMD_H
#define __GSMD_ATCMD_H

#include "gsmd.h"

typedef int atcmd_cb_t(struct gsmd_atcmd *cmd, void *ctx, char *resp);

struct gsmd_atcmd *atcmd_fill(const char *cmd, int rlen, atcmd_cb_t *cb, void *ctx, u_int16_t id);
int atcmd_submit(struct gsmd *g, struct gsmd_atcmd *cmd);
int atcmd_init(struct gsmd *g, int sockfd);
void atcmd_drain(int fd);

#endif
