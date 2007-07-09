AC_PREREQ(2.53)
AC_INIT(libmokogsmd, 0.0.4, http://www.openmoko.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(libmokogsmd/moko-gsmd-connection.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

GTK_VERSION=2.6

# base deps
PKG_CHECK_MODULES(DEPS, libgsmd glib-2.0)

if test x$MOKO_DEVEL = xon ; then
    AC_MSG_NOTICE([removing -O2 from CFLAGS])
    CFLAGS="-g -Wall"
fi

LIBS="$LIBS $DEPS_LIBS"
CFLAGS="$CFLAGS $DEPS_CFLAGS"

AC_SUBST(LIBS)

# glib-genmarshal
GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)



# output stuff
AC_OUTPUT([
Makefile
libmokogsmd.pc
libmokogsmd/Makefile
])

echo "CFLAGS = $CFLAGS"
echo "PREFIX = $prefix"
echo "LIBS   = $LIBS"
