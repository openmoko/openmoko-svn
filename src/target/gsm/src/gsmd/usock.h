#ifndef __GSMD_USOCK_H
#define __GSMD_USOCK_H

#include <gsmd/usock.h>

int usock_init(struct gsmd *g);

struct gsmd_ucmd {
	struct llist_head list;
	struct gsmd_msg_hdr hdr;
	char buf[];
} __attribute__ ((packed));

#endif

