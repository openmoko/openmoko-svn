#ifndef _LIBGSMD_H
#define _LIBGSMD_H

/* libgsmd.h - Library API for gsmd, the GSM Daemon
 * (C) 2006 by Harald Welte <hwelte@hmw-consulting.de>
 * Development funded by First International Computers, Inc.
 */

#include <sys/types.h>
#include <errno.h>

/* Generic Information
 *
 * Return value:
 * 	< 0	Error, see libgsmd/errno.h and errno.h
 * 	= 0	Success
 * 	> 0	Success, number of information elements returned
 *
 * Allocation:
 * 	All data structures are caller-allocated.  The only exception
 * 	is struct lgsm_handle which is allocatedi in lgsm_init() and 
 * 	free'd in lgsm_exit()
 *
 * References:
 * 	Recefences to "Chapter X" are referring to 3GPP TS 07.07 version 7.8.0
 */

/* Opaque data structure, content only known to libgsm implementation */
struct lgsm_handle;

#define LGSMD_DEVICE_GSMD	"gsmd"

/* initialize usage of libgsmd, obtain handle for othe API calls */
extern struct lgsm_handle *lgsm_init(const char *device);

/* Terminate usage of libgsmd */
extern int lgsm_exit(struct lgsm_handle *lh); 

/* Obtain file descriptor (e.g. for select-loop under app control) */
extern int lgsm_fd(struct lgsm_handle *lh);

/* Refer to GSM 04.08 [8] subclause 10.5.4.7 */
enum lgsm_addr_type {
	LGSM_ATYPE_ISDN_UNKN		= 161,
	//LGSM_ATYPE_ISDN_INTL		= ,
	//LGSM_ATYPE_ISDN_NATIONAL	= ,
};

#define LGSM_ADDR_MAXLEN	31
struct lgsm_addr {
	char addr[LGSM_ADDR_MAXLEN+1];
	enum lgsm_addr_type tyoe;
};

extern int lgsm_passthrough(struct lgsm_handle *lh, const char *tx, char *rx, unsigned int *rx_len);
#endif
