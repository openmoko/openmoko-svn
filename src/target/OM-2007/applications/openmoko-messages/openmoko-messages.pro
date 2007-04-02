TEMPLATE = app
DEPENDPATH += src
INCLUDEPATH += . src

# Input
HEADERS += src/callbacks.h \
           src/detail-area.h \
           src/foldersdb.h \
           src/main.h \
           src/message.h \
           src/sms-contact-window.h \
           src/sms-dialog-window.h \
           src/sms-membership-window.h
SOURCES += src/callbacks.c \
           src/detail-area.c \
           src/foldersdb.c \
           src/main.c \
           src/sms-contact-window.c \
           src/sms-dialog-window.c \
           src/sms-membership-window.c

MOKOCONFIG = mokoui
PKGCONFIG += dbus-glib-1 libebook-1.2
include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )

