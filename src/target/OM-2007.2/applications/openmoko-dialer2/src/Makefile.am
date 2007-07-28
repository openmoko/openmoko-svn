AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall -pedantic -std=c99 @CFLAGS@

bin_PROGRAMS = openmoko-dialer

openmoko_dialer_SOURCES = \
	dialer-callbacks-connection.c\
	dialer-main.c\
	moko-dialer.c\
	moko-digit-button.c\
	moko-dialer-panel.c\
	moko-dialer-textview.c\
	moko-dialer-tip.c\
	moko-dialer-autolist.c\
	contacts.c\
	common.c\
	dialer-window-history.c\
	dialer-window-dialer.c\
	dialer-window-outgoing.c\
	dialer-window-incoming.c\
	dialer-window-talking.c\
	dialer-window-pin.c\
	moko-dialer-status.c\
	alsa.c \
	alsa.h \
	common.h \
	contacts.h \
	dialer-callbacks-connection.h \
	dialer-main.h \
	dialer-window-dialer.h \
	dialer-window-history.h \
	dialer-window-incoming.h \
	dialer-window-outgoing.h \
	dialer-window-pin.h \
	dialer-window-talking.h \
	error.h \
	moko-dialer.h\
	moko-dialer-glue.h\
	moko-dialer-autolist.h \
	moko-dialer-declares.h \
	moko-dialer-includes.h \
	moko-dialer-panel.h \
	moko-dialer-status.h \
	moko-dialer-textview.h \
	moko-dialer-tip.h \
	moko-digit-button.h

openmoko_dialer_LDADD = @LIBS@

moko-dialer-glue.h: moko-dialer-dbus.xml
	$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=moko_dialer --mode=glib-server --output=$@ $<
	
#BUILT_SOURCES =                         \
        moko-dialer-glue.h

EXTRA_DIST =                            \
        moko-dialer-dbus.xml



