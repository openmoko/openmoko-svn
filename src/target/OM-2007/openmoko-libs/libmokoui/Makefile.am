source_h = \
    moko-ui.h \
    moko-alignment.h \
    moko-application.h \
    moko-details-window.h \
    moko-dialog-window.h \
    moko-finger-tool-box.h \
    moko-finger-wheel.h \
    moko-finger-window.h \
    moko-fixed.h \
    moko-menu-box.h \
    moko-navigation-list.h \
    moko-panel-applet.h \
    moko-pixmap-button.h \
    moko-paned-window.h \
    moko-scrolled-pane.h \
    moko-stock.h \
    moko-tool-box.h \
    moko-tree-view.h \
    moko-window.h \
    moko-dialog.h \
    moko-message-dialog.h

source_c = \
    moko-alignment.c \
    moko-application.c \
    moko-details-window.c \
    moko-dialog-window.c \
    moko-finger-tool-box.c \
    moko-finger-wheel.c \
    moko-finger-window.c \
    moko-fixed.c \
    moko-menu-box.c \
    moko-navigation-list.c \
    moko-panel-applet.c \
    moko-pixmap-button.c \
    moko-paned-window.c \
    moko-scrolled-pane.c \
    moko-stock.c \
    moko-tool-box.c \
    moko-tree-view.c \
    moko-window.c \
    moko-dialog.c \
    moko-message-dialog.c

DATADIR=$(datadir)

AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"

AM_CFLAGS = -Wall -pedantic -std=c99

lib_LTLIBRARIES      = libmokoui.la
libmokoui_la_SOURCES     = $(source_c) $(source_h)
libmokoui_la_LIBADD      =
libmokoui_la_LDFLAGS = -version-info 0:1:0

libopenmokouiheadersdir = $(includedir)/openmoko-libs/libmokoui
libopenmokouiheaders_DATA = $(source_h)
