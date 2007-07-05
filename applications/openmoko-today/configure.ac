AC_PREREQ(2.53)
AC_INIT(today, 0.1, http://www.openedhand.com/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/today-main.c)
AM_CONFIG_HEADER(src/config.h)
AM_MAINTAINER_MODE

LIBEBOOK_VERSION=1.4.2
LIBECAL_VERSION=1.4.2
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

PKG_CHECK_MODULES(SEXY, libsexy,
                  [enable_sexy=yes
                   AC_DEFINE(HAVE_SEXY, 1, [We are sexy enough])],
                   enable_sexy=false)

PKG_CHECK_MODULES(GTK, gtk+-2.0 >= $LIBGTK_VERSION)
PKG_CHECK_MODULES(ECAL, libecal-1.2 >= $LIBECAL_VERSION)
PKG_CHECK_MODULES(EBOOK, libebook-1.2 >= $LIBEBOOK_VERSION)

if test x$TODAY_DEVEL != x ; then
    CFLAGS="-Wall -Wextra -Wshadow -Wpointer-arith -g"
fi

old_cflags=$CFLAGS
CFLAGS=$ECAL_CFLAGS
AC_CHECK_TYPE(ECalComponentId, [], [], [#include <libecal/e-cal.h>])

if test $ac_cv_type_ECalComponentId = yes; then
        AC_DEFINE(HAVE_CID_TYPE, 1, [Defined if ECalComponentId exists])
fi

CFLAGS=$GTK_CFLAGS
AC_CHECK_DECLS([g_date_set_time_t],
               [have_date=yes], [have_date=no], [#include <glib.h>])
AM_CONDITIONAL(HAVE_GDATE_TIME_T, test "x$have_date" = "xyes")
CFLAGS=$old_cflags

AC_OUTPUT([
Makefile
po/Makefile.in
libkoto/Makefile
src/Makefile
])

