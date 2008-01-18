AC_PREREQ(2.53)
AC_INIT(openmoko-panel-memory, 0.0, http://www.openmoko.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/openmoko-panel-memory.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(APPLET, libmokopanelui2 libnotify dbus-glib-1)

AC_OUTPUT([
Makefile
src/Makefile
])
