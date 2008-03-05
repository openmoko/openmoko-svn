AC_PREREQ(2.53)
AC_INIT(openmoko-mediaplayer2, 0.0.1, http://openmoko.org)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(src/main.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AC_STDC_HEADERS
AC_PROG_LIBTOOL
AC_PROG_INTLTOOL([0.35.0])
AC_LANG_CPLUSPLUS

dnl i18n support
GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT


# Check for required libraries

PKG_CHECK_MODULES(GTK, [glib-2.0 >= 2.4.0 gtk+-2.0 >= 2.4.0 gthread-2.0 pango],,
	AC_MSG_ERROR([*** Cannot find glib2/gtk2/gthread/pango ***]))

PKG_CHECK_MODULES(DBUS, [dbus-glib-1 >= 0.23.4],,
	AC_MSG_ERROR([*** Cannot find dbus-glib-1 ***]))

# We just check for libmokoui2 for now (down below)
#PKG_CHECK_MODULES(OPENMOKO, [openmoko-libs >= 0.1.0],,
#	AC_MSG_ERROR([*** Required OpenMoko Libraries >= 0.1.0 not installed ***]))

PKG_CHECK_MODULES(GCONF, [gconf-2.0],,
	AC_MSG_ERROR([*** Cannot find gconf ***]))

PKG_CHECK_MODULES(GSTREAMER, [gstreamer-0.10 >= 0.10],,
	AC_MSG_ERROR([*** Cannot find gstreamer ***]))

# gstinterfaces-0.10 is contained in libgstximagesink - which in turn is
# contained in gst-plugins-base. So if we have gstreamer we also have gstinterfaces-0.10
LIBS="$GTK_LIBS $OPENMOKO_LIBS $GSTREAMER_LIBS $GCONF_LIBS $LIBS -lgstinterfaces-0.10"

LIBS="-lexpat $LIBS"
AC_CHECK_LIB(expat, XML_Parse, , [echo "Error: libexpat not found"; exit -1])

LIBS="-luriparser $LIBS"
AC_CHECK_LIB(uriparser, uriParseUriW, , [echo "Error: liburiparser not found"; exit -1])

LIBS="-lspiff $LIBS"
AC_CHECK_LIB(spiff, spiff_parse, , [echo "Error: XSPF library (libspiff) not found"; exit -1])

LIBS="-lmokoui2 $LIBS"
AC_CHECK_LIB(mokoui2, moko_stock_register, , [echo "Error: libmokoui2 not found"; exit -1])

LIBS="-lts $LIBS"
AC_CHECK_LIB(ts, ts_open, , [echo "Error: libts not found"; exit -1])


CFLAGS="$TS_CFLAGS $OPENMOKO_CFLAGS $GSTREAMER_CFLAGS $GCONF_CFLAGS $CFLAGS"

localedir=$datadir/locale
AC_SUBST(localedir)

OMP_DEFINES="-DDATA_DIR=\\\"$datadir/openmoko-mediaplayer\\\" -DLOCALEDIR=\\\"$localedir\\\""
AC_SUBST(OMP_DEFINES)

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    images/Makefile
    po/Makefile.in
])
AC_OUTPUT
