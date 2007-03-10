#ifndef _LGSM_PIN_H
#define _LGSM_PIN_H

extern const char *lgsm_pin_name(enum gsmd_pin_type ptype);

extern int lgsm_pin(struct lgsm_handle *lh, unsigned int type, char *pin, char *newpin);

#endif
