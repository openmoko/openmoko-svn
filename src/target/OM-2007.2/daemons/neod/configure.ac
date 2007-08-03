AC_PREREQ(2.53)
AC_INIT(neod, 0.1, http://www.openmoko.org)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/neod-main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

LIBGTK_VERSION=2.6.0

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CC_STDC
AC_STDC_HEADERS
AC_PROG_LIBTOOL
AC_PROG_INTLTOOL([0.35.0])

dnl i18n support
GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT

PKG_CHECK_MODULES(GTK, gtk+-2.0 >= $LIBGTK_VERSION)

CFLAGS=$GTK_CFLAGS
AC_CHECK_DECLS([g_date_set_time_t],
               [have_date=yes], [have_date=no], [#include <glib.h>])
AM_CONDITIONAL(HAVE_GDATE_TIME_T, test "x$have_date" = "xyes")
CFLAGS=$old_cflags

AC_OUTPUT([
Makefile
po/Makefile.in
src/Makefile
])
