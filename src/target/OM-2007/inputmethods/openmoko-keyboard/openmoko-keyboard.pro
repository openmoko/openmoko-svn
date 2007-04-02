MOKOCONFIG = mokoui
PKGCONFIG += libfakekey expat
QMAKE_CFLAGS += -UWANT_CAIRO -UHAVE_ALLOCA
LIBS += -lexpat -lXft
DEFINES += VERSION=1
TEMPLATE = app
DEPENDPATH += src examples
INCLUDEPATH += . src

HEADERS += src/matchbox-keyboard-ui-cairo-backend.h \
           src/matchbox-keyboard-ui-xft-backend.h \
           src/matchbox-keyboard.h \
           src/util-list.h

SOURCES += src/config-parser.c \
           src/matchbox-keyboard-defs.c \
           src/matchbox-keyboard-image.c \
           src/matchbox-keyboard-key.c \
           src/matchbox-keyboard-layout.c \
           src/matchbox-keyboard-row.c \
           src/matchbox-keyboard-ui-xft-backend.c \
           src/matchbox-keyboard-ui.c \
           src/matchbox-keyboard-xembed.c \
           src/matchbox-keyboard.c \
           src/util-list.c \
           src/util.c

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
