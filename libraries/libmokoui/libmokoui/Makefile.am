source_h = \
	moko-stock.h

source_c = \
	moko-stock.c

DATADIR=$(datadir)

AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"

AM_CFLAGS = -Wall -pedantic -std=c99

lib_LTLIBRARIES      = libmokoui.la
libmokoui_la_SOURCES     = $(source_c) $(source_h)
libmokoui_la_LIBADD      =
libmokoui_la_LDFLAGS = -version-info 0:2:0

libmokouiheadersdir = $(includedir)/libmokoui
libmokouiheaders_DATA = $(source_h)
