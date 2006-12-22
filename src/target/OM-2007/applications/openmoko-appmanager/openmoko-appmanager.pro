MOKOCONFIG = mokoui

HEADERS += src/application-menu.h \
           src/appmanager-data.h \
           src/appmanager-window.h \
           src/detail-area.h \
           src/errorcode.h \
           src/filter-menu.h \
           src/ipkg_cmd.h \
           src/ipkgapi.h \
           src/navigation-area.h \
           src/package-list.h \
           src/pixbuf-list.h \
           src/select-menu.h \
           src/tool-box.h \
           src/apply-dialog.h
SOURCES += src/application-menu.c \
           src/appmanager-data.c \
           src/appmanager-window.c \
           src/detail-area.c \
           src/filter-menu.c \
           src/ipkgapi.c \
           src/navigation-area.c \
           src/package-list.c \
           src/pixbuf-list.c \
           src/select-menu.c \
           src/tool-box.c \
           src/apply-dialog.c

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )

