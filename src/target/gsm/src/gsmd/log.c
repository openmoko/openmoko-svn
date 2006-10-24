#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <syslog.h>
#include <time.h>

#include <sys/types.h>

#include "gsmd.h"

#include <gsmd/gsmd.h>

static FILE *logfile;
static FILE syslog_dummy;
static int loglevel;

static int gsmd2syslog[] = {
	[GSMD_DEBUG]	= LOG_DEBUG,
	[GSMD_INFO]	= LOG_INFO,
	[GSMD_NOTICE]	= LOG_NOTICE,
	[GSMD_ERROR]	= LOG_ERR,
	[GSMD_FATAL]	= LOG_CRIT,
};

static inline int gsmd2syslog_level(int level)
{
	if (level >= ARRAY_SIZE(gsmd2syslog))
		return LOG_ERR;

	return gsmd2syslog[level];
}

void __gsmd_log(int level, const char *file, int line, const char *function,
		const char *format, ...)
{
	char *timestr;
	va_list ap;
	time_t tm;
	FILE *outfd;

	if (level < loglevel)
		return;
	
	if (logfile == &syslog_dummy) {
		va_start(ap, format);
		vsyslog(gsmd2syslog_level(level), format, ap);
		va_end(ap);
	} else {
		if (logfile)
			outfd = logfile;
		else
			outfd = stderr;

		tm = time(NULL);
		timestr = ctime(&tm);
		timestr[strlen(timestr)-1] = '\0';
		fprintf(outfd, "%s <%1.1d> %s:%d:%s() ", timestr, level, file, 
			line, function);

		va_start(ap, format);
		vfprintf(outfd, format, ap);
		va_end(ap);

		fflush(outfd);
	}
}

int gsmdlog_init(const char *path)
{
	
	if (!strcmp(path, "syslog")) {
		logfile = &syslog_dummy;
		openlog("gsmd", 0, LOG_DAEMON);
	} else {
		logfile = fopen(path, "a+");
	}

	if (logfile == NULL)
		return -1;
	
	gsmd_log(LOG_INFO, "logfile successfully opened\n");

	return 0;
}
