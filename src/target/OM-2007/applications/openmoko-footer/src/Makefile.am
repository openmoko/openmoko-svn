INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @OPENMOKO_CFLAGS@ -Wall -std=c99 -pedantic

bin_PROGRAMS = openmoko-footer

openmoko_footer_SOURCES = main.c footer.c callbacks.c misc.c

openmoko_footer_LDADD = @OPENMOKO_LIBS@

