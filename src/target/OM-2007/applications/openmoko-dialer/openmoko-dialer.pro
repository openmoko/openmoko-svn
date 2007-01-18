HEADERS += \
           src/alsa.h \
           src/common.h \
           src/contacts.h \
           src/dialergsm.h \
           src/error.h \
           src/event.h \
           src/history.h \
           src/moko-dialer-autolist.h \
           src/moko-dialer-declares.h \
           src/moko-dialer-includes.h \
           src/moko-dialer-panel.h \
           src/moko-dialer-status.h \
           src/moko-dialer-textview.h \
           src/moko-dialer-tip.h \
           src/moko-digit-button.h \
           src/openmoko-dialer-main.h \
           src/openmoko-dialer-window-dialer.h \
           src/openmoko-dialer-window-history.h \
           src/openmoko-dialer-window-incoming.h \
           src/openmoko-dialer-window-outgoing.h \
           src/openmoko-dialer-window-talking.h \
           src/pin.h

SOURCES += \
           src/alsa.c \
           src/common.c \
           src/contacts.c \
           src/dialergsm.c \
           src/event.c \
           src/history.c \
           src/moko-dialer-autolist.c \
           src/moko-dialer-panel.c \
           src/moko-dialer-status.c \
           src/moko-dialer-textview.c \
           src/moko-dialer-tip.c \
           src/moko-digit-button.c \
           src/openmoko-dialer-main.c \
           src/openmoko-dialer-window-dialer.c \
           src/openmoko-dialer-window-history.c \
           src/openmoko-dialer-window-incoming.c \
           src/openmoko-dialer-window-outgoing.c \
           src/openmoko-dialer-window-talking.c \
           src/pin.c

MOKOCONFIG = mokoui
PKGCONFIG += libgsmd alsa

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )

