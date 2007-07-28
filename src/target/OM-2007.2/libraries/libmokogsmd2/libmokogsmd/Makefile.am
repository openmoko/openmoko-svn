lib_LTLIBRARIES=libmokogsmd2.la

public_headers=moko-gsmd-connection.h
built_sources=moko-gsmd-marshal.h moko-gsmd-marshal.c
stamp_files=stamp-moko-gsmd-marshal.h stamp-moko-gsmd-marshal.c
sources=moko-gsmd-connection.c

$(libmokogsmd2_la_OBJECTS): $(built_sources)
libmokogsmd2_la_SOURCES= $(public_headers) $(sources) $(built_sources)
libmokogsmd2_la_LIBADD=
libmokogsmd2_la_LDFLAGS= -version-info 0:1:0
AM_CFLAGS = -Wall -pedantic -std=c99

libmokogsmd2headersdir=$(includedir)/libmokogsmd2
libmokogsmd2headers_DATA=$(public_headers)


moko-gsmd-marshal.c: stamp-moko-gsmd-marshal.c
	@true
stamp-moko-gsmd-marshal.c: moko-gsmd-marshal.list
	$(GLIB_GENMARSHAL) --prefix=moko_gsmd_marshal $(srcdir)/moko-gsmd-marshal.list --header --body >moko-gsmd-marshal.c \
	&& echo timestamp >$(@F)

moko-gsmd-marshal.h: stamp-moko-gsmd-marshal.h
	@true
stamp-moko-gsmd-marshal.h: moko-gsmd-marshal.list
	$(GLIB_GENMARSHAL) --prefix=moko_gsmd_marshal $(srcdir)/moko-gsmd-marshal.list --header >moko-gsmd-marshal.h \
	&& echo timestamp >$(@F)

CLEANFILES = $(stamp_files) $(built_sources)
DISTCLEANFILES = $(stamp_files) $(built_sources)
MAINTAINERCLEANFILES = $(stamp_files) $(built_sources)
