#INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @OPENMOKO_CFLAGS@ -Wall -std=c99 -pedantic
AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"

AM_CFLAGS = -Wall -std=c99 -pedantic @OPENMOKO_CFLAGS@

bin_PROGRAMS = openmoko-footer

openmoko_footer_SOURCES = main.c footer.c callbacks.c taskmanager.c misc.c

openmoko_footer_LDADD = @OPENMOKO_LIBS@

