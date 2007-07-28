TEMPLATE = app
DEPENDPATH += src
INCLUDEPATH += . src

# Input
HEADERS += config.h \
           src/alsa.h \
           src/common.h \
           src/contacts.h \
           src/dialer-callbacks-connection.h \
           src/dialer-main.h \
           src/dialer-window-dialer.h \
           src/dialer-window-history.h \
           src/dialer-window-incoming.h \
           src/dialer-window-outgoing.h \
           src/dialer-window-pin.h \
           src/dialer-window-talking.h \
           src/error.h \
           src/moko-dialer-autolist.h \
           src/moko-dialer-declares.h \
           src/moko-dialer-includes.h \
           src/moko-dialer-panel.h \
           src/moko-dialer-status.h \
           src/moko-dialer-textview.h \
           src/moko-dialer-tip.h \
           src/moko-digit-button.h

SOURCES += src/alsa.c \
           src/common.c \
           src/contacts.c \
           src/dialer-callbacks-connection.c \
           src/dialer-main.c \
           src/dialer-window-dialer.c \
           src/dialer-window-history.c \
           src/dialer-window-incoming.c \
           src/dialer-window-outgoing.c \
           src/dialer-window-pin.c \
           src/dialer-window-talking.c \
           src/moko-dialer-autolist.c \
           src/moko-dialer-panel.c \
           src/moko-dialer-status.c \
           src/moko-dialer-textview.c \
           src/moko-dialer-tip.c \
           src/moko-digit-button.c

MOKOCONFIG = mokoui mokogsmd mokojournal
PKGCONFIG += libebook-1.2 alsa

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
