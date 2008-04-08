INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS  = -DPKGDATADIR=\"$(pkgdatadir)\" \
               -DNEOD_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
               -DDATADIR=\""$(datadir)"\" -D_GNU_SOURCE

AM_CFLAGS    = -Wall -pedantic -std=c99 @NEOD_CFLAGS@

bin_PROGRAMS = neod

neod_SOURCES = \
  buttonactions.c \
  neod-main.c \
  wifi.c \
  wifi.h

neod_LDADD = @NEOD_LIBS@ -lapm

MAINTAINERCLEANFILES  = config.h.in Makefile.in

