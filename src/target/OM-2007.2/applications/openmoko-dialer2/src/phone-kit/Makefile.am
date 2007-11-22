AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall @DIALER_CFLAGS@ @JANA_CFLAGS@

INCLUDES = -I$(top_srcdir)/src/common

bin_PROGRAMS = phone-kit

phone_kit_SOURCES = \
	dialer-main.c \
	moko-dialer.c moko-dialer.h \
	moko-notify.c moko-notify.h \
	moko-sound.c moko-sound.h \
	moko-talking.c moko-talking.h \
	moko-pin.c \
	moko-pin.h

phone_kit_LDADD = @DIALER_LIBS@ @JANA_LIBS@ $(top_srcdir)/src/common/libdialer-common.a


%-glue.h: %-dbus.xml
	$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=$(subst -,_,$*) --mode=glib-server --output=$@ $<
	
BUILT_SOURCES = moko-dialer-glue.h

EXTRA_DIST = moko-dialer-dbus.xml

