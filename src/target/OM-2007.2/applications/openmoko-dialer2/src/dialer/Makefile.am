AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall @DIALER_CFLAGS@ @JANA_CFLAGS@

INCLUDES = -I$(top_srcdir)/src/common -I$(top_srcdir)/libhito

bin_PROGRAMS = openmoko-dialer

openmoko_dialer_SOURCES = \
	moko-history.c moko-history.h \
	moko-keypad.c moko-keypad.h \
	moko-tips.c moko-tips.h \
	dialer-main.c

openmoko_dialer_LDADD = @DIALER_LIBS@ @JANA_LIBS@ \
			$(top_srcdir)/src/common/libdialer-common.a \
			$(top_srcdir)/libhito/libhito.a

