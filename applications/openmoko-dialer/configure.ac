AC_PREREQ(2.53)
AC_INIT(openmoko-dialer, 0.1, http://www.openmoko.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/dialer-main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

# base deps
PKG_CHECK_MODULES(DIALER, dbus-glib-1 gtk+-2.0 libmokojournal libmokogsmd libmokoui)

old_cflags=$CFLAGS
CFLAGS=$DIALER_CFLAGS
AC_CHECK_TYPE(EContactPhotoType, [], [], [#include <libebook/e-book.h>])
CFLAGS=$old_cflags

if test $ac_cv_type_EContactPhotoType = yes; then
	AC_DEFINE(HAVE_ECONTACT_PHOTO_TYPE, 1, [Defined if EContactPhotoType exists])
fi


LIBS="$DIALER_LIBS  -lm -ldl"

CFLAGS="$DIALER_CFLAGS"

dnl ==============================================
dnl DBUS
dnl ==============================================

DBUS_GLIB_BIN="`$PKG_CONFIG --variable=exec_prefix dbus-glib-1`/bin"

dnl AC_ARG_WITH(dbusbindir,
dnl            AC_HELP_STRING([--with-dbusbindir=[=DIR]],
dnl                           [Use a different path to dbus tools]),,
dnl                           dbusdir=$withval;)

AC_ARG_WITH(dbusbindir,[  --with-dbusbindir=DIR  dbus tools in DIR], [DBUS_GLIB_BIN="$withval"])
echo "DBus tools location ${DBUS_GLIB_BIN}"
AC_SUBST(DBUS_GLIB_BIN)


# output stuff
AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])
