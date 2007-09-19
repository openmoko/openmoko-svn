AC_PREREQ(2.53)
AC_INIT(libmokoui2, 0.3, http://www.openmoko.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(libmokoui/moko-stock.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

GTK_VERSION=2.6

dnl i18n support
GETTEXT_PACKAGE=libmokoui
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT

# base deps
PKG_CHECK_MODULES(DEPS, gtk+-2.0)

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
libmokoui2.pc
libmokoui/Makefile
doc/Makefile
doc/reference/Makefile
doc/reference/version.xml
])

echo "CFLAGS = $CFLAGS"
echo "PREFIX = $prefix"
echo "LIBS   = $LIBS"
