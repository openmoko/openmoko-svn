#include <stdio.h>
#include <string.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/event.h>

#define PIN_SIZE 32

static char *pin;
static char pinbuf[PIN_SIZE + 1];

static int
pin_handler (struct lgsm_handle *lh, int evt, struct gsmd_evt_auxdata *aux)
{
  int rc;

  printf ("EVENT: PIN request (type=%u) ", aux->u.pin.type);

  /* FIXME: read pin from STDIN and send it back via lgsm_pin */
  if (aux->u.pin.type == 1 && pin)
  {
    printf ("Auto-responding with pin `%s'\n", pin);
    lgsm_pin (lh, pin);
  }
  else
  {
#warning FIXME for tony: gsm_pin_require is not found
#if 0
      gsm_pin_require (pinbuf);
#endif
    return lgsm_pin (lh, pinbuf);
  }

  return 0;
}

int
pin_init (struct lgsm_handle *lh, char *pin_preset)
{
  pin = pin_preset;
  return lgsm_evt_handler_register (lh, GSMD_EVT_PIN, &pin_handler);
}
