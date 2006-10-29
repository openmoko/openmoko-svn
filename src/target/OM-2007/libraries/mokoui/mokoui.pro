TEMPLATE = lib
VERSION = 0.0.1

HEADERS = \
	moko-alignment.h \
	moko-pixmap-container.h \
	moko-pixmap-button.h \
	moko-application.h \
	moko-window.h \
	moko-finger-window.h \
	moko-paned-window.h \
	moko-menubox.h \
	moko-toolbox.h \
	moko-search-bar.h
SOURCES = \
	moko-alignment.c \
    moko-pixmap-container.c \
	moko-pixmap-button.c \
	moko-application.c \
	moko-window.c \
	moko-finger-window.c \
	moko-paned-window.c \
	moko-menubox.c \
	moko-toolbox.c \
	moko-search-bar.c

PKGCONFIG += gtk+-2.0

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
