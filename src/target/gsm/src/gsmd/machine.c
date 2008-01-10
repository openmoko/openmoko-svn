/* gsmd machine plugin core
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

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <common/linux_list.h>

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/machineplugin.h>

static LLIST_HEAD(machinepl_list);

int gsmd_machine_plugin_register(struct gsmd_machine_plugin *pl)
{
	llist_add(&pl->list, &machinepl_list);

	return 0;
}

void gsmd_machine_plugin_unregister(struct gsmd_machine_plugin *pl)
{
	llist_del(&pl->list);
}

int gsmd_machine_plugin_find(struct gsmd *g)
{
	struct gsmd_machine_plugin *pl;

	if (g->machinepl)
		return -EEXIST;

	llist_for_each_entry(pl, &machinepl_list, list) {
		if (pl->detect(g) == 1) {
			DEBUGP("selecting machine plugin \"%s\"\n", pl->name);
			g->machinepl = pl;
			return 1;
		}
	}

	return 0;
}

int gsmd_machine_plugin_load(char *name)
{
	int rc = -1;
	void *plugin;
	struct gsmd_machine_plugin *pl;
	char buf[128];

	DEBUGP("loading machine plugin \"%s\"\n", name);

	snprintf(buf, sizeof(buf), PLUGINDIR"/libgsmd-machine_%s.so", name);

	plugin = dlopen(buf, RTLD_LAZY);
	if (!plugin) {
		fprintf(stderr, "gsmd_machine_plugin_load: %s\n", dlerror());
		return -1;
	}

	pl = dlsym(plugin, "gsmd_machine_plugin");
	if (pl)
		rc = gsmd_machine_plugin_register(pl);
	else
		dlclose(plugin);

	return rc;
}

/* maybe /etc/gsmd/cpuinfo */
struct machines {
	char *cpuinfo;
	char *machine;
	char *vendor;
} machines[] = {
	{ "GTA01",		"gta01",	"ti" },
	{ "GTA02",		"gta01",	"ti" },
	{ "HTC Blueangel",	"tihtc",	"tihtc" },
	{ "HTC Himalaya",	"tihtc",	"tihtc" },
	{ "HTC Magician",	"tihtc",	"tihtc" },
	{ "HTC Universal",	"generic",	"qc" },
	{ "Palm Treo 650",	"generic",	"bcm" },
	{ NULL, NULL, NULL },
};

int gsmd_machine_plugin_init(struct gsmd *g, char *machine_name, char *vendor_name)
{
	FILE *cpuinfo;
	char buf[1024];
	char *line, *hw = NULL;
	int i, rc;

	cpuinfo = fopen("/proc/cpuinfo", "r");
	fread(buf, sizeof(buf), 1, cpuinfo);
	fclose(cpuinfo);
	buf[sizeof(buf)-1] = '\0';

	line = strtok(buf, "\n");
	while ((line = strtok(NULL, "\n"))) {
		if (strncmp(line, "Hardware\t: ", 11) == 0) {
			hw = line+11;
			break;
		}
	}

	if (hw) {
		/* FIXME: do this dynamically */
		for (i = 0; machines[i].cpuinfo; i++) {
			if (strcmp(hw, machines[i].cpuinfo) == 0) {
				DEBUGP("detected '%s' hardware\n", hw);
				if (machine_name)
					DEBUGP("warning: auto-detected machine '%s', "
						"but user override to '%s'\n",
						machines[i].machine, machine_name);
				else
					machine_name = machines[i].machine;

				if (vendor_name)
					DEBUGP("wanring: auto-detected vendor '%s', "
						"but user override to '%s'\n",
						machines[i].vendor, vendor_name);
				else
					vendor_name = machines[i].vendor;
				break;
			}
		}
	}

	if (machine_name)
		rc = gsmd_machine_plugin_load(machine_name);
	else
		rc = gsmd_machine_plugin_load("generic");
	
	if (vendor_name)
		gsmd_vendor_plugin_load(vendor_name);
	else {
		gsmd_vendor_plugin_load("ti");
		gsmd_vendor_plugin_load("tihtc");
		gsmd_vendor_plugin_load("qc");
		gsmd_vendor_plugin_load("bcm");
	}

	return rc;
}
