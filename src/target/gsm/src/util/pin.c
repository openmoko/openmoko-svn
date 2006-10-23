#include <stdio.h>
#include <string.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/event.h>

#define PIN_SIZE 32

static char *pin;
static char pinbuf[PIN_SIZE+1];

static int pin_handler(struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
	int rc;

	printf("EVENT: PIN request (type=%u) ", aux->u.pin.type);

	/* FIXME: read pin from STDIN and send it back via lgsm_pin */
	if (aux->u.pin.type == 1 && pin) {
		printf("Auto-responding with pin `%s'\n", pin);
		lgsm_pin(lh, pin);
	} else {
		do {
			printf("Please enter PIN: ");
			rc = fscanf(stdin, "%32s\n", &pinbuf);
		} while (rc < 1);

		return lgsm_pin(lh, pinbuf);
	}

	return 0;
}

int pin_init(struct lgsm_handle *lh, const char *pin_preset)
{
	pin = pin_preset;
	return lgsm_evt_handler_register(lh, GSMD_EVT_PIN, &pin_handler);
}

