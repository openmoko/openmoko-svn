#ifndef __GSMD_UNSOLICITED_H
#define __GSMD_UNSOLICITED_H

#ifdef __GSMD__

#include <gsmd/gsmd.h>

struct gsmd_unsolicit {
	const char *prefix;
	int (*parse)(char *unsol, int len, const char *param, struct gsmd *gsmd);
};

extern int unsolicited_parse(struct gsmd *g, char *buf, int len, const char *param);
extern int generate_event_from_cme(struct gsmd *g, unsigned int cme_error);

#endif /* __GSMD__ */

#endif
