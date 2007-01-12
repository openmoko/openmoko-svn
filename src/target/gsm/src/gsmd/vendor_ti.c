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


static int csq_parse(char *buf, int len, const char *param,
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
	tok = strtok(buf, ",");
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
	/* FIXME: parse ciphering indication */
}

/* Call Progress Information */
static int cpi_parse(char *buf, int len, const char *param, struct gsmd *gsmd)
{
	char *tok;
	struct gsmd_evt_auxdata *aux;
	struct gsmd_ucmd *ucmd = usock_build_event(GSMD_MSG_EVENT,
						   GSMD_EVT_OUT_STATUS,
						   sizeof(*aux));
	
	DEBUGP("entering cpi_parse param=`%s'\n", param);
	if (!ucmd)
		return -EINVAL;
	
	aux = (struct gsmd_evt_auxdata *) ucmd->buf;

	/* Format: cId, msgType, ibt, tch, dir,[mode],[number],[type],[alpha],[cause],line */

	/* call ID */
	tok = strtok(buf, ",");
	if (!tok)
		goto out_free_io;

	/* message type (layer 3) */
	tok = strtok(NULL, ",");
	if (!tok)
		goto out_free_io;
	aux->u.call_status.prog = atoi(tok);

	/* in-band tones */
	tok = strtok(NULL, ",");
	if (!tok)
		goto out_free_io;

	if (*tok == '1')
		aux->u.call_status.ibt = 1;
	else
		aux->u.call_status.ibt = 0;
	
	/* TCH allocated */
	tok = strtok(NULL, ",");
	if (!tok)
		goto out_free_io;

	if (*tok == '1')
		aux->u.call_status.tch = 1;
	else
		aux->u.call_status.tch = 0;
	
	/* direction */
	tok = strtok(NULL, ",");
	if (!tok)
		goto out_free_io;
	
	switch (*tok) {
	case '0':
	case '1':
	case '2':
	case '3':
		aux->u.call_status.dir = (*tok - '0');
		break;
	default:
		break;
	}

	/* mode */
	tok = strtok(NULL, ",");
	if (!tok)
		goto out_free_io;
	
	usock_evt_send(gsmd, ucmd, GSMD_EVT_OUT_STATUS);

	return 0;

out_free_io:
	free(ucmd);
	return -EIO;
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
	int rc;
	/* enable %CPI: call progress indication */
	rc = gsmd_simplecmd(g, "AT\%CPI=3");
	/* enable %CPRI: ciphering indications */
	rc |= gsmd_simplecmd(g, "AT\%CPRI=1");
	/* enable %CSQ: signal quality reports */
	rc |= gsmd_simplecmd(g, "AT\%CSQ=1");
	/* send unsolicited commands at any time */
	rc |= gsmd_simplecmd(g, "AT\%CUNS=0");

	return rc;
}

static struct gsmd_vendor_plugin plugin_ticalypso = {
	.name = "TI Calypso",
	.num_unsolicit = ARRAY_SIZE(ticalypso_unsolicit),
	.unsolicit = ticalypso_unsolicit,
	.detect = &ticalypso_detect,
	.initsettings = &ticalypso_initsettings,
};
 
/* FIXME: this will be _init() when we make this a plugin */
int ticalypso_init(void)
{
	return gsmd_vendor_plugin_register(&plugin_ticalypso);
}
