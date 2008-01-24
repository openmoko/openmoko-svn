AC_PREREQ(2.53)
AC_INIT(launcher, 0.0, http://www.openedhand.com/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/launcher-main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

LIBGTK_VERSION=2.6.0
LIBMOKOUI_VERSION=0.0.5

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

AC_CHECK_HEADERS([sys/inotify.h], inotify_support=yes, inotify_support=no)
AM_CONDITIONAL(HAVE_INOTIFY, [test "$inotify_support" = "yes"])
if test x$inotify_support = xyes; then
        AC_DEFINE(WITH_INOTIFY, [1], [If inotify is enabled])
fi

PKG_CHECK_MODULES(MOKOUI, libmokoui2 >= $LIBMOKOUI_VERSION)
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= $LIBGTK_VERSION)

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

AC_OUTPUT([
Makefile
po/Makefile.in
libtaku/Makefile
src/Makefile
])

