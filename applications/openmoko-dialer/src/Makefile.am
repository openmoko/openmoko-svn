AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall -pedantic @CFLAGS@

bin_PROGRAMS = openmoko-dialer

openmoko_dialer_SOURCES = \
	dialer-defines.h		\
	dialer-main.c			\
	moko-contacts.c			\
	moko-contacts.h			\
	moko-dialer.c			\
	moko-dialer.h			\
	moko-dialer-glue.h		\
	moko-dialer-panel.c		\
	moko-dialer-panel.h		\
	moko-dialer-textview.c		\
	moko-dialer-textview.h		\
	moko-digit-button.c		\
	moko-digit-button.h		\
	moko-history.c			\
	moko-history.h			\
	moko-keypad.c			\
	moko-keypad.h			\
	moko-talking.c			\
	moko-talking.h


openmoko_dialer_LDADD = @LIBS@

moko-dialer-glue.h: moko-dialer-dbus.xml
	$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=moko_dialer --mode=glib-server --output=$@ $<
	
BUILT_SOURCES =                         \
        moko-dialer-glue.h

EXTRA_DIST =                            \
        moko-dialer-dbus.xml



