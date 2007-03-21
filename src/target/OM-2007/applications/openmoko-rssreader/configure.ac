AC_PREREQ([2.59])
AC_INIT([openmoko-rssreader], [0.0.4], [http://openmoko.org])
AM_INIT_AUTOMAKE([1.9 foreign dist-bzip2 no-dist-gzip])
AC_CONFIG_SRCDIR(src/main.c)
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

# base deps
PKG_CHECK_MODULES(OPENMOKO, openmoko-libs >= 0.0.1)
PKG_CHECK_MODULES(MRSS,     mrss          >= 0.17 )
PKG_CHECK_MODULES(GTHREAD,  gthread-2.0           )

# output stuff
AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])
