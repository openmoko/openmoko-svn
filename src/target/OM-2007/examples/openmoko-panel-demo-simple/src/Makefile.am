AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -Werror -g -std=c99 -pedantic @MATCHBOX_PANEL_CFLAGS@ @OPENMOKO_CFLAGS@

appletdir = $(libdir)/matchbox-panel/
applet_LTLIBRARIES = libopenmoko-panel-demo-simple.la

libopenmoko_panel_demo_simple_la_SOURCES = demo-panel-applet.c

libopenmoko_panel_demo_simple_la_LIBADD = @OPENMOKO_LIBS@

MAINTAINERCLEANFILES = Makefile.in

