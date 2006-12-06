AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"

AM_CFLAGS = -Wall -pedantic -std=c99 @OPENMOKO_CFLAGS@

bin_PROGRAMS = openmoko-chordmaster

openmoko_chordmaster_SOURCES = callbacks.c  chordsdb.c  fretboard-widget.c  main.c

openmoko_chordmaster_LDADD = @OPENMOKO_LIBS@

