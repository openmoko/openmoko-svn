INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS  = -DPKGDATADIR=\"$(pkgdatadir)\" 				\
	       -DTODAY_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	       -DDATADIR=\""$(datadir)"\" -D_GNU_SOURCE

AM_CFLAGS    = -Wall -pedantic -std=c99 @TODAY_CFLAGS@

bin_PROGRAMS = openmoko-today

openmoko_today_SOURCES        = today-main.c \
	today.h\
	today-header-box.h\
	today-header-box.c                          \
	today-events.c                          \
	today-events.h                          \
	today-utils.c                          \
	today-utils.h                          \
	today-events-list-store.h                          \
	today-events-list-store.c 

openmoko_today_LDADD          = @TODAY_LIBS@

MAINTAINERCLEANFILES  = config.h.in Makefile.in


















