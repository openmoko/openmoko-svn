AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -std=c99 -pedantic @OPENMOKO_CFLAGS@

appletdir = $(libdir)/matchbox-panel/
applet_LTLIBRARIES = libopenmoko-panel-battery.la

libopenmoko_panel_battery_la_SOURCES = openmoko-panel-battery.c
libopenmoko_panel_battery_la_LIBADD = @OPENMOKO_LIBS@ -lapm

MAINTAINERCLEANFILES = Makefile.in

