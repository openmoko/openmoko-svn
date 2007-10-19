INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS  = -DPKGDATADIR=\"$(pkgdatadir)\"

AM_CFLAGS    = -Wall @MOKOUI_CFLAGS@

bin_PROGRAMS = openmoko-appearance

openmoko_appearance_SOURCES = \
	appearance.h \
	appearance-main.c \
	appearance-colors.c \
	appearance-colors.h

openmoko_appearance_LDADD  = @MOKOUI_LIBS@

MAINTAINERCLEANFILES  = config.h.in Makefile.in

