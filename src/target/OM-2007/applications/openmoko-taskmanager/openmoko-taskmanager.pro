HEADERS = footer.h callbacks.h main.h
SOURCES = footer.c callbacks.c main.c

PKGCONFIG += gtk+-2.0 dbus-glib-1

SERVICES = services.xml

DEFINES += DBUS_API_SUBJECT_TO_CHANGE

include ( ../../qmake/openmoko-include.pro )
