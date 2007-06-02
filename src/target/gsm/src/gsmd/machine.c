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
	{ "GTA01",		"generic",	"ti" },
	{ "HTC Blueangel",	"tihtc",	"tihtc" },
	{ "HTC Himalaya",	"tihtc",	"tihtc" },
	{ "HTC Magician",	"tihtc",	"tihtc" },
	{ "HTC Universal",	"generic",	"qc" },
	{ NULL, NULL, NULL },
};

int gsmd_machine_plugin_init(struct gsmd *g, int fd)
{
	FILE *cpuinfo;
	char buf[1024];
	char *line, *machine = NULL;
	int i, rc;

	cpuinfo = fopen("/proc/cpuinfo", "r");
	fread(buf, sizeof(buf), 1, cpuinfo);
	fclose(cpuinfo);
	buf[sizeof(buf)-1] = '\0';

	line = strtok(buf, "\n");
	while (line = strtok(NULL, "\n")) {
		if (strncmp(line, "Hardware\t: ", 11) == 0) {
			machine = line+11;
			break;
		}
	}
	/* FIXME: do this dynamically */
	for (i = 0; machines[i].cpuinfo; i++) {
		if (machine && strcmp(machine, machines[i].cpuinfo) == 0) {
			DEBUGP("detected %s\n", machine);
			rc = gsmd_machine_plugin_load(machines[i].machine);
			rc |= gsmd_vendor_plugin_load(machines[i].vendor);
			return rc;
		}
	}
	/* load generic machine and all vendor plugins */
	rc = gsmd_machine_plugin_load("generic");
	gsmd_vendor_plugin_load("ti");
	gsmd_vendor_plugin_load("tihtc");
	gsmd_vendor_plugin_load("qc");
	return rc;
}
