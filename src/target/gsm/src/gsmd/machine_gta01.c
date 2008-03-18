/* gta01 machine plugin
 *
 * (C) 2007 by OpenMoko, Inc.
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

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/usock.h>
#include <gsmd/event.h>
#include <gsmd/talloc.h>
#include <gsmd/extrsp.h>
#include <gsmd/machineplugin.h>
#include <gsmd/atcmd.h>

#define GSMD_MODEM_WAKEUP_TIMEOUT	3
#define GSMD_MODEM_POWEROFF_TIMEOUT	3   	 

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

static int gta01_power(struct gsmd *g, int power)
{
	struct gsmd_atcmd *cmd = NULL;

	switch (power) {
		case GSMD_MODEM_POWERUP:
			break;

		case GSMD_MODEM_POWERDOWN:
			/* After sending "AT@POFF" to GSM modem, modem will
			 * soon power off, and then gsmd gets no response
			 * from modem. */
			cmd = atcmd_fill("AT@POFF", 7 + 1, NULL,
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

static int null_wakeup_cb(struct gsmd_atcmd *cmd, void *ctx, char *resp) 
{
	DEBUGP("The wake up callback!!\n");
	return 0;
}

static void wakeup_timeout(struct gsmd_timer *tmr, void *data) 
{
	struct gsmd *g = data;
	struct gsmd_atcmd *cmd = NULL;
	DEBUGP("Wakeup time out!!\n");
	if (!llist_empty(&g->busy_atcmds)) {
		cmd = llist_entry(g->busy_atcmds.next, struct gsmd_atcmd, list);
	}
	if (!cmd) { 
		DEBUGP("ERROR!! busy_atcmds is NULL\n");
		return;
	}

	if (cmd->timeout != tmr) {
		DEBUGP("ERROR!! cmd->timeout != tmr\n");
		return;
	}

	gsmd_timer_free(cmd->timeout);
	cmd->timeout = NULL;

	/* It's a wakeup command */
	if ( cmd->buf[0] == ' ') {
		llist_del(&cmd->list);
		talloc_free(cmd);
		/* discard the wakeup command, and pass the real command. */
		if (llist_empty(&g->busy_atcmds) &&
				!llist_empty(&g->pending_atcmds)) {
			atcmd_wake_pending_queue(g);
		}
	} else {
		DEBUGP("ERROR!! Wakeup timeout and "
				"cmd->buf is not wakeup command!! %s\n",
				cmd->buf);
	}
}

static struct gsmd_timer * wakeup_timer(struct gsmd *g)
{
	struct timeval tv;
	tv.tv_sec = GSMD_MODEM_WAKEUP_TIMEOUT;
	tv.tv_usec = 0;
	DEBUGP("Create wake up timer\n");

	return gsmd_timer_create(&tv, &wakeup_timeout, g);
}

/* adding a null '\r' before real at command. */
static int atcmd_wakeup_modem(struct gsmd *g) 
{
	DEBUGP("try to wake up\n");
	struct gsmd_atcmd *cmd = atcmd_fill(" \r", 2, null_wakeup_cb,
			g, 0, wakeup_timer);

	llist_add_tail(&cmd->list, &g->pending_atcmds);
	if (llist_empty(&g->busy_atcmds) &&
			!llist_empty(&g->pending_atcmds)) {
		atcmd_wake_pending_queue(g);
	}

	return 0;
}

static int gta01_detect(struct gsmd *g)
{
	/* FIXME: do actual detection of machine if we have multiple machines */
	return 1;
}

static int gta01_init(struct gsmd *g, int fd)
{
	/*
	 * We assume that the GSM chipset can take
	 * input immediately, so we don't have to
	 * wait for the "AT-Command Interpreter ready"
	 * message before trying to send commands.
	 */
	g->interpreter_ready = 1;

	return 0;
}

static int gta01_initsettings(struct gsmd *g)
{
	int rc = 0;
	struct gsmd_atcmd *cmd;

	/* use @AUL: to load audio table */
	rc |= gsmd_simplecmd(g, "AT@AUL=\"0\"");
	/* use @ST: configure the sidetone level */
	rc |= gsmd_simplecmd(g, "AT@ST=\"-26\"");

	return rc;
}

struct gsmd_machine_plugin gsmd_machine_plugin = {
	.name = "TI Calypso / FIC firmware",
	.power = &gta01_power,
	.ex_submit = &atcmd_wakeup_modem,
	.detect = &gta01_detect,
	.init = &gta01_init,
	.initsettings = &gta01_initsettings,
};
