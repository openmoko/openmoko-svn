AC_PREREQ(2.53)
AC_INIT(openmoko-panel-mainmenu, %(1.0.0), http://www.openmoko.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/openmoko-panel-mainmenu.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

# base deps
PKG_CHECK_MODULES(OPENMOKO, openmoko-libs >= 0.0.2,,
                 AC_MSG_ERROR([*** Required OpenMoko Libraries >= 0.0.2 not installed ***]))
PKG_CHECK_MODULES(LIBMB, libmb,,
                 AC_MSG_ERROR([matchbox-lib required]))
PKG_CHECK_MODULES(LIBSN, libstartup-notification-1.0, ,
                 AC_MSG_ERROR([*** Required Startup Notification library not installed ***]))

#AC_SUBST(SN_LIBS)
#AC_SUBST(SN_CFLAGS)

LIBS="$LIBS $OPENMOKO_LIBS $LIBMB_LIBS $LIBSN_LIBS"
CFLAGS="$CFLAGS $OPENMOKO_CFLAGS $LIBMB_CFLAGS $LIBSN_CFLAGS"

# output stuff
AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])
