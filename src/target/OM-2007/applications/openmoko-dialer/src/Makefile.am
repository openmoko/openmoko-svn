INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @OPENMOKO_CFLAGS@ -Wall -std=c99 -pedantic

bin_PROGRAMS = openmoko-dialer

openmoko_dialer_SOURCES = openmoko-dialer-main.c\
moko-digit-button.c\
moko-dialer-panel.c\
moko-dialer-textview.c\
moko-dialer-tip.c\
moko-dialer-autolist.c\
contacts.c\
common.c

openmoko_dialer_LDADD = @OPENMOKO_LIBS@

