/*
 * Copyright (c) 2007 OpenMoko, Inc.
 * Modified for use in QEMU by Andrzej Zaborowski <andrew@openedhand.com>
 */
/*

  $Id: virtmodem.c,v 1.49 2006/10/19 16:05:35 dforsi Exp $

  G N O K I I

  A Linux/Unix toolset and driver for the mobile phones.

  This file is part of gnokii.

  Gnokii is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Gnokii is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with gnokii; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Copyright (C) 1999-2000  Hugh Blemings & Pavel Janík ml.
  Copyright (C) 2002       Ladis Michl, Manfred Jonsson, Jan Kratochvil
  Copyright (C) 2001-2004  Pawel Kot
  Copyright (C) 2002-2003  BORBELY Zoltan

  This file provides a virtual modem interface to the GSM phone by calling
  code in gsm-api.c, at-emulator.c and datapump.c. The code here provides
  the overall framework and coordinates switching between command mode
  (AT-emulator) and "online" mode where the data pump code translates data
  from/to the GSM handset and the modem data/fax stream.

*/

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <grp.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/param.h>

#include "compat.h"
#include "misc.h"
#include "gnokii-internal.h"
#include "at-emulator.h"
#include "datapump.h"
#include "device.h"

/* Functions from parts of GNOKII not included in QEMU */
GNOKII_API void gn_data_clear(gn_data *data)
{
	memset(data, 0, sizeof(gn_data));
}

GNOKII_API gn_state gn_sm_loop(int timeout, struct gn_statemachine *state)
{
	return GN_SM_Initialised;
}

GNOKII_API gn_error gn_sm_functions(gn_operation op, gn_data *data,
		struct gn_statemachine *sm)
{
	return sm->info->gn_sm_functions(op, data, sm);
}

GNOKII_API gn_error gn_sms_get(gn_data *data, struct gn_statemachine *state)
{
	return gn_sm_functions(GN_OP_GetSMS, data, state);
}

GNOKII_API gn_error gn_sms_send(gn_data *data, struct gn_statemachine *state)
{
	return gn_sm_functions(GN_OP_SendSMS, data, state);
}

bool GTerminateThread = false;

GNOKII_API gn_error gn_cfg_phone_load(const char *iname,
		struct gn_statemachine *state)
{
	return GN_ERR_NOTSUPPORTED;
}

GNOKII_API gn_error gn_gsm_initialise(struct gn_statemachine *sm)
{
	return GN_ERR_NOTSUPPORTED;
}

/* Defines */

#ifndef AF_LOCAL 
#  ifdef AF_UNIX
#    define AF_LOCAL AF_UNIX
#  else
#    error AF_LOCAL not defined
#  endif
#endif

/* Prototypes */
static int  VM_PtySetup();
static gn_error VM_GSMInitialise(struct gn_statemachine *sm);

/* Global variables */

extern bool GTerminateThread;
int ConnectCount;
bool CommandMode;

/* Local variables */

static int PtyRDFD;	/* File descriptor for reading and writing to/from */
static int PtyWRFD;	/* pty interface - only different in debug mode. */

struct vm_queue queue;

/* If initialised in debug mode, stdin/out is used instead
   of ptys for interface. */
bool gn_vm_initialise(const char *iname, bool GSMInit)
{
	static struct gn_statemachine State;
	sm = &State;
	queue.n = 0;
	queue.head = 0;
	queue.tail = 0;

	CommandMode = true;

	if (GSMInit) {
		dprintf("Initialising GSM\n");
		if (gn_cfg_phone_load(iname, sm) != GN_ERR_NONE) return false;
		if ((VM_GSMInitialise(sm) != GN_ERR_NONE)) {
			fprintf (stderr, _("gn_vm_initialise - VM_GSMInitialise failed!\n"));
			return (false);
		}
	}
	GSMInit = false;

	if (VM_PtySetup() < 0) {
		fprintf (stderr, _("gn_vm_initialise - VM_PtySetup failed!\n"));
		return (false);
	}

	if (gn_atem_initialise(sm) != true) {
		fprintf (stderr, _("gn_vm_initialise - gn_atem_initialise failed!\n"));
		return (false);
	}

	if (dp_Initialise() != true) {
		fprintf (stderr, _("gn_vm_Initialise - dp_Initialise failed!\n"));
		return (false);
	}

	return (true);
}

void gn_vm_loop(void)
{
	fd_set rfds;
	struct timeval tv;
	int res;
	int nfd;
	int i, n;
	char buf[256], *d;

	nfd = PtyRDFD + 1;

	while (!GTerminateThread) {
		if (CommandMode && gn_atem_initialised && queue.n > 0) {
			d = queue.buf + queue.head;
			queue.head = (queue.head + 1) % sizeof(queue.buf);
			queue.n--;
			gn_atem_incoming_data_handle(d, 1);
			continue;
		}

		FD_ZERO(&rfds);
		if ( queue.n < sizeof(queue.buf) ) {
			FD_SET(PtyRDFD, &rfds);
		}
		tv.tv_sec = 0;
		tv.tv_usec = 500000;
		res = select(nfd, &rfds, NULL, NULL, &tv);

		switch (res) {
		case 0: /* Timeout */
			continue;

		case -1:
			perror("gn_vm_loop - select");
			exit (-1);

		default:
			break;
		}

		if (FD_ISSET(PtyRDFD, &rfds)) {
			n = sizeof(queue.buf) - queue.n < sizeof(buf) ?
				sizeof(queue.buf) - queue.n :
				sizeof(buf);
			if ( (n = read(PtyRDFD, buf, n)) <= 0 ) gn_vm_terminate();

			for (i = 0; i < n; i++) {
				queue.buf[queue.tail++] = buf[i];
				queue.tail %= sizeof(queue.buf);
				queue.n++;
			}
		}
	}
}

/* Application should call gn_vm_terminate to shut down
   the virtual modem thread */
void gn_vm_terminate(void)
{
	/* Request termination of thread */
	GTerminateThread = true;

	close (PtyRDFD);
	close (PtyWRFD);

	/* Shutdown device */
	gn_sm_functions(GN_OP_Terminate, NULL, sm);
}

/* Open pseudo tty interface and (in due course create a symlink
   to be /dev/gnokii etc. ) */
static int VM_PtySetup()
{
	PtyRDFD = STDIN_FILENO;
	PtyWRFD = STDOUT_FILENO;
	return (0);
}

/* Initialise GSM interface, returning gn_error as appropriate  */
static gn_error VM_GSMInitialise(struct gn_statemachine *sm)
{
	gn_error error;

	/* Initialise the code for the GSM interface. */
	error = gn_gsm_initialise(sm);

	if (error != GN_ERR_NONE)
		fprintf(stderr, _("GSM/FBUS init failed! (Unknown model?). Quitting.\n"));

	return (error);
}
