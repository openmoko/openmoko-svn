
AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" $(GTK_CFLAGS) $(JANA_CFLAGS) $(MOKOUI_CFLAGS) -Wall
AM_LDFLAGS = $(GTK_LIBS) $(JANA_LIBS) $(MOKOUI_LIBS)

bin_PROGRAMS=openmoko-worldclock

openmoko_worldclock_SOURCES = \
	worldclock-main.c \
	worldclock-data.c \
	worldclock-data.h

