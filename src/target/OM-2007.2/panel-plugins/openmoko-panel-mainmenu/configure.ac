AC_PREREQ(2.53)
AC_INIT(openmoko-panel-mainmenu, 1.0.0, http://www.openmoko.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/openmoko-panel-mainmenu.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

# base deps
PKG_CHECK_MODULES(OPENMOKO, libmokopanelui2)
PKG_CHECK_MODULES(LIBMB, libmb,,
                 AC_MSG_ERROR([matchbox-lib required]))
PKG_CHECK_MODULES(LIBSN, libstartup-notification-1.0, ,
                 AC_MSG_ERROR([*** Required Startup Notification library not installed ***]))
PKG_CHECK_MODULES(LIBPULSE, libpulse,,
                 AC_MSG_ERROR([*** Required pulseaudio client library not installed ***]))
PKG_CHECK_MODULES(DBUSGLIB, dbus-glib-1,,
                 AC_MSG_ERROR([*** Required dbus-glib-1 not installed ***]))
PKG_CHECK_MODULES(DBUS, dbus-1,,
                 AC_MSG_ERROR([*** Required dbus-1 not installed ***]))
 
LIBS="$LIBS $OPENMOKO_LIBS $LIBMB_LIBS $LIBSN_LIBS $LIBPULSE_LIBS $DBUSGLIB_LIBS $DBUS_LIBS"
CFLAGS="$CFLAGS $OPENMOKO_CFLAGS $LIBMB_CFLAGS $LIBSN_CFLAGS $LIBPULSE_CFLAGS $DBUSGLIB_CFLAGS $DBUS_CFALGS"

# output stuff
AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])
