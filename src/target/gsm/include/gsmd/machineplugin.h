#ifndef _GSMD_MACHINEPLUG_H
#define _GSMD_MACHINEPLUG_H

#ifdef __GSMD__

#include <common/linux_list.h>
#include <gsmd/gsmd.h>

struct gsmd;

struct gsmd_machine_plugin {
	struct llist_head list;
	char *name;
	int (*power)(struct gsmd *g, int power);
	int (*ex_submit)(struct gsmd *g);
	int (*detect)(struct gsmd *g);
	int (*init)(struct gsmd *g, int fd);
	int (*initsettings)(struct gsmd *g);
};

extern int gsmd_machine_plugin_init(struct gsmd *g,
		char *machine_name, char *vendor_name);
extern int gsmd_machine_plugin_register(struct gsmd_machine_plugin *pl);
extern void gsmd_machine_plugin_unregister(struct gsmd_machine_plugin *pl);
extern int gsmd_machine_plugin_find(struct gsmd *g);

#endif /* __GSMD__ */

#endif
