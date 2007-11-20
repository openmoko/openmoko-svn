/* TI [Calypso] with HTC firmware machine plugin
 *
 * Written by Philipp Zabel <philipp.zabel@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/usock.h>
#include <gsmd/event.h>
#include <gsmd/talloc.h>
#include <gsmd/extrsp.h>
#include <gsmd/machineplugin.h>

#define N_TIHTC 17

static int tihtc_detect(struct gsmd *g)
{
	/* FIXME: do actual detection of machine if we have multiple machines */
	return 1;
}

static int tihtc_init(struct gsmd *g, int fd)
{
	int ldisc = N_TIHTC;
	int rc;

	/*
	 * Himalaya, Blueangel, Alpine and Magican
	 * power up their GSM chipsets when the
	 * tty is opened. Wait for the "AT-Command
	 * Interpreter ready" message before trying
	 * to send commands.
	 */
	g->interpreter_ready = 0;

	/* Set the line discipline to N_TIHTC */
	rc = ioctl(fd, TIOCSETD, &ldisc);
	if (rc < 0)
		fprintf(stderr, "can't set line discipline\n");

	return rc;
}

struct gsmd_machine_plugin gsmd_machine_plugin = {
	.name = "TI Calypso / HTC firmware",
	.power = NULL,
	.ex_submit = NULL,
	.detect = &tihtc_detect,
	.init = &tihtc_init,
};
