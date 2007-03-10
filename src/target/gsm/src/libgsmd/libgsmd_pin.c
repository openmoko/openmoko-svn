#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <gsmd/event.h>
#include <libgsmd/libgsmd.h>

static const char *pin_type_names[__NUM_GSMD_PIN] = {
	[GSMD_PIN_NONE]		= "NONE",
	[GSMD_PIN_SIM_PIN]	= "SIM PIN",
	[GSMD_PIN_SIM_PUK]	= "SIM PUK",
	[GSMD_PIN_PH_SIM_PIN]	= "Phone-to-SIM PIN",
	[GSMD_PIN_PH_FSIM_PIN]	= "Phone-to-very-first SIM PIN",
	[GSMD_PIN_PH_FSIM_PUK]	= "Phone-to-very-first SIM PUK",
	[GSMD_PIN_SIM_PIN2]	= "SIM PIN2",
	[GSMD_PIN_SIM_PUK2]	= "SIM PUK2",
	[GSMD_PIN_PH_NET_PIN]	= "Network personalization PIN",
	[GSMD_PIN_PH_NET_PUK]	= "Network personalizaiton PUK",
	[GSMD_PIN_PH_NETSUB_PIN]= "Network subset personalisation PIN",
	[GSMD_PIN_PH_NETSUB_PUK]= "Network subset personalisation PUK",
	[GSMD_PIN_PH_SP_PIN]	= "Service provider personalisation PIN",
	[GSMD_PIN_PH_SP_PUK]	= "Service provider personalisation PUK",
	[GSMD_PIN_PH_CORP_PIN]	= "Corporate personalisation PIN",
	[GSMD_PIN_PH_CORP_PUK]	= "Corporate personalisation PUK",
};

const char *lgsm_pin_name(enum gsmd_pin_type ptype)
{
	if (ptype >= __NUM_GSMD_PIN)
		return "unknown";

	return pin_type_names[ptype];
}

int lgsm_pin(struct lgsm_handle *lh, unsigned int type, char *pin, char *newpin)
{
	int rc;
	struct {
		struct gsmd_msg_hdr gmh;
		struct gsmd_pin gp;
	} __attribute__ ((packed)) *gm;

	if (strlen(pin) > GSMD_PIN_MAXLEN || 
	    (newpin && strlen(newpin) > GSMD_PIN_MAXLEN) ||
	    type >= __NUM_GSMD_PIN)
		return -EINVAL;

	gm = (void *) lgsm_gmh_fill(GSMD_MSG_PIN, GSMD_PIN_INPUT,
				    sizeof(struct gsmd_pin));
	if (!gm)
		return -ENOMEM;

	gm->gp.type = type;

	gm->gp.pin[0] = '\0';
	strcat(gm->gp.pin, pin);

	switch (type) {
	case GSMD_PIN_SIM_PUK:
	case GSMD_PIN_SIM_PUK2:
		/* GSM 07.07 explicitly states that only those two PUK types
		 * require a new pin to be specified! Don't know if this is a
		 * bug or a feature. */
		if (!newpin)
			return -EINVAL;
		gm->gp.newpin[0] = '\0';
		strcat(gm->gp.newpin, newpin);
		break;
	default:
		break;
	}
	printf("sending pin='%s', newpin='%s'\n", gm->gp.pin, gm->gp.newpin);
	rc = lgsm_send(lh, &gm->gmh);
	free(gm);

	return rc;
}
