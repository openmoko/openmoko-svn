AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -std=c99 -pedantic @OPENMOKO_CFLAGS@

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libopenmoko-panel-demo.la

libopenmoko_panel_demo_la_SOURCES = main.c demo-panel-applet.c

MAINTAINERCLEANFILES = Makefile.in

