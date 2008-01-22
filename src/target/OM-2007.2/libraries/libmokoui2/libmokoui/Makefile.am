source_h = \
	moko-stock.h \
	moko-finger-scroll.h \
	moko-search-bar.h \
	moko-hint-entry.h \
	moko-type.h

source_c = \
	moko-stock.c \
	moko-finger-scroll.c \
	moko-search-bar.c \
	moko-hint-entry.c

DATADIR=$(datadir)

AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"

AM_CFLAGS = -Wall -pedantic -std=c99

lib_LTLIBRARIES      = libmokoui2.la
libmokoui2_la_SOURCES     = $(source_c) $(source_h)
libmokoui2_la_LIBADD      =
libmokoui2_la_LDFLAGS = -version-info 0:2:0

libmokouiheadersdir = $(includedir)/libmokoui2
libmokouiheaders_DATA = $(source_h)
