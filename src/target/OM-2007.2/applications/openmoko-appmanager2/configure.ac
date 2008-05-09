AC_PREREQ(2.53)
AC_INIT(openmoko-appmanager, 1.0.0, http://www.openmoko.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/appmanager-window.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

# base deps
PKG_CHECK_MODULES(OPENMOKO, libmokoui2,,
                 AC_MSG_ERROR([*** Required OpenMoko Libraries not installed ***]))
PKG_CHECK_MODULES(LIBOPKG, libopkg = 0.1.3)
PKG_CHECK_MODULES(GTHREAD, gthread-2.0)
LIBS="$LIBS $GTK_LIBS $LIBOPKG_LIBS $GTHREAD_LIBS"
CFLAGS="$CFLAGS $OPENMOKO_CFLAGS $LIBOPKG_CFLAGS -DOPKG_LIB"

# output stuff
AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])
