AC_PREREQ(2.53)
AC_INIT(neod, 0.1, http://www.openmoko.org)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/neod-main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

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

PKG_CHECK_MODULES(NEOD, gtk+-2.0 gthread-2.0 gconf-2.0 libpulse)

AC_ARG_WITH([platform],
            AC_HELP_STRING([--with-platform], [Which platform to use [[default=vanilla]]]),
            [neod_platform=$with_platform])

if test x"$neod_platform" = x"neo1973"; then
        NEOD_CFLAGS="$NEOD_CFLAGS -DNEOD_PLATFORM_FIC_NEO1973"
fi

if test x"$neod_platform" = x"ezx"; then
        NEOD_CFLAGS="$NEOD_CFLAGS -DNEOD_PLATFORM_MOTOROLA_EZX"
fi

if test x"$neod_platform" = x"htc"; then
	NEOD_CFLAGS="$NEOD_CFLAGS -DNEOD_PLATFORM_HTC"
fi

if test x"$neod_platform" = x"ipaq"; then
	NEOD_CFLAGS="$NEOD_CFLAGS -DNEOD_PLATFORM_IPAQ"
fi

CFLAGS=$NEOD_CFLAGS

AC_OUTPUT([
Makefile
po/Makefile.in
src/Makefile
data/Makefile
])
