#ifndef _LGSM_PIN_H
#define _LGSM_PIN_H

extern const char *lgsm_pin_name(enum gsmd_pin_type ptype);

extern int lgsm_pin(struct lgsm_handle *lh, unsigned int type,
		const char *pin, const char *newpin);
/* Get PIN status information */
extern int lgsm_pin_status(struct lgsm_handle *lh);
#endif
