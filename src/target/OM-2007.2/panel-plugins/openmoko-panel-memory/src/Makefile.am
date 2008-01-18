AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall @APPLET_CFLAGS@

appletdir = $(libdir)/matchbox-panel/
applet_LTLIBRARIES = libopenmoko-panel-memory.la

libopenmoko_panel_memory_la_SOURCES = \
	openmoko-panel-memory.c

libopenmoko_panel_memory_la_LIBADD = @APPLET_LIBS@

MAINTAINERCLEANFILES = Makefile.in
