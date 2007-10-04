AC_PREREQ(2.53)
AC_INIT(libmokojournal2, 0.1.0, http://www.openmoko.org/)
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

# Check for existence of ECalComponentId
old_cflags=$CFLAGS
CFLAGS=$DEPS_CFLAGS
AC_CHECK_TYPE(ECalComponentId, [], [], [#include <libecal/e-cal.h>])

if test $ac_cv_type_ECalComponentId = yes; then
        AC_DEFINE(HAVE_ECALCOMPONENTID, 1, [Defined if ECalComponentId exists])
fi
CFLAGS=$old_cflags

if test x$MOKO_DEVEL = xon ; then
    AC_MSG_NOTICE([removing -O2 from CFLAGS])
    CFLAGS="-g -Wall"
fi

LIBS="$LIBS $DEPS_LIBS"
CFLAGS="$CFLAGS $DEPS_CFLAGS"

AC_SUBST(LIBS)

GTK_DOC_CHECK([1.4])

# output stuff
AC_OUTPUT([
Makefile
libmokojournal2.pc
mokojournal/Makefile
mokojournal-tool/Makefile
tests/Makefile
doc/Makefile
doc/reference/Makefile
doc/reference/version.xml
])

echo "CFLAGS = $CFLAGS"
echo "PREFIX = $prefix"
echo "LIBS   = $LIBS"
