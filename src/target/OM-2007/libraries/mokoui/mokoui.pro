TEMPLATE = lib
VERSION = 0.0.1

HEADERS = moko-application.h moko-window.h moko-paned-window.h moko-menubar.h moko-toolbar.h
SOURCES = moko-application.c moko-window.c moko-paned-window.c moko-menubar.c moko-toolbar.c

PKGCONFIG += gtk+-2.0

include ( $(OPENMOKODIR)/qmake/openmoko-include.pro )
