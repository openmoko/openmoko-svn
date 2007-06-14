AC_PREREQ(2.53)
AC_INIT(openmoko-libs, 0.0.4, http://www.openmoko.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(libmokoui/moko-window.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

GTK_VERSION=2.6
LIBECAL_VERSION=1.4.2

dnl i18n support
GETTEXT_PACKAGE=openmoko-libs
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT


# base deps
PKG_CHECK_MODULES(DEPS,
                  gtk+-2.0 >= $GTK_VERSION
                  libecal-1.2 >= $LIBECAL_VERSION
                  gdk-pixbuf-2.0
                  libgsmd)

AM_PATH_LIBXOSD(have_xosd=true,AC_MSG_ERROR([*** Required libxosd not installed ***]))

if test x$MOKO_DEVEL = xon ; then
    AC_MSG_NOTICE([removing -O2 from CFLAGS])
    CFLAGS="-g -Wall"
fi

LIBS="$LIBS $DEPS_LIBS $LIBXOSD_LIBS"
CFLAGS="$CFLAGS $DEPS_CFLAGS $LIBXOSD_CLFAGS"

AC_SUBST(LIBS)

# foo
#AC_ARG_ENABLE(foo,
#  [  --enable-foo   enable foo support],
#     enable_foo=$enableval, enable_foo=yes )
#
#if test x$enable_startup_notification != xno; then
#  PKG_CHECK_MODULES(SN, libstartup-notification-1.0, ,
#    AC_MSG_ERROR([*** Required Startup Notification library not installed ***]))
#
#  AC_DEFINE(USE_LIBSN, [1], [Has Startup Notification Support])
#fi

# glib-genmarshal
GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)

# output stuff
AC_OUTPUT([
Makefile
openmoko-libs.pc
po/Makefile.in
libmokocore/Makefile
libmokogsmd/Makefile
libmokoui/Makefile
libmokopim/Makefile
libmokonet/Makefile
libmokojournal/Makefile
libmokojournal/src/Makefile
libmokojournal/tests/Makefile
data/Makefile
])

echo "CFLAGS = $CFLAGS"
echo "PREFIX = $prefix"
echo "LIBS   = $LIBS"
