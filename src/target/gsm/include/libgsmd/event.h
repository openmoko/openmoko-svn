#ifndef _LIBGSMD_EVENT_H
#define _LIBGSMD_EVENT_H

#include <gsmd/event.h>

/* Prototype of libgsmd callback handler function */
typedef int evt_cb_func(struct lgsm_handle *lh, enum gsmd_events evt, 
			void *user);

/* Register an event callback handler with libgsmd */
extern int lgsm_register_evt_cb(struct lgsm_handle *lh, 
				evt_cb_func *cb, void *user);

#endif
