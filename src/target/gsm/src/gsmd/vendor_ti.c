/* TI [Calypso] compatible backend */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/usock.h>
#include <gsmd/event.h>
#include <gsmd/vendorplugin.h>
#include <gsmd/unsolicited.h>

#if 0
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

#endif


static int csq_parse(char *buf, int len, char *param,
		     struct gsmd *gsmd)
{
	char *tok;
	struct gsmd_evt_auxdata *aux;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT, GSMD_EVT_SIGNAL,
					     sizeof(*aux));

	DEBUGP("entering csq_parse param=`%s'\n", param);
	if (!ucmd)
		return -EINVAL;
	
	
	aux = (struct gsmd_evt_auxdata *) ucmd->buf;
	tok = strtok(param, ",");
	if (!tok)
		goto out_free_io;
	
	aux->u.signal.sigq.rssi = atoi(tok);

	tok = strtok(NULL, ",");
	if (!tok)
		goto out_free_io;

	aux->u.signal.sigq.ber = atoi(tok);

	DEBUGP("sending EVT_SIGNAL\n");
	usock_evt_send(gsmd, ucmd, GSMD_EVT_SIGNAL);

	return 0;

out_free_io:
	free(ucmd);
	return -EIO;
}

static int cpri_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{

}

static int cpi_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{

}

static const struct gsmd_unsolicit ticalypso_unsolicit[] = {
	{ "\%CSQ",	&csq_parse },	/* Signal Quality */
	{ "\%CPRI",	&cpri_parse },	/* Ciphering Indication */
	{ "\%CPI",	&cpi_parse },	/* Call Progress Information */

	/* FIXME: parse all the below and generate the respective events */

	/* %CPROAM: CPHS Home Country Roaming Indicator */
	/* %CPVWI: CPHS Voice Message Waiting */
	/* %CGREG: reports extended information about GPRS registration state */
	/* %CTZV: reports network time and date information */
	/* %CNIV: reports network name information */
	/* %CPKY: Press Key */
	/* %CMGRS: Message Retransmission Service */
	/* %CGEV: reports GPRS network events */
};

static int ticalypso_detect(struct gsmd *g)
{
	/* FIXME: do actual detection of vendor if we have multiple vendors */
	return 1;
}

static int ticalypso_initsettings(struct gsmd *g)
{
	return gsmd_simplecmd(g, "AT\%CPI=3;\%CPRI=1;\%CSQ=1");
}

static struct gsmd_vendor_plugin plugin_ticalypso = {
	.name = "TI Calypso",
	.num_unsolicit = ARRAY_SIZE(ticalypso_unsolicit),
	.unsolicit = &ticalypso_unsolicit,
	.detect = &ticalypso_detect,
	.initsettings = &ticalypso_initsettings,
};
 
/* FIXME: this will be _init() when we make this a plugin */
int ticalypso_init(void)
{
	return gsmd_vendor_plugin_register(&plugin_ticalypso);
}
