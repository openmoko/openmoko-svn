AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -std=c99 -pedantic @OPENMOKO_CFLAGS@ @SN_CFLAGS@

appletdir = $(libdir)/matchbox-panel/
applet_LTLIBRARIES = libopenmoko-panel-startup.la

libopenmoko_panel_startup_la_SOURCES = openmoko-panel-startup.c
libopenmoko_panel_startup_la_LIBADD = @OPENMOKO_LIBS@ @SN_LIBS@

MAINTAINERCLEANFILES = Makefile.in

