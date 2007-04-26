AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall -pedantic -std=c99 @CFLAGS@

bin_PROGRAMS = openmoko-dialer

openmoko_dialer_SOURCES = \
dialer-callbacks-connection.c\
dialer-main.c\
moko-digit-button.c\
moko-dialer-panel.c\
moko-dialer-textview.c\
moko-dialer-tip.c\
moko-dialer-autolist.c\
history.c\
contacts.c\
common.c\
dialer-window-history.c\
dialer-window-dialer.c\
dialer-window-outgoing.c\
dialer-window-incoming.c\
dialer-window-talking.c\
dialer-window-pin.c\
moko-dialer-status.c\
alsa.c

openmoko_dialer_LDADD = @LIBS@

