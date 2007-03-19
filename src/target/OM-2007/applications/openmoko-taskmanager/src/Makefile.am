INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @OPENMOKO_CFLAGS@ -Wall -std=c99 -pedantic

bin_PROGRAMS = openmoko-taskmanager

openmoko_taskmanager_SOURCES = taskmanager.c list_view.c callbacks.c \
				               misc.c xatoms.c dbus-conn.c

openmoko_taskmanager_LDADD = @OPENMOKO_LIBS@

