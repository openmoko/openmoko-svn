INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS  = -DPKGDATADIR=\"$(pkgdatadir)\" \
               -DNEOD_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
               -DDATADIR=\""$(datadir)"\" -D_GNU_SOURCE

AM_CFLAGS    = -Wall -pedantic -std=c99 @GTK_CFLAGS@

bin_PROGRAMS = neod

neod_SOURCES = \
	neod-main.c

MAINTAINERCLEANFILES  = config.h.in Makefile.in

