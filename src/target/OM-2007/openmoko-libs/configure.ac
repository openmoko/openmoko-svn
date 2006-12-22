AC_PREREQ(2.53)
AC_INIT(openmoko-libs, 0.0.2, http://www.openmoko.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(libmokoui/moko-window.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

# base deps
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.6,,
                 AC_MSG_ERROR([*** Required Gtk+ Library >= 2.6 not installed ***]))
LIBS="$LIBS $GTK_LIBS"
CFLAGS="$CFLAGS $GTK_CFLAGS"

PKG_CHECK_MODULES(LIBMB, libmb >= 1.6,,
                 AC_MSG_ERROR([*** Required Matchbox Library >= 1.6 not installed ***]))
LIBS="$LIBS $LIBMB_LIBS"
CFLAGS="$CFLAGS $LIBMB_CFLAGS"


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
libmokocore/Makefile
libmokoui/Makefile
libmokopim/Makefile
libmokonet/Makefile
data/Makefile
])
