HEADERS += src/bacon-message-connection.h \
           src/contacts-callbacks-ebook.h \
           src/contacts-callbacks-ui.h \
           src/contacts-contact-pane.h \
           src/contacts-defs.h \
           src/contacts-groups-editor.h \
           src/contacts-gtk.h \
           src/contacts-main.h \
           src/contacts-omoko.h \
           src/contacts-ui.h \
           src/contacts-utils.h

SOURCES += src/bacon-message-connection.c \
           src/contacts-callbacks-ebook.c \
           src/contacts-callbacks-ui.c \
           src/contacts-contact-pane.c \
           src/contacts-groups-editor.c \
           src/contacts-gtk.c \
           src/contacts-main.c \
           src/contacts-omoko.c \
           src/contacts-ui.c \
           src/contacts-utils.c

MOKOCONFIG = mokoui
PKGCONFIG += libebook-1.2
include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
