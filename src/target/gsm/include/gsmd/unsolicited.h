#ifndef __GSMD_UNSOLICITED_H
#define __GSMD_UNSOLICITED_H

#ifdef __GSMD__

#include <gsmd/gsmd.h>

struct gsmd_unsolicit {
	const char *prefix;
	int (*parse)(const char *unsol,
			int len, const char *param, struct gsmd *gsmd);
};

extern void unsolicited_init(struct gsmd *g);
extern int unsolicited_parse(struct gsmd *g,
		const char *buf, int len, const char *param);
extern int generate_event_from_cme(struct gsmd *g, unsigned int cme_error);
extern void unsolicited_generic_init(struct gsmd *g);
extern int unsolicited_register_array(const struct gsmd_unsolicit *arr,
		int len);
extern int generate_event_from_cms(struct gsmd *g, unsigned int cms_error);
extern const int pintype_from_cme[];

#endif /* __GSMD__ */

#endif
