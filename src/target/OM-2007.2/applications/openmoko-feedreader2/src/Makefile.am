AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DRSSREADER_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"


AM_CFLAGS  = -Wall -Werror -pedantic -std=c99 @OPENMOKO_CFLAGS@ @MRSS_CFLAGS@ @WEBKITGTK_CFLAGS@

bin_PROGRAMS = openmoko-feedreader2

EXTRA_DIST = application-data.h callbacks.h rfcdate.h moko_cache.h

openmoko_feedreader2_SOURCES = main.c callbacks.c rfcdate.c moko_cache.c
openmoko_feedreader2_LDADD = @OPENMOKO_LIBS@ @MRSS_LIBS@ @GTHREAD_LIBS@ @WEBKITGTK_LIBS@

