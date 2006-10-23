TEMPLATE = lib
VERSION = 0.0.1

HEADERS = moko-application.h moko-window.h moko-finger-window.h moko-paned-window.h moko-menubox.h moko-toolbox.h
SOURCES = moko-application.c moko-window.c moko-finger-window.c moko-paned-window.c moko-menubox.c moko-toolbox.c

PKGCONFIG += gtk+-2.0

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
