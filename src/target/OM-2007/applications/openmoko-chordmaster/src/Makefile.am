INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @OPENMOKO_CFLAGS@ -Wall -std=c99 -pedantic

bin_PROGRAMS = openmoko-chordmaster

openmoko_chordmaster_SOURCES = callbacks.c  chordsdb.c  fretboard-widget.c  main.c

openmoko_chordmaster_LDADD = @OPENMOKO_LIBS@

