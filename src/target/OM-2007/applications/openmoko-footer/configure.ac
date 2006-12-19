AC_PREREQ(2.53)
AC_INIT(openmoko-footer, 0.0.1, http://openmoko.org)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

# base deps
PKG_CHECK_MODULES(OPENMOKO, openmoko-libs >= 0.0.1,,
                 AC_MSG_ERROR([*** Required OpenMoko Libraries >= 0.0.1 not installed ***]))
PKG_CHECK_MODULES(DBUSGLIB, dbus-glib-1,,
			exit)
PKG_CHECK_MODULES(DBUS, dbus-1,,
			exit)			
LIBS="$LIBS $GTK_LIBS $DBUSGLIB_LIBS $DBUS" 
CFLAGS="$CFLAGS $OPENMOKO_CFLAGS $DBUSGLIB_CFLAGS $DBUS_DFALGS"

# output stuff
AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])
