/* Telit GM862 / RS323 machine plugin
 *
 * (c) 2008 Florian Boor <florian@kernelconcepts.de>
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
#include <gsmd/atcmd.h>

#define GSMD_MODEM_POWEROFF_TIMEOUT 6

static void poweroff_timeout(struct gsmd_timer *tmr, void *data) 
{
	exit(0);
}

static struct gsmd_timer *poweroff_timer(struct gsmd *g)
{
	struct timeval tv;
	tv.tv_sec = GSMD_MODEM_POWEROFF_TIMEOUT;
	tv.tv_usec = 0;
	DEBUGP("Create power off timer\n");

	return gsmd_timer_create(&tv, &poweroff_timeout, g);
}

static int telit_detect(struct gsmd *g)
{
	return 1; /* not yet implemented */
}

static int telit_init(struct gsmd *g, int fd)
{
    /* We assume the modem has been turned on manually using the S3
       baseboard or on device startup. */
	g->interpreter_ready = 1;
    
	return 0;
}

static int telit_power(struct gsmd *g, int power)
{
	struct gsmd_atcmd *cmd = NULL;

	switch (power) {
		case GSMD_MODEM_POWERUP:
			break;

		case GSMD_MODEM_POWERDOWN:
			cmd = atcmd_fill("AT#SHDN", 7 + 1, NULL,
					g, 0, poweroff_timer);

			if (!cmd)
				return -ENOMEM;

			llist_add_tail(&cmd->list, &g->pending_atcmds);
			if (llist_empty(&g->busy_atcmds) && 
					!llist_empty(&g->pending_atcmds)) {
				atcmd_wake_pending_queue(g);
			}
			break;

		default:
			return -EINVAL;
	}

	return 0;
}

struct gsmd_machine_plugin gsmd_machine_plugin = {
	.name = "Telit GM862",
	.power = telit_power,
	.ex_submit = NULL,
	.detect = &telit_detect,
	.init = &telit_init,
};
