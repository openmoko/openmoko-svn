AC_PREREQ(2.53)
AC_INIT(libmokojournal, 0.0.4, http://www.openmoko.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(mokojournal/moko-journal.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

LIBECAL_VERSION=1.4.2

# base deps
PKG_CHECK_MODULES(DEPS, libecal-1.2 >= $LIBECAL_VERSION)

if test x$MOKO_DEVEL = xon ; then
    AC_MSG_NOTICE([removing -O2 from CFLAGS])
    CFLAGS="-g -Wall"
fi

LIBS="$LIBS $DEPS_LIBS"
CFLAGS="$CFLAGS $DEPS_CFLAGS"

AC_SUBST(LIBS)

# output stuff
AC_OUTPUT([
Makefile
libmokojournal.pc
mokojournal/Makefile
tests/Makefile
])

echo "CFLAGS = $CFLAGS"
echo "PREFIX = $prefix"
echo "LIBS   = $LIBS"
