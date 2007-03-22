AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DRSSREADER_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"


AM_CFLAGS  = -Wall -Werror -pedantic -std=c99 @OPENMOKO_CFLAGS@ @MRSS_CFLAGS@

bin_PROGRAMS = openmoko-rssreader

EXTRA_DIST = application-data.h callbacks.h

openmoko_rssreader_SOURCES = main.c callbacks.c
openmoko_rssreader_LDADD = @OPENMOKO_LIBS@ @MRSS_LIBS@ @GTHREAD_LIBS@

