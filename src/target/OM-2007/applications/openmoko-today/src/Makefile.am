INCLUDES = 						\
	-I$(top_srcdir)

AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" \
            -DDATADIR=\""$(datadir)"\"

AM_CFLAGS = -Wall -export-dynamic $(CFLAGS)

bin_PROGRAMS = today

today_SOURCES = today-main.c

today_LDADD   = $(TODAY_LIBS)
today_LDFLAGS = @TODAY_LIBS@

MAINTAINERCLEANFILES = config.h.in Makefile.in
