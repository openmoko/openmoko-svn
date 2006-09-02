/* TI [Calypso] compatible backend */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "vendorplugin.h"

static int 
ti_getopt(struct gsmd *gh, int optname, void *optval, int *optlen)
{
	switch (optname) {
	case GSMD_OPT_CIPHER_IND:
		/* FIXME: send AT%CPRI=? */
		break;
	default:
		return -EINVAL;
	}
}

static int 
ti_setopt(struct gsmd *gh, int optname, const void *optval, int optlen)
{
	switch (optname) {
	case GSMD_OPT_CIPHER_IND:
		/* FIXME: send AT%CPRI= */
		break;
	default:
		return -EINVAL;
	}
}

static int ti_parseunsolicit(struct gsmd *gh)
{
	/* FIXME: parse all the below and generate the respective events */

	/* %CPROAM: CPHS Home Country Roaming Indicator */
	/* %CPVWI: CPHS Voice Message Waiting */
	/* %CGREG: reports extended information about GPRS registration state */
	/* %CTZV: reports network time and date information */
	/* %CNIV: reports network name information */
	/* %CPKY: Press Key */
	/* %CMGRS: Message Retransmission Service */
	/* %CGEV: reports GPRS network events */
	return -EINVAL;
}

struct gsmd_vendorspecific ti_vendorspec = {
	.getopt	= &ti_getopt,
	.setopt = &ti_setopt,
	.parse_unsolicit = &ti_parseunsolicit,
};
