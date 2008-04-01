AC_PREREQ(2.53)
AC_INIT(openmoko-messages, 0.0, http://www.openedhand.com/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/sms-main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL
AC_GNU_SOURCE

PKG_CHECK_MODULES(EBOOK, libebook-1.2)
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.6.0)
PKG_CHECK_MODULES(MOKOUI, libmokoui2 >= 0.3)
PKG_CHECK_MODULES(JANA, libjana libjana-ecal libjana-gtk)
PKG_CHECK_MODULES(DBUS, dbus-glib-1)


# CFLAGS and LIBS definitions for libhito
CONTACTS_CFLAGS="$EBOOK_CFLAGS $GTK_CFLAGS"
CONTACTS_LIBS="$EBOOK_LIBS $GTK_LIBS"
AC_SUBST(CONTACTS_CFLAGS)
AC_SUBST(CONTACTS_LIBS)

old_cflags=$CFLAGS
CFLAGS=$EBOOK_CFLAGS
AC_CHECK_TYPE(EContactPhotoType, [], [], [#include <libebook/e-book.h>])
CFLAGS=$old_cflags

if test $ac_cv_type_EContactPhotoType = yes; then
        AC_DEFINE(HAVE_PHOTO_TYPE, 1, [Defined if EContactPhotoType exists])
fi

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

AC_OUTPUT([
Makefile
libhito/Makefile
src/Makefile
data/Makefile
])

