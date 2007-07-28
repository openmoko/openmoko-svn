AC_PREREQ(2.53)
AC_INIT(openmoko-panel-bt, %(1.0.0), http://www.openmoko.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/openmoko-panel-bt.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

# base deps
PKG_CHECK_MODULES(OPENMOKO, openmoko-libs >= 0.0.2,,
                 AC_MSG_ERROR([*** Required OpenMoko Libraries >= 0.0.2 not installed ***]))

LIBS="$LIBS $OPENMOKO_LIBS"
CFLAGS="$CFLAGS $OPENMOKO_CFLAGS" 

# output stuff
AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])
