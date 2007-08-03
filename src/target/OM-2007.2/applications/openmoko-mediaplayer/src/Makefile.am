AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"

#AM_CFLAGS = -Wall -pedantic -std=c99 @OPENMOKO_CFLAGS@
AM_CFLAGS = -Wall -pedantic -std=c99

bin_PROGRAMS = openmoko-mediaplayer

openmoko_mediaplayer_LDADD = \
        $(GTK_LIBS) \
        $(DBUS_LIBS) \
        $(OPENMOKO_LIBS)

openmoko_mediaplayer_CFLAGS = \
        $(GTK_CFLAGS) \
        $(DBUS_CFLAGS) \
        $(OPENMOKO_CFLAGS) \
	$(OMP_DEFINES) \
        -I$(top_srcdir) \
        -I$(top_srcdir)/intl

INCLUDES = $(openmoko_mediaplayer_CFLAGS)

openmoko_mediaplayer_SOURCES = \
        main.c main.h \
	mainwin.c mainwin.h \
	playlist.c playlist.h \
	playback.c playback.h \
	guitools.c guitools.h
