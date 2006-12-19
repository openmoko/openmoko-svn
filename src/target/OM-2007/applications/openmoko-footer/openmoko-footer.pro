DEPENDPATH += . src
INCLUDEPATH += . src

# Input
HEADERS += src/callbacks.h src/footer.h src/main.h src/misc.h
SOURCES += src/callbacks.c src/footer.c src/main.c src/misc.c

PKGCONFIG += gtk+-2.0 dbus-glib-1
SERVICES = services.xml
DEFINES += DBUS_API_SUBJECT_TO_CHANGE

#MOKOCONFIG = mokoui
include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )

