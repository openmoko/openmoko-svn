AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DBROWSER_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"


AM_CFLAGS  = -Wall -Werror -std=c99 @OPENMOKO_CFLAGS@ @WEBKITGTK_CFLAGS@ @GTHREAD_CFLAGS@

bin_PROGRAMS = openmoko-browser

EXTRA_DIST = browser-data.h current-page.h
openmoko_browser_SOURCES = main.c current-page.c
openmoko_browser_LDADD = @OPENMOKO_LIBS@ @WEBKITGTK_LIBS@ @GTHREAD_LIBS@

CLEANFILES = $(stamp_files)
DISTCLEANFILES = $(stamp_files)
MAINTAINERCLEANFILES = $(stamp_files)
