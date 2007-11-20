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

PKG_CHECK_MODULES(CONTACTS, glib-2.0 glib-2.0 gtk+-2.0 libebook-1.2)

old_cflags=$CFLAGS
CFLAGS=$CONTACTS_CFLAGS
AC_CHECK_TYPE(EContactPhotoType, [], [], [#include <libebook/e-book.h>])
CFLAGS=$old_cflags

if test $ac_cv_type_EContactPhotoType = yes; then
        AC_DEFINE(HAVE_PHOTO_TYPE, 1, [Defined if EContactPhotoType exists])
fi

PKG_CHECK_MODULES(EBOOK, libebook-1.2)
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.6.0)
PKG_CHECK_MODULES(MOKOUI, libmokoui2 >= 0.3)
PKG_CHECK_MODULES(JANA, libjana libjana-ecal libjana-gtk)
PKG_CHECK_MODULES(DBUS, dbus-glib-1)

AC_OUTPUT([
Makefile
libhito/Makefile
src/Makefile
])

