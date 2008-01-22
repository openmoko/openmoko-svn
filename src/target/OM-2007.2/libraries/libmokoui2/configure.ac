AC_PREREQ(2.53)
AC_INIT(libmokoui2, 0.3, http://www.openmoko.org/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(libmokoui/moko-stock.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

GTK_VERSION=2.6

dnl i18n support
GETTEXT_PACKAGE=libmokoui
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT

# base deps
PKG_CHECK_MODULES(DEPS, gtk+-2.0)

AC_ARG_ENABLE(python, 
  [  --enable-python   	  enable python binding [default=yes]],
	   enable_python=$enableval,
	   enable_python=yes)

if test $enable_python != no ; then
    AM_PATH_PYTHON(2.3.5)
    PKG_CHECK_MODULES(GNOME_PYTHON, gnome-python-2.0 >= 2.10.00)
    AC_SUBST(PYGDK_CFLAGS)
    AC_SUBST(PYGDK_LIBS)

    PKG_CHECK_MODULES(PYGOB, pygobject-2.0 >= 2.11.0)
    AC_SUBST(PYGOB_CFLAGS)
    AC_SUBST(PYGOB_LIBS)
    PYTHON_BIND_DIR=bindings/python
    AC_SUBST(PYTHON_BIND_DIR)
fi

if test x$MOKO_DEVEL = xon ; then
    AC_MSG_NOTICE([removing -O2 from CFLAGS])
    CFLAGS="-g -Wall"
fi

LIBS="$LIBS $DEPS_LIBS"
CFLAGS="$CFLAGS $DEPS_CFLAGS"

AC_SUBST(LIBS)

GTK_DOC_CHECK([1.4])


# output stuff
AC_OUTPUT([
Makefile
libmokoui2.pc
libmokoui/Makefile
bindings/vala/Makefile
doc/Makefile
doc/reference/Makefile
doc/reference/version.xml
bindings/python/Makefile
])

echo "CFLAGS = $CFLAGS"
echo "PREFIX = $prefix"
echo "LIBS   = $LIBS"
