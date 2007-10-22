AC_PREREQ(2.53)
AC_INIT(today, 0.1, http://www.openedhand.com/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/today-main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

LIBEBOOK_VERSION=1.4.2
LIBECAL_VERSION=1.4.2
LIBGTK_VERSION=2.6.0
LIBMOKOUI_VERSION=0.0.5
LIBMOKOJOURNAL_VERSION=0.1.0

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

PKG_CHECK_MODULES(MOKOUI, libmokoui2 >= $LIBMOKOUI_VERSION)
PKG_CHECK_MODULES(MOKOJOURNAL, libmokojournal2 >= $LIBMOKOJOURNAL_VERSION)
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= $LIBGTK_VERSION)
PKG_CHECK_MODULES(ECAL, libecal-1.2 >= $LIBECAL_VERSION)
PKG_CHECK_MODULES(EBOOK, libebook-1.2 >= $LIBEBOOK_VERSION)
PKG_CHECK_MODULES(GCONF, gconf-2.0)

AC_ARG_ENABLE(startup_notification,
        AC_HELP_STRING([--enable-startup-notification], [enable startup notification support]),
        enable_startup_notification=$enableval, enable_startup_notification=yes)

if test x$enable_startup_notification != xno; then
  PKG_CHECK_MODULES(SN, libstartup-notification-1.0, ,
    AC_MSG_ERROR([*** Required Startup Notification library not installed ***]))
  AC_DEFINE(USE_LIBSN, [1], [Has Startup Notification Support])
fi

AC_ARG_ENABLE(standalone,
        AC_HELP_STRING([--enable-standalone], [run in a window, not as the desktop]),
        enable_standalone=$enableval, enable_standalone=no)
if test x$enable_standalone != xno; then
  AC_DEFINE(STANDALONE, [1], [Run in a window])
fi

if test x$TODAY_DEVEL != x ; then
    CFLAGS="-Wall -Wextra -Wshadow -Wpointer-arith -g"
fi

old_cflags=$CFLAGS
CFLAGS=$ECAL_CFLAGS
AC_CHECK_TYPE(ECalComponentId, [], [], [#include <libecal/e-cal.h>])

if test $ac_cv_type_ECalComponentId = yes; then
        AC_DEFINE(HAVE_ECALCOMPONENTID, 1, [Defined if ECalComponentId exists])
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
libtaku/Makefile
src/Makefile
])

