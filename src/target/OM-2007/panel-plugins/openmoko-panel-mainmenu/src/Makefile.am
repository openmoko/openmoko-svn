AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -std=c99 -pedantic @OPENMOKO_CFLAGS@ @LIBMB_CFLAGS@

appletdir = $(libdir)/matchbox-panel/
applet_LTLIBRARIES = libopenmoko-panel-mainmenu.la

libopenmoko_panel_mainmenu_la_SOURCES = openmoko-panel-mainmenu.c stylusmenu.c mokodesktop.c mokodesktop_item.c
libopenmoko_panel_mainmenu_la_LIBADD = @OPENMOKO_LIBS@ @LIBMB_LIBS@

MAINTAINERCLEANFILES = Makefile.in

