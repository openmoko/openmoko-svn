TEMPLATE = lib
TARGET = mokogsmd

HEADERS += moko-gsmd-connection.h
SOURCES += moko-gsmd-connection.c

PKGCONFIG += glib-2.0 libgsmd

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )

