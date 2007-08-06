AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DRSSREADER_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"


AM_CFLAGS  = -Wall -Werror -std=c99 @OPENMOKO_CFLAGS@ @MRSS_CFLAGS@ @WEBKITGTK_CFLAGS@

bin_PROGRAMS = openmoko-feedreader2

EXTRA_DIST = rfcdate.h moko_cache.h feed-configuration.h feed-data.h feed-item-view.h feed-selection-view.h application-data.h

built_sources = feed-marshal.h feed-marshal.c
stamp_files = stamp-feed-marshal.h stamp-feed-marshal.c

$(openmoko_feedreader2_OBJECTS): $(built_sources)
openmoko_feedreader2_SOURCES = feed-configuration.c feed-item-view.c feed-selection-view.c feed-data.c rfcdate.c moko_cache.c main.c $(built_sources)
openmoko_feedreader2_LDADD = @OPENMOKO_LIBS@ @MRSS_LIBS@ @GTHREAD_LIBS@ @WEBKITGTK_LIBS@

feed-marshal.c: stamp-feed-marshal.c
	@true
stamp-feed-marshal.c: feed-marshal.list
	$(GLIB_GENMARSHAL) --prefix=feed_marshal $(srcdir)/feed-marshal.list --header --body > feed-marshal.c \
	&& echo timestamp >$(@F)

feed-marshal.h: stamp-feed-marshal.h
	@true

stamp-feed-marshal.h:
	$(GLIB_GENMARSHAL) --prefix=feed_marshal $(srcdir)/feed-marshal.list --header >feed-marshal.h \
	&& echo timestamp >$(@F)

CLEANFILES = $(stamp_files) $(built_sources)
DISTCLEANFILES = $(stamp_files) $(built_sources)
MAINTAINERCLEANFILES = $(stamp_files) $(built_sources)
