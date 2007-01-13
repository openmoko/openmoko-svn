/* libgsmd network related functions
 *
 * (C) 2006-2007 by OpenMoko, Inc.
 * Written by Harald Welte <laforge@openmoko.org>
 * All Rights Reserved
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libgsmd/libgsmd.h>
#include <libgsmd/misc.h>

#include <gsmd/usock.h>
#include <gsmd/event.h>

#include "lgsm_internals.h"

int lgsm_netreg_register(struct lgsm_handle *lh, int oper)
{
	/* FIXME: implement oper selection */
	return lgsm_send_simple(lh, GSMD_MSG_NETWORK, GSMD_NETWORK_REGISTER);
}

int lgsm_signal_quality(struct lgsm_handle *lh)
{
	return lgsm_send_simple(lh, GSMD_MSG_NETWORK, GSMD_NETWORK_SIGQ_GET);
}

int lgsmd_operator_name(struct lgsm_handle *lh)
{
	return lgms_send_simple(lh, GSMD_MSG_NETWORK, GSMD_NETWORK_OPER_GET);
}
