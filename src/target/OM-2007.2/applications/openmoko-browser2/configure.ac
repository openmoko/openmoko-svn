AC_PREREQ([2.59])
AC_INIT([openmoko-browser], [0.0.1], [http://openmoko.org])
AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2 no-dist-gzip])
AC_CONFIG_SRCDIR(src/main.c)
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

#i18n
AC_PROG_INTLTOOL([0.23])
GETTEXT_PACKAGE=openmoko-browser
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT

# unit testing (inspired by opensync)
# enable gcov profiling
GCOV_CPPFLAGS=""
GCOV_LDFLAGS=""
AC_ARG_ENABLE([profiling],
  [AS_HELP_STRING([--enable-profiling], [enable gcov profiling])],
  [case "${enableval}" in
    yes) GCOV_PROFILING="yes"
         GCOV_CPPFLAGS="-ftest-coverage -fprofile-arcs"
         GCOV_LDFLAGS="-lgcov"
        ;;
    no)  ;;
     *)  AC_MSG_ERROR([bad value ${enableval} for --enable-profiling]) ;;
    esac],[GCOV_PROFILING="no"])
AC_SUBST(GCOV_CPPFLAGS)
AC_SUBST(GCOV_LDFLAGS)
AM_CONDITIONAL(ENABLE_PROFILING, test "x${GCOV_PROFILING}" = "xyes")

#check for check
AM_PATH_CHECK([0.9.2], [ENABLE_TESTING="yes"], [ENABLE_TESTING="no"])
AM_CONDITIONAL(ENABLE_TESTING, test "x${ENABLE_TESTING}" = "xyes")

# base deps
PKG_CHECK_MODULES(OPENMOKO,  libmokoui2    >= 0.1.0)
PKG_CHECK_MODULES(GTHREAD,   gthread-2.0           )
PKG_CHECK_MODULES(WEBKITGTK, WebKitGtk)

# glib-genmarshal
GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)

# output stuff
AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
po/Makefile.in
])
