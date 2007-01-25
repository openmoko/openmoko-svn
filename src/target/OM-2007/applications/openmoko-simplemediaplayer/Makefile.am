ACLOCAL_AMFLAGS = -I m4

SUBDIRS = intl libbeep beep Output Input Visualization po images

man_MANS = beep-media-player.1

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = bmp.pc

EXTRA_DIST = \
  FAQ \
  bmp.spec \
  $(pkgconfig_DATA) \
  $(wildcard m4/*.m4)
