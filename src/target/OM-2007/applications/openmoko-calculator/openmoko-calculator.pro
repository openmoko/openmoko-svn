TEMPLATE = app
DEPENDPATH += src
INCLUDEPATH += . src

# Input
HEADERS += src/calc-main.h
SOURCES += src/calc-main.c

MOKOCONFIG = mokoui
include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
