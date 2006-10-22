#ifndef _LGSM_INTERNALS_H
#define _LGSM_INTERNALS_H

#include <gsmd/usock.h>

typedef int lgsm_msg_handler(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh);

struct lgsm_handle {
	int fd;
	lgsm_msg_handler *handler[__NUM_GSMD_MSGS];
};

#endif /* _LGSM_INTERNALS_H */
