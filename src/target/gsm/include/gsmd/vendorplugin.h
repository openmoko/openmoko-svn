#ifndef _GSMD_VENDORPLUGIN_H
#define _GSMD_VENDORPLUGIN_H

#ifdef __GSMD__

#include <common/linux_list.h>
#include <gsmd/gsmd.h>

struct gsmd;
struct gsmd_unsolicit;

struct gsmd_vendor_plugin {
	struct llist_head list;
	char *name;
	char *ext_chars;
	unsigned int num_unsolicit;
	const struct gsmd_unsolicit *unsolicit;
	int (*detect)(struct gsmd *g);
	int (*initsettings)(struct gsmd *g);
};

extern int gsmd_vendor_plugin_load(char *name);
extern int gsmd_vendor_plugin_register(struct gsmd_vendor_plugin *pl);
extern void gsmd_vendor_plugin_unregister(struct gsmd_vendor_plugin *pl);
extern int gsmd_vendor_plugin_find(struct gsmd *g);

#endif /* __GSMD__ */

#endif
