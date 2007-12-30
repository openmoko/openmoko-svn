NULL = 

AUTOMAKE_OPTIONS = subdir-objects

SUBDIRS = \
	po \
	$(NULL)

INCLUDES = \
	$(OPENMOKO_TERMINAL2_CFLAGS) \
	-include $(CONFIG_HEADER) \
	$(NULL)

BUILT_SOURCES = src/openmoko-terminal2.vala.stamp

bin_PROGRAMS = openmoko-terminal2

openmoko_terminal2_VALASOURCES = \
	src/mokoterminal.vala \
	src/mainwindow.vala \
	$(NULL)

openmoko_terminal2_SOURCES = \
	$(openmoko_terminal2_VALASOURCES:.vala=.c) \
	$(openmoko_terminal2_VALASOURCES:.vala=.h) \
	$(NULL)

src/openmoko-terminal2.vala.stamp: $(openmoko_terminal2_VALASOURCES)
	$(VALAC) --pkg vte --basedir $(top_srcdir) $^
	touch $@

openmoko_terminal2_LDADD = \
	$(OPENMOKO_TERMINAL2_LIBS) \
	$(NULL)

EXTRA_DIST = \
	intltool-extract.in \
	intltool-update.in \
	intltool-merge.in \
	$(openmoko_terminal2_VALASOURCES) \
	src/openmoko-terminal2.vala.stamp \
	$(NULL)

DISTCLEANFILES = \
	intltool-extract \
	intltool-update \
	intltool-merge \
	po/.intltool-merge-cache \
	$(NULL)

MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub \
                       configure depcomp install-sh ltmain.sh     \
                       Makefile.in missing config.h.in            \
                       intltool-extract \
                       intltool-merge \
                       intltool-update \
                       mkinstalldirs
