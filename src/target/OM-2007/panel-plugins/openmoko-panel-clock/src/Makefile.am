AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -std=c99 -pedantic @OPENMOKO_CFLAGS@

appletdir = $(libdir)/matchbox-panel/
applet_LTLIBRARIES = libopenmoko-panel-clock.la

libopenmoko_panel_clock_la_SOURCES = openmoko-panel-clock.c
libopenmoko_panel_clock_la_LIBADD = @OPENMOKO_LIBS@

MAINTAINERCLEANFILES = Makefile.in

