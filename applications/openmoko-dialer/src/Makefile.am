AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall -pedantic @CFLAGS@

bin_PROGRAMS = openmoko-dialer

openmoko_dialer_SOURCES = \
	dialer-main.c			\
	moko-dialer.c			\
	moko-dialer.h			\
	moko-dialer-glue.h		\
	moko-keypad.c			\
	moko-keypad.h			
	

openmoko_dialer_LDADD = @LIBS@

moko-dialer-glue.h: moko-dialer-dbus.xml
	$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=moko_dialer --mode=glib-server --output=$@ $<
	
BUILT_SOURCES =                         \
        moko-dialer-glue.h

EXTRA_DIST =                            \
        moko-dialer-dbus.xml



