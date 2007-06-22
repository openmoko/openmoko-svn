AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -std=c99 -pedantic @OPENMOKO_CFLAGS@ @LIBMB_CFLAGS@ @LIBSN_CFLAGS@

appletdir = $(libdir)/matchbox-panel/
applet_LTLIBRARIES = libopenmoko-panel-mainmenu.la

libopenmoko_panel_mainmenu_la_SOURCES = \
  openmoko-panel-mainmenu.c \
  buttonactions.c \
  buttonactions.h \
  stylusmenu.c \
  stylusmenu.h \
  mokodesktop.c \
  mokodesktop.h \
  mokodesktop_item.c \
  mokodesktop_item.h
libopenmoko_panel_mainmenu_la_LIBADD = @OPENMOKO_LIBS@ @LIBMB_LIBS@ @LIBSN_LIBS@

MAINTAINERCLEANFILES = Makefile.in

