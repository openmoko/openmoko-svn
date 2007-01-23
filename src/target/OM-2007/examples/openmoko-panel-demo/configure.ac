AC_PREREQ(2.53)
AC_INIT(openmoko-panel-demo, 1.0, http://www.openmoko.org)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/demo-panel-applet.c)
#AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

# base deps
PKG_CHECK_MODULES(MATCHBOX_PANEL, matchbox-panel >= 0.1,,
                 AC_MSG_ERROR([*** Required Matchbox Panel 2 >= 0.1 not installed ***]))
LIBS="$LIBS $GTK_LIBS"
CFLAGS="$CFLAGS $MATCHBOX_PANEL"

# output stuff
AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])
