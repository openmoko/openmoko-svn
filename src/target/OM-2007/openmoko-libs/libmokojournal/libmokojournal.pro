TEMPLATE = lib
TARGET = mokojournal

HEADERS += src/moko-journal.h src/moko-time-priv.h src/moko-time.h
SOURCES += src/moko-journal.c src/moko-time.c

PKGCONFIG += libecal-1.2

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )

