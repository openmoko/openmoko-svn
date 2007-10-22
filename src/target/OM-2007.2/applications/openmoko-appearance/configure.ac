AC_PREREQ(2.53)
AC_INIT(openmoko-appearance, 0.1, http://www.openedhand.com/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/appearance-main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CC_STDC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(MOKOUI, libmokoui2 gconf-2.0)

AC_SUBST(MOKOUI)

AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])

