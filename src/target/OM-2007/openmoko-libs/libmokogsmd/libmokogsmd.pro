TEMPLATE = lib
TARGET = mokogsmd

GENMARSHALS = moko-gsmd-marshal.list
GENMARSHALS_PREFIX = moko_gsmd_marshal

HEADERS += moko-gsmd-connection.h
SOURCES += moko-gsmd-connection.c

PKGCONFIG += glib-2.0 libgsmd

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )

