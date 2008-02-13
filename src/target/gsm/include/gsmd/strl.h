#ifndef __GSMD_STRL_H
#define __GSMD_STRL_H

#ifdef __GSMD__

/* safe strcpy and strcat versions */
extern size_t strlcpy(char *dest, const char *src, size_t size);
extern size_t strlcat(char *dest, const char *src, size_t count);

#endif /* __GSMD__ */

#endif
