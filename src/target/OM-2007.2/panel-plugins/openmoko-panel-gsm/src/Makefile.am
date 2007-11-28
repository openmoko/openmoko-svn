AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -std=c99 -pedantic @OPENMOKO_CFLAGS@

appletdir = $(libdir)/matchbox-panel/
applet_LTLIBRARIES = libopenmoko-panel-gsm.la

libopenmoko_panel_gsm_la_SOURCES = \
  openmoko-panel-gsm.c \
  moko-pin.c \
  moko-dialer-textview.c \
  moko-dialer-panel.c \
  moko-digit-button.c 
libopenmoko_panel_gsm_la_LIBADD = @OPENMOKO_LIBS@

MAINTAINERCLEANFILES = Makefile.in

