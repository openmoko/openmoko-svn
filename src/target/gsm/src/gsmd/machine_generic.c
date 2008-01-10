/* generic machine plugin
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

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/usock.h>
#include <gsmd/event.h>
#include <gsmd/talloc.h>
#include <gsmd/extrsp.h>
#include <gsmd/machineplugin.h>

static int generic_detect(struct gsmd *g)
{
	/* FIXME: do actual detection of machine if we have multiple machines */
	return 1;
}

static int generic_init(struct gsmd *g, int fd)
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

struct gsmd_machine_plugin gsmd_machine_plugin = {
	.name = "generic",
	.power = NULL,
	.ex_submit = NULL,
	.detect = &generic_detect,
	.init = &generic_init,
};
