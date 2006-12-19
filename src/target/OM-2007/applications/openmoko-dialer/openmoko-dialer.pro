DEPENDPATH += . src
INCLUDEPATH += . src

# Input
HEADERS += src/common.h \
           src/contacts.h \
           src/error.h \
           src/moko-dialer-autolist.h \
           src/moko-dialer-declares.h \
           src/moko-dialer-includes.h \
           src/moko-dialer-panel.h \
           src/moko-dialer-textview.h \
           src/moko-dialer-tip.h \
           src/moko-digit-button.h \
           src/openmoko-dialer-main.h

SOURCES += src/common.c \
           src/contacts.c \
           src/moko-dialer-autolist.c \
           src/moko-dialer-panel.c \
           src/moko-dialer-textview.c \
           src/moko-dialer-tip.c \
           src/moko-digit-button.c \
           src/openmoko-dialer-main.c

MOKOCONFIG = mokoui
include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
