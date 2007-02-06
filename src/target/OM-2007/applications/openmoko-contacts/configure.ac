AC_PREREQ(2.53)
AC_INIT(contacts, 0.2, http://www.openedhand.com/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/contacts-main.c)
AM_CONFIG_HEADER(src/config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL
AC_PROG_INTLTOOL

dnl i18n support
GETTEXT_PACKAGE=Contacts
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])
AC_SUBST(GETTEXT_PACKAGE)
ALL_LINGUAS="de et fi fr it nl sv ca"
AM_GLIB_GNU_GETTEXT

PKG_CHECK_MODULES(CONTACTS, glib-2.0 glib-2.0 gtk+-2.0 libebook-1.2)

old_cflags=$CFLAGS
CFLAGS=$CONTACTS_CFLAGS
AC_CHECK_TYPE(EContactPhotoType, [], [], [#include <libebook/e-book.h>])
CFLAGS=$old_cflags

if test $ac_cv_type_EContactPhotoType = yes; then
	AC_DEFINE(HAVE_PHOTO_TYPE, 1, [Defined if EContactPhotoType exists])
fi

AC_ARG_ENABLE(gnome-vfs, [  --enable-gnome-vfs      Use gnome-vfs for importing and exporting vCards [[default=no]]],
 [want_gnomevfs="$enableval"])

if test "$want_gnomevfs" = "yes"; then
	PKG_CHECK_MODULES(GNOMEVFS, gnome-vfs-2.0)
	AC_DEFINE(HAVE_GNOMEVFS, 1, [Define if you want Gnome VFS support])
	CONTACTS_CFLAGS="$CONTACTS_CFLAGS $GNOMEVFS_CFLAGS"
	CONTACTS_LIBS="$CONTACTS_LIBS $GNOMEVFS_LIBS"
fi

AC_ARG_ENABLE(gconf, [  --enable-gconf          Use gconf for storing settings [[default=no]]],
 [want_gconf="$enableval"])

if test "$want_gconf" = "yes"; then
	PKG_CHECK_MODULES(GCONF, gconf-2.0)
	AC_DEFINE(HAVE_GCONF, 1, [Define if you want Gconf support])
	CONTACTS_CFLAGS="$CONTACTS_CFLAGS $GCONF_CFLAGS"
	CONTACTS_LIBS="$CONTACTS_LIBS $GCONF_LIBS"

	# Find how and where to put the GConf schemas
	AC_PATH_PROG(GCONFTOOL, gconftool-2, no)
	if test x"$GCONFTOOL" = xno; then
		AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
	fi
fi
AM_CONDITIONAL([HAVE_GCONF], [test "$want_gconf" = "yes"])
 

PKG_CHECK_MODULES(OMOKO,[openmoko-libs])
CONTACTS_CFLAGS="$CONTACTS_CFLAGS $OMOKO_CFLAGS"
CONTACTS_LIBS="$CONTACTS_LIBS $OMOKO_LIBS"
AC_SUBST(FRONTEND)

AM_GCONF_SOURCE_2

AC_OUTPUT([
Makefile
data/Makefile
po/Makefile.in
src/Makefile
])
