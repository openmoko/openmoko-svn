AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DBROWSER_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"


AM_CFLAGS  = -Wall -Werror -std=c99 @OPENMOKO_CFLAGS@ @WEBKITGTK_CFLAGS@ @GTHREAD_CFLAGS@

bin_PROGRAMS = openmoko-browser2

EXTRA_DIST = browser-data.h current-page.h go-page.h open-pages-page.h
openmoko_browser2_SOURCES = main.c current-page.c go-page.c open-pages-page.c
openmoko_browser2_LDADD = @OPENMOKO_LIBS@ @WEBKITGTK_LIBS@ @GTHREAD_LIBS@

CLEANFILES = $(stamp_files)
DISTCLEANFILES = $(stamp_files)
MAINTAINERCLEANFILES = $(stamp_files)
