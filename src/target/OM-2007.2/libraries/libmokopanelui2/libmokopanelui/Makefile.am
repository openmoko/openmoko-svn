source_h = \
	moko-panel-applet.h

source_c = \
	moko-panel-applet.c

DATADIR=$(datadir)

AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"

AM_CFLAGS = -Wall -pedantic -std=c99

lib_LTLIBRARIES      = libmokopanelui2.la
libmokopanelui2_la_SOURCES     = $(source_c) $(source_h)
libmokopanelui2_la_LIBADD      =
libmokopanelui2_la_LDFLAGS = -version-info 0:3:0

libmokopanelui2headersdir = $(includedir)/libmokopanelui2
libmokopanelui2headers_DATA = $(source_h)
