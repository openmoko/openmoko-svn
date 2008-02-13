/* gsmd cache of operator names
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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "gsmd.h"

#include <common/linux_list.h>
#include <gsmd/gsmd.h>
#include <gsmd/talloc.h>

static void *__opc_ctx;

struct opname {
	struct llist_head list;
	struct {
		u_int16_t mcc;	/* mobile country code */
		u_int8_t mnc;	/* mobile network code */
	} numeric;
	char alnum_long[16+1];
	//char alnum_short[8+1];
};

/* add an entry to the operator name list, overwrite existing entries for
 * same mcc/mnc */
static int _opname_add(struct gsmd *g, struct opname *op)
{
	struct opname *cur, *cur2;

	llist_for_each_entry_safe(cur, cur2, &g->operators, list) {
		if (op->numeric.mcc == cur->numeric.mcc &&
		    op->numeric.mnc == cur->numeric.mnc) {
			llist_del(&cur->list);
			talloc_free(cur);
		}
	}
	llist_add_tail(&op->list, &g->operators);

	return 0;
}

int gsmd_opname_add(struct gsmd *g, const char *numeric_bcd_string,
		    const char *alnum_long)
{
	struct opname *op;
	char mcc[3+1];
	char mnc[2+1];

	if (strlen(numeric_bcd_string) != 5)
		return -EINVAL;

	op = talloc(__opc_ctx, struct opname);
	if (!op)
		return -ENOMEM;
	
	memset(mcc, 0, sizeof(mcc));
	memset(mnc, 0, sizeof(mnc));

	strncpy(mcc, numeric_bcd_string, 3);
	strncpy(mnc, numeric_bcd_string+3, 2);

	strlcpy(op->alnum_long, alnum_long, sizeof(op->alnum_long));
	op->numeric.mcc = atoi(mcc);
	op->numeric.mnc = atoi(mnc);

	return _opname_add(g, op);
}

int gsmd_opname_init(struct gsmd *g)
{
	INIT_LLIST_HEAD(&g->operators);

	__opc_ctx = talloc_named_const(gsmd_tallocs, 1, "operator_cache");

	return 0;
}
