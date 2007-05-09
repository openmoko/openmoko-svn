INCLUDES = 						\
	-I$(top_srcdir)

AM_CPPFLAGS           =-DPKGDATADIR=\"$(pkgdatadir)\" \
-DDATADIR=\""$(datadir)"\"

AM_CFLAGS             = -Wall -pedantic -std=c99 @TODAY_CFLAGS@

bin_PROGRAMS          = openmoko-today

openmoko_today_SOURCES         = today-main.c \
today-events-area.h today-events-area.c \
today-utils.h today-utils.c xutil.c xutil.h

openmoko_today_LDADD          = @TODAY_LIBS@

MAINTAINERCLEANFILES  = config.h.in Makefile.in

