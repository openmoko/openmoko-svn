AC_PREREQ(2.53)
AC_INIT(openmoko-calculator, 0.0.5, rodolphe.ortalo@free.fr)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/calc-main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

dnl i18n support
GETTEXT_PACKAGE=Calculator
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])
AC_SUBST(GETTEXT_PACKAGE)
ALL_LINGUAS="fr ru sv"
AM_GLIB_GNU_GETTEXT

# base deps
PKG_CHECK_MODULES(OPENMOKO, openmoko-libs >= 0.0.1,,
                 AC_MSG_ERROR([*** Required OpenMoko Libraries >= 0.0.1 not installed ***]))
LIBS="$LIBS $GTK_LIBS $OPENMOKO_LIBS"
CFLAGS="$CFLAGS $OPENMOKO_CFLAGS"

# output stuff
AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
data/Makefile
])
