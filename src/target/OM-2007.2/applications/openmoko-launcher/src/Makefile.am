INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS  = -DPKGDATADIR=\"$(pkgdatadir)\" 				\
	       -DTODAY_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	       -DDATADIR=\""$(datadir)"\" -D_GNU_SOURCE

AM_CFLAGS    = -Wall -pedantic -std=c99 @GTK_CFLAGS@ @MOKOUI_CFLAGS@

bin_PROGRAMS = openmoko-launcher

openmoko_launcher_SOURCES =		\
	launcher-main.c 		\
	launcher.h			\
	today.h				\
	today-utils.c			\
	today-utils.h			\
	today-task-manager.c		\
	today-task-manager.h

openmoko_launcher_LDADD  = \
			$(top_builddir)/libtaku/libtaku.a	\
			@GTK_LIBS@ @SN_LIBS@ @MOKOUI_LIBS@

# Optional inotify support
if HAVE_INOTIFY
openmoko_launcher_LDADD += $(top_builddir)/libtaku/libinotify.a
endif

MAINTAINERCLEANFILES  = config.h.in Makefile.in

