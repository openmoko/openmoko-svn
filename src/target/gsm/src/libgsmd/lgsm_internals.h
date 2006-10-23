#ifndef _LGSM_INTERNALS_H
#define _LGSM_INTERNALS_H

#include <gsmd/usock.h>

struct lgsm_handle {
	int fd;
	lgsm_msg_handler *handler[__NUM_GSMD_MSGS];
};

int lgsm_send(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh);
struct gsmd_msg_hdr *lgsm_gmh_fill(int type, int subtype, int payload_len);
#define lgsm_gmh_free(x)	free(x)

#endif /* _LGSM_INTERNALS_H */
