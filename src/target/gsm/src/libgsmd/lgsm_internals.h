#ifndef _LGSM_INTERNALS_H
#define _LGSM_INTERNALS_H

#include <gsmd/usock.h>
#include <libgsmd/misc.h>

struct lgsm_handle {
	int fd;
	lgsm_msg_handler *handler[__NUM_GSMD_MSGS];
	enum lgsm_netreg_state netreg_state;
};

int lgsm_send(struct lgsm_handle *lh, struct gsmd_msg_hdr *gmh);
int lgsm_send_simple(struct lgsm_handle *lh, int type, int sub_type);
struct gsmd_msg_hdr *lgsm_gmh_fill(int type, int subtype, int payload_len);
#define lgsm_gmh_free(x)	free(x)

#endif /* _LGSM_INTERNALS_H */
