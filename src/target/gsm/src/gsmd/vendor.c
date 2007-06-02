/* gsmd vendor plugin core
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

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <common/linux_list.h>

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/vendorplugin.h>

static LLIST_HEAD(vendorpl_list);

int gsmd_vendor_plugin_register(struct gsmd_vendor_plugin *pl)
{
	llist_add(&pl->list, &vendorpl_list);

	return 0;
}

void gsmd_vendor_plugin_unregister(struct gsmd_vendor_plugin *pl)
{
	llist_del(&pl->list);
}

int gsmd_vendor_plugin_find(struct gsmd *g)
{
	struct gsmd_vendor_plugin *pl;

	if (g->vendorpl)
		return -EEXIST;
	
	llist_for_each_entry(pl, &vendorpl_list, list) {
		if (pl->detect(g) == 1) {
			DEBUGP("selecting vendor plugin \"%s\"\n", pl->name);
			g->vendorpl = pl;
			return 1;
		}
	}

	return 0;
}

int gsmd_vendor_plugin_load(char *name)
{
	int rc = -1;
	void *lib;
	struct gsmd_vendor_plugin *pl;
	char buf[PATH_MAX+1];

	DEBUGP("loading vendor plugin \"%s\"\n", name);

	buf[PATH_MAX] = '\0';
	snprintf(buf, sizeof(buf), PLUGINDIR "/libgsmd-vendor_%s.so", name);

	lib = dlopen(buf, RTLD_LAZY);
	if (!lib) {
		fprintf(stderr, "gsmd_vendor_plugin_load: %s\n", dlerror());
		return -1;
	}

	pl = dlsym(lib, "gsmd_vendor_plugin");
	if (pl)
		rc = gsmd_vendor_plugin_register(pl);
	else
		dlclose(lib);

	return rc;
}
