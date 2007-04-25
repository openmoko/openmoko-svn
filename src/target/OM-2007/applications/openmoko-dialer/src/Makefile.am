AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall -pedantic -std=c99 @CFLAGS@

bin_PROGRAMS = openmoko-dialer

openmoko_dialer_SOURCES = openmoko-dialer-main.c\
moko-digit-button.c\
moko-dialer-panel.c\
moko-dialer-textview.c\
moko-dialer-tip.c\
moko-dialer-autolist.c\
history.c\
contacts.c\
common.c\
openmoko-dialer-window-history.c\
openmoko-dialer-window-dialer.c\
openmoko-dialer-window-outgoing.c\
openmoko-dialer-window-incoming.c\
openmoko-dialer-window-talking.c\
openmoko-dialer-window-pin.c\
moko-dialer-status.c\
alsa.c

openmoko_dialer_LDADD = @LIBS@

