AC_PREREQ(2.53)
AC_INIT(openmoko-mediaplayer, 0.0.1, http://openmoko.org)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(src/main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AC_STDC_HEADERS
AC_PROG_LIBTOOL

AM_GNU_GETTEXT
AM_GNU_GETTEXT_VERSION([0.12.1])


CFLAGS="-I${prefix}/include $CFLAGS"
AC_SUBST(CFLAGS)
CPPFLAGS="-I${prefix}/include $CPPFLAGS"
AC_SUBST(CPPFLAGS)

# Check for GTK/GLib/GThread/Pango, dbus, OpenMoko libs, gstreamer, expat, uriparser and XSPF

PKG_CHECK_MODULES(GTK, [glib-2.0 >= 2.4.0 gtk+-2.0 >= 2.4.0 gthread-2.0 pango],,
	AC_MSG_ERROR([*** Cannot find glib2/gtk2/gthread/pango ***]))

PKG_CHECK_MODULES(DBUS, [dbus-glib-1 >= 0.23.4],,
	AC_MSG_ERROR([*** Cannot find dbus-glib-1 ***]))

#PKG_CHECK_MODULES(OPENMOKO, [openmoko-libs >= 0.0.1],,
#	AC_MSG_ERROR([*** Required OpenMoko Libraries >= 0.0.1 not installed ***]))

PKG_CHECK_MODULES(GSTREAMER, [gstreamer-0.10 >= 0.10],,
	AC_MSG_ERROR([*** Cannot find gstreamer ***]))

LIBS="-lexpat -L${prefix}/lib $LIBS"
AC_CHECK_LIB(expat, XML_Parse, [], [echo "Error: libexpat not found"; exit -1])

LIBS="-luriparser $LIBS"
AC_CHECK_LIB(uriparser, uriParseUriW, [], [echo "Error: liburiparser not found"; exit -1])

LIBS="-lspiff $LIBS"
AC_CHECK_LIB(spiff, spiff_parse, [], [echo "Error: XSPF library (libspiff) not found"; exit -1])

LIBS="-lmokoui2 $LIBS"
AC_CHECK_LIB(mokoui2, moko_stock_register, [], [echo "Error: libmokoui2 not found"; exit -1])



LIBS="$GTK_LIBS $OPENMOKO_LIBS $GSTREAMER_LIBS $LIBS"
CFLAGS="$OPENMOKO_CFLAGS $GSTREAMER_CFLAGS $CFLAGS"

localedir=$datadir/locale
AC_SUBST(localedir)

OMP_DEFINES="-DDATA_DIR=\\\"$datadir/openmoko-mediaplayer\\\" -DLOCALEDIR=\\\"$localedir\\\""
AC_SUBST(OMP_DEFINES)

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    images/Makefile
    po/Makefile.in
    intl/Makefile
])
AC_OUTPUT
