AC_PREREQ(2.53)
AC_INIT(openmoko-dialer, %(APPVERSION), %(APPCONTACT))
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/openmoko-dialer-main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

# base deps
PKG_CHECK_MODULES(OPENMOKO, openmoko-libs >= 0.0.1,,
                 AC_MSG_ERROR([*** Required OpenMoko Libraries >= 0.0.1 not installed ***]))
                 
PKG_CHECK_MODULES(LIBGSMD, libgsmd ,,
                 AC_MSG_ERROR([*** Required libgsmd not installed ***]))

PKG_CHECK_MODULES(LIBASOUND, alsa,,
                 AC_MSG_ERROR([*** Required libasound not installed ***]))


LIBS="$LIBS $GTK_LIBS $LIBGSMD_LIBS $LIBASOUND_LIBS -lm -ldl"

CFLAGS="$CFLAGS $OPENMOKO_CFLAGS $LIBGSMD_CFLAGS $LIBASOUND_CFLAGS"

# output stuff
AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])
