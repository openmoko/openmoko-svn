/* libgsmd tool
 *
 * (C) 2006-2007 by OpenMoko, Inc.
 * Written by Harald Welte <laforge@openmoko.org>
 * All Rights Reserved
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

#include <stdio.h>
#include <string.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/event.h>
#include <libgsmd/pin.h>

#include "shell.h"

#define PIN_SIZE 8

static const char *pin;
static char pinbuf[PIN_SIZE+1];
static char pinbuf2[PIN_SIZE+1];

static int pin_handler(struct lgsm_handle *lh,
		int evt, struct gsmd_evt_auxdata *aux)
{
	int rc;
	int type = aux->u.pin.type;
	char *newpin = NULL;

	printf("EVENT: PIN request (type='%s') ", lgsm_pin_name(type));

	/* FIXME: read pin from STDIN and send it back via lgsm_pin */
	if (type == 1 && pin) {
		printf("Auto-responding with pin `%s'\n", pin);
		pending_responses ++;
		return lgsm_pin(lh, type, pin, NULL);
	} else {
		do {
			printf("Please enter %s: ", lgsm_pin_name(type));
			rc = fscanf(stdin, "%8s", pinbuf);
		} while (rc < 1);

		switch (type) {
		case GSMD_PIN_SIM_PUK:
		case GSMD_PIN_SIM_PUK2:
			do {
				printf("Please enter new PIN: ");
				rc = fscanf(stdin, "%8s", pinbuf2);
				newpin = pinbuf2;
			} while (rc < 1);
			break;
		default:
			break;
		}

		pending_responses ++;
		return lgsm_pin(lh, type, pinbuf, newpin);
	}
}

int pin_init(struct lgsm_handle *lh, const char *pin_preset)
{
	pin = pin_preset;
	return lgsm_evt_handler_register(lh, GSMD_EVT_PIN, &pin_handler);
}

