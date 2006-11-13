source_h = \
    moko-alignment.h \
    moko-application.h \
    moko-finger-wheel.h \
    moko-finger-window.h \
    moko-fixed.h
    moko-menu-box.h \
    moko-pixmap-button.h \
    moko-paned-window.h \
    moko-search-bar.h \
    moko-tool-box.h \
    moko-tree-view.h \
    moko-window.h

source_c = \
    moko-alignment.c \
    moko-application.c \
    moko-finger-wheel.c \
    moko-finger-window.c \
    moko-fixed.c
    moko-menu-box.c \
    moko-pixmap-button.c \
    moko-paned-window.c \
    moko-search-bar.c \
    moko-tree-view.c \
    moko-tool-box.c \
    moko-window.c

DATADIR=$(datadir)

AM_CFLAGS = -Wall -pedantic -std=c99 -DDATADIR=\"$(datadir)\libopenmoko\"

lib_LTLIBRARIES      = libmokoui.la
libmokoui_la_SOURCES     = $(source_c) $(source_h)
libmokoui_la_LIBADD      =
libmokoui_la_LDFLAGS = -version-info 0:1:0

libopenmokouiheadersdir = $(includedir)/libmokoui
libopenmokouiheaders_DATA = $(source_h)
