#ifndef _LIBGSMD_HANDSET_H
#define _LIBGSMD_HANDSET_H

#include <libgsmd/libgsmd.h>
/* Set speaker level (Chapter 8.23) */
extern int lgsm_set_spkr_level(struct lgsm_handle *lh,
			       u_int32_t level);

/* Mute call during voice call */
extern int lgsm_mute_set(struct lgsm_handle *lh, u_int8_t on);

/* Get information on whether voice call is muted or not */
extern int lgsm_mute_get(struct lgsm_handle *lh, u_int8_t *on);

#endif
