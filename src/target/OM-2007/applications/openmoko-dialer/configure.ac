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
PKG_CHECK_MODULES(DIALER, openmoko-libs >= 0.0.1 alsa libebook-1.2)

old_cflags=$CFLAGS
CFLAGS=$DIALER_CFLAGS
AC_CHECK_TYPE(EContactPhotoType, [], [], [#include <libebook/e-book.h>])
CFLAGS=$old_cflags

if test $ac_cv_type_EContactPhotoType = yes; then
	AC_DEFINE(HAVE_ECONTACT_PHOTO_TYPE, 1, [Defined if EContactPhotoType exists])
fi


LIBS="$DIALER_LIBS  -lm -ldl"

CFLAGS="$DIALER_CFLAGS"

# output stuff
AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])
