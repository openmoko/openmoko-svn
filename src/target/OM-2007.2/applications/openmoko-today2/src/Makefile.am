INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS  = -DPKGDATADIR=\"$(pkgdatadir)\" 				\
	       -DTODAY_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	       -DDATADIR=\""$(datadir)"\" -D_GNU_SOURCE

AM_CFLAGS    = -Wall -pedantic -std=c99 @GTK_CFLAGS@ @ECAL_CFLAGS@ \
		@EBOOK_CFLAGS@ @MOKOUI_CFLAGS@ @MOKOJOURNAL_CFLAGS@ \
		@GCONF_CFLAGS@ @JANA_CFLAGS@

bin_PROGRAMS = openmoko-today

openmoko_today_SOURCES =		\
	today-main.c 			\
	today.h				\
	today-utils.c			\
	today-utils.h			\
	today-events-store.h		\
	today-events-store.c		\
	today-tasks-store.c		\
	today-tasks-store.h		\
	today-launcher.c		\
	today-launcher.h		\
	today-task-manager.c		\
	today-task-manager.h           \
	today-pim-journal.c           \
	today-pim-journal.h

openmoko_today_LDADD  = $(top_builddir)/libkoto/libkoto.a 	\
			$(top_builddir)/libtaku/libtaku.a	\
			@GTK_LIBS@ @ECAL_LIBS@ @EBOOK_LIBS@ @SEXY_LIBS@ \
			@SN_LIBS@ @MOKOUI_LIBS@ @MOKOJOURNAL_LIBS@ \
			@GCONF_LIBS@ @JANA_LIBS@

# Optional inotify support
if HAVE_INOTIFY
openmoko_today_LDADD += $(top_builddir)/libtaku/libinotify.a
endif

MAINTAINERCLEANFILES  = config.h.in Makefile.in

