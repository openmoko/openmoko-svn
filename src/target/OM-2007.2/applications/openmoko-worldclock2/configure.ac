AC_PREREQ(2.53)
AC_INIT(openmoko-worldclock, 0.0, http://www.openedhand.com/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/worldclock-main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.6.0)
PKG_CHECK_MODULES(MOKOUI, libmokoui2 >= 0.3)
PKG_CHECK_MODULES(JANA, libjana libjana-ecal libjana-gtk)

AC_OUTPUT([
Makefile
src/Makefile
])

