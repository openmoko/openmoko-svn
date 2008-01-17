AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall @DIALER_CFLAGS@ @JANA_CFLAGS@ @NOTIFY_CFLAGS@ @ALSA_CFLAGS@

GLIB_GENMARSHAL = `pkg-config --variable=glib_genmarshal glib-2.0`

INCLUDES = -I$(top_srcdir)/src/common

bin_PROGRAMS = phone-kit

phone_kit_SOURCES = \
	dialer-main.c \
	moko-listener.c moko-listener.h \
	moko-network.c moko-network.h \
	moko-sms.c moko-sms.h \
	moko-dialer.c moko-dialer.h \
	moko-notify.c moko-notify.h \
	moko-sound.c moko-sound.h \
	moko-talking.c moko-talking.h \
	moko-alsa-volume-control.c moko-alsa-volume-control.h \
	moko-alsa-volume-scale.c moko-alsa-volume-scale.h \
	moko-pin.c moko-pin.h \
	moko-mcc-dc.h \
	$(BUILT_SOURCES)

phone_kit_LDADD = @DIALER_LIBS@ @JANA_LIBS@ @NOTIFY_LIBS@ @ALSA_LIBS@ $(top_srcdir)/src/common/libdialer-common.a


%-glue.h: %-dbus.xml
	$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=$(subst -,_,$*) --mode=glib-server --output=$@ $<
	
BUILT_SOURCES = moko-dialer-glue.h moko-sms-glue.h moko-network-glue.h \
	moko-sms-marshal.h moko-sms-marshal.c

STAMP_FILES = stamp-moko-sms-marshal.h

moko-sms-marshal.h: stamp-moko-sms-marshal.h
	@true
stamp-moko-sms-marshal.h: Makefile moko-sms-marshal.list
	$(GLIB_GENMARSHAL) \
		--prefix=_moko_sms_marshal \
		--header \
	$(srcdir)/moko-sms-marshal.list > xgen-tmh && \
	(cmp -s xgen-tmh moko-sms-marshal.h || \
		cp -f xgen-tmh moko-sms-marshal.h) && \
	rm -f xgen-tmh && \
	echo timestamp > $(@F)

moko-sms-marshal.c: Makefile moko-sms-marshal.list
	(echo "#include \"moko-sms-marshal.h\"" ; \
	 $(GLIB_GENMARSHAL) \
		--prefix=_moko_sms_marshal \
		--body \
	 $(srcdir)/moko-sms-marshal.list ) > xgen-tmc && \
	cp -f xgen-tmc moko-sms-marshal.c && \
	rm -f xgen-tmc

CLEANFILES = $(STAMP_FILES) $(BUILT_SOURCES)

EXTRA_DIST = moko-dialer-dbus.xml moko-sms-dbus.xml moko-network-dbus.xml moko-sms-marshal.list

