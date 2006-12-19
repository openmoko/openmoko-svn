DEPENDPATH += . src
INCLUDEPATH += . src

# Input
HEADERS += src/app-history.h \
           src/callbacks.h \
           src/main.h \
           src/mainmenu.h \
           src/menu-list.h \
           src/mokoiconview.h \
           src/support.h
SOURCES += src/app-history.c \
           src/callbacks.c \
           src/main.c \
           src/mainmenu.c \
           src/menu-list.c \
           src/mokoiconview.c \
           src/support.c

MOKOCONFIG = mokoui
include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )

