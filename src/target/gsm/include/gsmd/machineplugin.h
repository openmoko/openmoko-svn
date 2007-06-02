#ifndef _GSMD_MACHINEPLUG_H
#define _GSMD_MACHINEPLUG_H

#ifdef __GSMD__

#include <common/linux_list.h>
#include <gsmd/gsmd.h>

struct gsmd;

struct gsmd_machine_plugin {
	struct llist_head list;
	unsigned char *name;
	int (*detect)(struct gsmd *g);
	int (*init)(struct gsmd *g, int fd);
};

extern int gsmd_machine_plugin_register(struct gsmd_machine_plugin *pl);
extern void gsmd_machine_plugin_unregister(struct gsmd_machine_plugin *pl);
extern int gsmd_machine_plugin_find(struct gsmd *g);

#endif /* __GSMD__ */

#endif
