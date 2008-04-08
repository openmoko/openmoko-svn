AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall @DIALER_CFLAGS@

AM_LDFLAGS = @DIALER_LIBS@

noinst_LIBRARIES = libdialer-common.a

libdialer_common_a_SOURCES = \
	moko-contacts.c moko-contacts.h \
	moko-dialer-panel.c moko-dialer-panel.h \
	moko-dialer-textview.c moko-dialer-textview.h \
	moko-digit-button.c moko-digit-button.h \
	dialer-defines.h
