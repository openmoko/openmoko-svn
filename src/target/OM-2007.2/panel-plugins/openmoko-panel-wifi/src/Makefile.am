AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -pedantic @OPENMOKO_CFLAGS@

appletdir = $(libdir)/matchbox-panel/
applet_LTLIBRARIES = libopenmoko-panel-wifi.la

libopenmoko_panel_wifi_la_SOURCES = openmoko-panel-wifi.c
libopenmoko_panel_wifi_la_LIBADD = @OPENMOKO_LIBS@

MAINTAINERCLEANFILES = Makefile.in

