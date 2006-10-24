#include <errno.h>

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
			g->vendorpl = pl;
			return 1;
		}
	}

	return 0;
}
