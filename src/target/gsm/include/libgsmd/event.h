#ifndef _LIBGSMD_EVENT_H
#define _LIBGSMD_EVENT_H

#include <gsmd/event.h>

/* Prototype of libgsmd callback handler function */
typedef int lgsm_evt_handler(struct lgsm_handle *lh, int evt_type, struct gsmd_evt_auxdata *aux);

/* Register an event callback handler with libgsmd */
extern int lgsm_evt_handler_register(struct lgsm_handle *lh, int evt_type,
				     lgsm_evt_handler *handler);
extern void lgsm_evt_handler_unregister(struct lgsm_handle *lh, int evt_type);

extern int lgsm_evt_init(struct lgsm_handle *lh);
extern void lgsm_evt_exit(struct lgsm_handle *lh);


#endif
