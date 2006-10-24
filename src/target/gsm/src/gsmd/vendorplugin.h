#ifndef __GSMD_VENDORPLUG_H
#define __GSMD_VENDORPLUG_H

#ifdef __GSMD__

#include "gsmd.h"

/* gsmd vendor-specific plugin */

enum gsmd_options {
	GSMD_OPT_NONE,
	GSMD_OPT_CIPHER_IND,
};

/* CIPHER_IND */
enum gsmd_cipher_ind {
	GSMD_CIPHER_IND_OFF,
	GSMD_CIPHER_IND_ON,
	GSMD_CIPHER_IND_SIM_FORBID,
};

struct gsmd_vendorspecific {
	/* callback function to parse unknown unsolicited responses */	
	int (*parse_unsolicit)(void);
	int (*getopt)(struct gsmd *gh, int optname, void *optval, int *optlen);
	int (*setopt)(struct gsmd *gh, int optname, const void *optval, int optlen);
};

/* ciphering indications */

#endif /* __GSMD__ */

#endif
