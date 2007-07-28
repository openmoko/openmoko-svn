AC_PREREQ(2.53)
AC_INIT(today, 0.1, http://www.openedhand.com/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/today-main.c)
AM_MAINTAINER_MODE

LIBEBOOK_VERSION=1.4.2
LIBECAL_VERSION=1.4.2

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL
AC_PROG_INTLTOOL

dnl i18n support
GETTEXT_PACKAGE=Today
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])
AC_SUBST(GETTEXT_PACKAGE)
ALL_LINGUAS=""
AM_GLIB_GNU_GETTEXT

PKG_CHECK_MODULES(TODAY,
                  openmoko-libs
                  libstartup-notification-1.0
                  libebook-1.2 >= $LIBEBOOK_VERSION
                  libecal-1.2 >= $LIBEBOOK_VERSION)


if test x$TODAY_DEVEL != x ; then
    CFLAGS="-Wall -Wextra -Wshadow -Wpointer-arith -g"
fi

AC_OUTPUT([
Makefile
po/Makefile.in
src/Makefile
tests/Makefile
])
