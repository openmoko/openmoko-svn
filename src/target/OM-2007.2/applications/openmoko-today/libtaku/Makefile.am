INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS = $(GTK_CFLAGS) $(SN_CFLAGS) -DPKGDATADIR=\"$(PKGDATADIR)\"
AM_CFLAGS = -Wall
AM_LDFLAGS = $(GTK_LIBS) $(SN_LIBS)

noinst_LIBRARIES = libtaku.a
libtaku_a_SOURCES = \
	taku-tile.h taku-tile.c \
	taku-icon-tile.h taku-icon-tile.c \
	taku-launcher-tile.h taku-launcher-tile.c \
	taku-table.h taku-table.c \
	launcher-util.h launcher-util.c \
	xutil.h xutil.c \
	eggsequence.h eggsequence.c

# make maintainer-clean should remove *everything* 
MAINTAINERCLEANFILES = \
	aclocal.m4 \
	compile \
	config.guess \
	config.h.in \
	configure \
	depcomp \
	install-sh \
	Makefile.in \
	missing
